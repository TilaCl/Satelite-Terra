create table sat_dba.sensores
(
    id             int auto_increment
        primary key,
    humedad_porc   float                               not null,
    temp_celsius   float                               not null,
    temp_subterrra int                                 not null,
    uv             int                                 not null,
    fecha          timestamp default CURRENT_TIMESTAMP null
);



