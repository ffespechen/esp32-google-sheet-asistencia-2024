
# Registro de Asistencia 

Projecto de registro de asistencia usando ESP32 + RFID MFRC522 + Google Spreadsheet Client.

Toma como base el ejemplo de la librería ESP Google Sheet Client [Crear-Leer-Actualizar](https://github.com/mobizt/ESP-Google-Sheet-Client/tree/master/examples/Values/Create_Update_Read) 

Cuenta con indicadores LED y una pantalla LCD como parte de la interfaz de usuario.

- LED ROJO -> Error
- LED AZUL -> Power / Conexión correcta
- LED AMARILLO -> Procesando
- LED VERDE -> Proceso OK

## Autor

- [ffespechen](https://github.com/ffespechen/)


## Documentación

#### RFID - MFRC522

Conexión (para ESP32) usando SPI
- SS/SDA -> D5
- SCK    -> D18
- MOSI   -> D23
- MISO   -> D19
- GND    -> GND
- VCC    -> 3.3V
- RST    -> D0

Librería

[Arduino RFID Library for MFRC522 (SPI). Read/Write a RFID Card or Tag using the ISO/IEC 14443A/MIFARE interface.](https://github.com/miguelbalboa/rfid)


#### LCD

Display LCD 16x2

- Conexión para ESP32
- SDA    -> D21
- SCL    -> D22
- GND    -> GND
- VCC    -> 5v

Librería

[LiquidCrystal_I2C](https://github.com/johnrickman/LiquidCrystal_I2C)


#### Google Sheet Client

[ESP Google Sheet Client](https://github.com/mobizt/ESP-Google-Sheet-Client/tree/master)

