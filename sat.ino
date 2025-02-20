#include <WiFi.h>
#include <HTTPClient.h>
#include <DHT.h>

#define DHTPIN 6
#define DHTTYPE DHT22
#define HW837_PIN 3
#define HUMIDITY_PIN 4

DHT dht(DHTPIN, DHTTYPE);

const char* ssid = "nombre red wifi";          // Reemplaza con tu SSID de WiFi
const char* password = "clave wifi";  // Reemplaza con tu contraseña de WiFi

const char* serverUrl = "http://192.168.100.134:3000/insert-data";  // URL del servidor Node.js

WiFiServer server(80);  // Servidor web en el puerto 80

unsigned long lastDataSend = 0;
const unsigned long dataSendInterval = 1800000; // Intervalo de 30 minutos (en milisegundos)
const unsigned long reconnectInterval = 10000;  // Intervalo de reconexión de 10 segundos

String dbStatus = "Esperando datos...";  // Variable para almacenar el estado de la base de datos

void setup() {
    Serial.begin(115200);
    dht.begin();

    pinMode(HUMIDITY_PIN, INPUT);
    pinMode(HW837_PIN, INPUT);

    // Conectar a WiFi
    connectToWiFi();

    // Mostrar la dirección IP asignada al ESP32
    IPAddress ip = WiFi.localIP();
    Serial.print("Servidor web iniciado en: http://");
    Serial.println(ip);

    // Iniciar servidor web
    server.begin();
}

void loop() {
    // Verificar y mantener la conexión WiFi
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("WiFi desconectado. Reconectando...");
        connectToWiFi();
    }

    // Leer datos de los sensores
    float temperature = dht.readTemperature();
    float humidity = dht.readHumidity();
    int soilMoistureRaw = analogRead(HUMIDITY_PIN);
    int uvValue = analogRead(HW837_PIN);

    // Enviar datos al servidor Node.js cada 30 minutos
    if (millis() - lastDataSend > dataSendInterval) {
        if (sendDataToServer(temperature, humidity, soilMoistureRaw, uvValue)) {
            lastDataSend = millis();  // Reiniciar el temporizador solo si el envío fue exitoso
        } else {
            // Si falla, intentar reconectar cada 10 segundos
            delay(reconnectInterval);
        }
    }

    // Manejar clientes web
    handleWebClient(temperature, humidity, soilMoistureRaw, uvValue);

    delay(2000);  // Esperar 2 segundos antes de la siguiente lectura
}

bool sendDataToServer(float temp, float hum, int soil, int uv) {
    if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;
        http.begin(serverUrl);
        http.addHeader("Content-Type", "application/json");

        String jsonData = "{\"temperature\":" + String(temp) + ",\"humidity\":" + String(hum) + ",\"soilMoisture\":" + String(soil) + ",\"uvValue\":" + String(uv) + "}";
        int httpResponseCode = http.POST(jsonData);

        if (httpResponseCode > 0) {
            String response = http.getString();
            Serial.println("Respuesta del servidor: " + response);

            // Actualizar el estado de la base de datos
            if (response.indexOf("Datos insertados correctamente") >= 0) {
                dbStatus = "OK de la DBA";
                return true;  // Éxito
            } else {
                dbStatus = "Error en la DBA";
            }
        } else {
            Serial.print("Error en la solicitud HTTP: ");
            Serial.println(httpResponseCode);
            dbStatus = "Error en la conexión";
        }

        http.end();
    } else {
        Serial.println("WiFi no conectado");
        dbStatus = "WiFi no conectado";
    }

    return false;  // Fallo
}

void handleWebClient(float temp, float hum, int soil, int uv) {
    WiFiClient client = server.available();
    if (client) {
        String request = client.readStringUntil('\r');
        client.flush();

        // Construir la respuesta HTML con Bootstrap
        String response = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n";
        response += "<!DOCTYPE html><html lang='es'><head>";
        response += "<meta charset='UTF-8'>";
        response += "<meta name='viewport' content='width=device-width, initial-scale=1.0'>";
        response += "<title>Datos de Sensores</title>";
        response += "<link href='https://cdn.jsdelivr.net/npm/bootstrap@5.3.0/dist/css/bootstrap.min.css' rel='stylesheet'>";
        response += "<style>";
        response += "body { background-color: #f8f9fa; }";
        response += ".card { margin-top: 20px; box-shadow: 0 4px 8px rgba(0, 0, 0, 0.1); }";
        response += ".card-header { background-color: #007bff; color: white; }";
        response += "</style>";
        response += "</head><body>";
        response += "<div class='container'>";
        response += "<div class='row justify-content-center'>";
        response += "<div class='col-md-8'>";
        response += "<div class='card'>";
        response += "<div class='card-header text-center'><h3>Datos de los Sensores</h3></div>";
        response += "<div class='card-body'>";
        response += "<p class='card-text'><strong>Temperatura:</strong> " + String(temp) + " °C</p>";
        response += "<p class='card-text'><strong>Humedad:</strong> " + String(hum) + " %</p>";
        response += "<p class='card-text'><strong>Humedad del suelo:</strong> " + String(soil) + "</p>";
        response += "<p class='card-text'><strong>UV:</strong> " + String(uv) + "</p>";
        response += "<p class='card-text'><strong>Estado de la DBA:</strong> " + dbStatus + "</p>";
        response += "</div></div></div></div></div>";
        response += "<script src='https://cdn.jsdelivr.net/npm/bootstrap@5.3.0/dist/js/bootstrap.bundle.min.js'></script>";
        response += "</body></html>";

        // Enviar respuesta al cliente
        client.print(response);
        delay(1);
        client.stop();
    }
}

void connectToWiFi() {
    Serial.println("Conectando a WiFi...");
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.print(".");
    }

    Serial.println("\nConectado a WiFi");
    Serial.print("Dirección IP: ");
    Serial.println(WiFi.localIP());
}