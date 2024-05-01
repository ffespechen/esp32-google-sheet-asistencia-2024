// Projecto de registro de asistencia usando ESP32 + RFID MFRC522 + Google Spreadsheet
// Toma como base el ejemplo de la librería ESP Google Sheet Client
// https://github.com/mobizt/ESP-Google-Sheet-Client/tree/master/examples/Values/Create_Update_Read

#include <Arduino.h>
#include <WiFi.h>
#include <ESP_Google_Sheet_Client.h>
#include "time.h"
#include <SPI.h>
#include <MFRC522.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

// Auxiliar para montaje de la SD
#include <GS_SDHelper.h>

// Credenciales para la conexión WIFI
#define WIFI_SSID "SSID"
#define WIFI_PASSWORD "PASSWORD"

// Credenciales para la conexión al servicio de Google
#define PROJECT_ID "NOMBRE_PROYECTO"
#define CLIENT_EMAIL "CLIENTE DE CORREO COMPARTIDO"
#define USER_EMAIL "CORREO DEL USUARIO"
const char PRIVATE_KEY[] PROGMEM = "-----BEGIN PRIVATE KEY----- clave privada -----END PRIVATE KEY-----\n";
const char SPREADSHEETID[] = "PARTE DE LA URL QUE IDENTIFICA UNIVOCAMENTE A ESA PLANILLA DE CÁLCULO";

// RFID - MFRC522

// Conexión (para ESP32) usando SPI
// SS/SDA -> D5
// SCK    -> D18
// MOSI   -> D23
// MISO   -> D19
// GND    -> GND
// VCC    -> 3.3V
// RST    -> D0
const int RST_PIN = 0;
const int SS_PIN = 5;

// Instancia del lector RFID
MFRC522 rfid(SS_PIN, RST_PIN); 

MFRC522::MIFARE_Key key; 

// Display LCD 16x2

// Conexión para ESP32
// SDA    -> D21
// SCL    -> D22
// GND    -> GND
// VCC    -> 5v
LiquidCrystal_I2C lcd(0x27, 16, 2);

// LEDs de señalización
const int LED_ROJO_ERROR = 25;
const int LED_VERDE_OK   = 26;
const int LED_AMARILLO_EN_PROCESO = 27;
const int LED_AZUL_POWER = 33;


// Servidor de Fecha + Hora
const char* ntpServer = "pool.ntp.org";
// Diferencia horaria en segundos para el timezone de Río Gallegos, Argentina
const long  gmtOffset_sec = -10800;

// Función auxiliar para el token de conexión con Google
void tokenStatusCallback(TokenInfo info);

// Función auxiliar para mostrar mensajes en el LCD
void mostrar_mensajes(String linea1, String linea2){
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(linea1);
  lcd.setCursor(0, 1);
  lcd.print(linea2);

}

// Array auxiliar para ilustrar el avance de la conexión al WiFi
char espera[] = { '-', '\\', '|', '/' };


void setup()
{

    Serial.begin(115200);
    Serial.println();
    Serial.println();

    // LEDs indicadores
    pinMode(LED_ROJO_ERROR, OUTPUT);
    pinMode(LED_VERDE_OK, OUTPUT);
    pinMode(LED_AMARILLO_EN_PROCESO, OUTPUT);
    pinMode(LED_AZUL_POWER, OUTPUT);

    // LED ROJO encendido
    digitalWrite(LED_ROJO_ERROR, HIGH);
    digitalWrite(LED_AZUL_POWER, LOW);
    digitalWrite(LED_AMARILLO_EN_PROCESO, LOW);
    digitalWrite(LED_VERDE_OK, LOW);

    // RFID
    SPI.begin();
    rfid.PCD_Init();

    // LCD
    lcd.init(); 
    lcd.backlight();
    lcd.setCursor(1, 0);
    lcd.print("Control Asistencia");


    // Configuración de la zona horaria para el servidor NTP
    configTime(gmtOffset_sec, 0, ntpServer);

    GSheet.printf("ESP Google Sheet Client v%s\n\n", ESP_GOOGLE_SHEET_CLIENT_VERSION);
    

    #if defined(ESP32) || defined(ESP8266)
        WiFi.setAutoReconnect(true);
    #endif

    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    unsigned long ms = millis();
    int avance = 0;
    while (WiFi.status() != WL_CONNECTED)
    {
      avance ++;
      mostrar_mensajes("Conectando WiFi...", String(espera[avance % 4]));
      delay(300);
    }

    // Una vez conectado, apago LED_ROJO y enciendo LED_AZUL
    digitalWrite(LED_ROJO_ERROR, LOW);
    digitalWrite(LED_AZUL_POWER, HIGH);

    mostrar_mensajes("Connectado IP: ", String(WiFi.localIP()));

    // Seteo de parámetros de la librería Google Sheet 
    // Establecer callBack para el estado de generación del token de acceso a la API de Google (sólo para depuración)
    GSheet.setTokenCallback(tokenStatusCallback);
    // Establezca los segundos para actualizar el token de autenticación antes de que caduque (de 60 a 3540, por defecto 300 segundos).
    GSheet.setPrerefreshSeconds(10 * 60);
    // Iniciar la generación del token de acceso para la autenticación de Google API
    GSheet.begin(CLIENT_EMAIL, PROJECT_ID, PRIVATE_KEY);


}

