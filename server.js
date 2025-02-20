const express = require('express');
const mysql = require('mysql2'); // Cambia a mysql2
const bodyParser = require('body-parser');
const cors = require('cors');

const app = express();
const port = 3000;

// Middleware
app.use(bodyParser.json());
app.use(cors());

// Configuración de la conexión a la base de datos
const db = mysql.createConnection({
  host: '192.168.100.134',
  port: 3306,               // Puerto de la base de datos (3306 por defecto)
  user: 'usuario',             // Usuario de la base de datos
  password: 'clave',         // Contraseña de la base de datos
  database: 'sat_dba'       // Nombre de la base de datos
});

// Conectar a la base de datos
db.connect((err) => {
  if (err) {
    console.error('Error conectando a la base de datos:', err);
    return;
  }
  console.log('Conectado a la base de datos MySQL en el puerto 3308');
});

// Ruta para insertar datos
app.post('/insert-data', (req, res) => {
  const { temperature, humidity, soilMoisture, uvValue } = req.body;

  // Validar que los datos no estén vacíos
  if (temperature === undefined || humidity === undefined || soilMoisture === undefined || uvValue === undefined) {
    return res.status(400).send('Datos incompletos');
  }

  // Query para insertar datos en la tabla `sensores`
  const query = 'INSERT INTO sensores (humedad_porc, temp_celsius, temp_subterrra, uv) VALUES (?, ?, ?, ?)';
  db.query(query, [humidity, temperature, soilMoisture, uvValue], (err, result) => {
    if (err) {
      console.error('Error insertando datos:', err);
      return res.status(500).send('Error insertando datos en la base de datos');
    }
    console.log('Datos insertados correctamente:', result);
    res.status(200).send('Datos insertados correctamente');
  });
});

// Ruta para obtener los últimos datos de los sensores (opcional)
app.get('/latest-data', (req, res) => {
  const query = 'SELECT * FROM sensores ORDER BY id DESC LIMIT 1';
  db.query(query, (err, result) => {
    if (err) {
      console.error('Error obteniendo datos:', err);
      return res.status(500).send('Error obteniendo datos de la base de datos');
    }
    if (result.length > 0) {
      res.status(200).json(result[0]);
    } else {
      res.status(404).send('No se encontraron datos');
    }
  });
});

// Iniciar el servidor
app.listen(port, () => {
  console.log(`Servidor Node.js corriendo en http://localhost:${port}`);
});