void loop()
{

  // Detecto si hay una nueva tarjeta presente
  if (rfid.PICC_IsNewCardPresent()){
    // Si la puedo leer
    if (rfid.PICC_ReadCardSerial()){

        MFRC522::StatusCode status;
        byte blockAddr = 4;
        String strNombre;

        // Enciendo LED_AMARILLO para indicar que esta en proceso
        digitalWrite(LED_AMARILLO_EN_PROCESO, HIGH);

        // Llamar a ready() repetidamente en bucle para comprobar y procesar la autenticación
        bool ready = GSheet.ready();

        if (ready)
        {

            FirebaseJson response;
            FirebaseJson valueRange;

            // Leer el ID
            String strUID = String(rfid.uid.uidByte[0], HEX) + " " + String(rfid.uid.uidByte[1], HEX) + " " + String(rfid.uid.uidByte[2], HEX) + " " + String(rfid.uid.uidByte[3], HEX);
            // Obtener día y hora
            String fecha, hora, hoja;
            struct tm timeinfo;

            if(!getLocalTime(&timeinfo)){
              fecha = "Error lectura fecha";
              hora = "Error lectura hora";
              mostrar_mensajes(fecha, hora);
              hoja = "Error";
            }
            else {
              fecha = String(timeinfo.tm_year +1900) + "-" + String(timeinfo.tm_mon + 1) + "-" + String(timeinfo.tm_mday);
              hora = String(timeinfo.tm_hour) + ":" + String(timeinfo.tm_min) + ":" + String(timeinfo.tm_sec);
              hoja = String(timeinfo.tm_mon + 1);
            }
            // TO DO: construir el String de Fecha
            valueRange.add("majorDimension", "COLUMNS");
            valueRange.set("values/[0]/[0]", fecha);
            valueRange.set("values/[1]/[0]", hora);
            valueRange.set("values/[2]/[0]", strUID);

            

            bool success = GSheet.values.append(&response /* returned response */, 
                                                    SPREADSHEETID /* spreadsheet Id to update */, 
                                                    hoja + "!A2" /* range to update */, 
                                                    &valueRange /* data to update */);

            // Apago LED_AMARILLO
            digitalWrite(LED_AMARILLO_EN_PROCESO, LOW);

            if (success) {
              mostrar_mensajes("Datos grabados.", "");
              // enciendo LED_VERDE
              digitalWrite(LED_VERDE_OK, HIGH);
            } else {
              mostrar_mensajes("Error!!", GSheet.errorReason());
              // enciendo LED_ROJO
              digitalWrite(LED_VERDE_OK, HIGH);

            }

        }
      rfid.PICC_HaltA();

      // Espero un segundo para que se anoticien del problema o el correcto registro de datos
      delay(1000);
      digitalWrite(LED_VERDE_OK, LOW);
      digitalWrite(LED_ROJO_ERROR, LOW);
      mostrar_mensajes("Asistencia 2024", "Siguiente ->");

    }
  }

}

void tokenStatusCallback(TokenInfo info)
{
    if (info.status == token_status_error)
    {
        GSheet.printf("Token info: type = %s, status = %s\n", GSheet.getTokenType(info).c_str(), GSheet.getTokenStatus(info).c_str());
        GSheet.printf("Token error: %s\n", GSheet.getTokenError(info).c_str());
    }
    else
    {
        GSheet.printf("Token info: type = %s, status = %s\n", GSheet.getTokenType(info).c_str(), GSheet.getTokenStatus(info).c_str());
    }
}