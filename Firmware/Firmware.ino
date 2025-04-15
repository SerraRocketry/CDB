// Inclusão de bibliotecas
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_BMP280.h>
#include <ESP32Servo.h>
#include <TinyGPS++.h>
#include "FS.h"
#include "SPIFFS.h"
#include <LoRa.h>

// Definições de pinos e constantes
#define PRESSAO_NIVEL_MAR 1013.25

#define INTERVALO_LEITURAS 100

#define LORA_FREQ 868E6

#define SS 5
#define RST 14
#define DIO0 4

#define BUZZER_PIN 15
#define SERVO_PIN 13
#define LED_PIN 2

#define RX_GPS 16
#define TX_GPS 17

// Instanciação de objetos
Adafruit_BMP280 bmp;
Servo escotilha;
HardwareSerial neogps(2);
TinyGPSPlus gps;

// Variáveis globais
unsigned long previousMillis = 0;
float previousAltitude = 0, maxAltitude = 0;
baseAltitude = 0;
const int MAXPOS = 180, MINPOS = 0;
String filedir = "/Dados.txt";

const float ALTITUDE_DROP_THRESHOLD = 10.0;
const float ALTITUDE_THRESHOLD = 100.0;

bool parachuteDeployed = false;

void setup()
{
  Serial.begin(115200);

  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);

  if (!(setupSPIFFS() && setupBMP() && setupLoRa()))
  {
    Serial.println("Erro na configuração!");
    buzzSignal("Alerta");
    delay(3000);
    ESP.restart();
  }

  setupServo();

  neogps.begin(9600, SERIAL_8N1, RX_GPS, TX_GPS);

  writeFile(filedir, "");

  buzzSignal("Sucesso");
}

void loop()
{
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= INTERVALO_LEITURAS) // A cada 100ms
  {
    logData(currentMillis);
    float altitude = bmp.readAltitude(PRESSAO_NIVEL_MAR);
    checkHighest(altitude);
    handleParachute(altitude, currentMillis);
    previousMillis = currentMillis;
  }
}

bool setupSPIFFS()
{
  if (!SPIFFS.begin(true))
  {
    Serial.println("Erro ao montar SPIFFS.");
    return false;
  }
  return true;
}

bool setupBMP()
{
  if (!bmp.begin(0x76))
  {
    Serial.println("Falha no BMP280.");
    return false;
  }
  previousAltitude = bmp.readAltitude(PRESSAO_NIVEL_MAR);
  maxAltitude = previousAltitude;
  baseAltitude = previousAltitude;
  return true;
}

bool setupLoRa()
{
  LoRa.setPins(SS, RST, DIO0);
  if (!LoRa.begin(LORA_FREQ))
  {
    Serial.println("Falha ao inicializar o LoRa!");
    return false;
  }
  LoRa.setSyncWord(0xF3);
  return true;
}

void setupServo()
{
  escotilha.attach(SERVO_PIN);
  escotilha.write(MINPOS);
}

void buzzSignal(String signal)
{
  if (signal == "Alerta")
  {
    for (int i = 0; i < 5; i++)
    {
      digitalWrite(BUZZER_PIN, HIGH);
      digitalWrite(LED_PIN, HIGH);
      delay(500);
      digitalWrite(BUZZER_PIN, LOW);
      digitalWrite(LED_PIN, LOW);
      delay(500);
    }
  }
  else if (signal == "Sucesso")
  {
    for (int i = 0; i < 3; i++)
    {
      digitalWrite(BUZZER_PIN, HIGH);
      digitalWrite(LED_PIN, HIGH);
      delay(100);
      digitalWrite(BUZZER_PIN, LOW);
      digitalWrite(LED_PIN, LOW);
      delay(100);
    }
  }
}

void logData(unsigned long currentMillis)
{
  String dataString = String(currentMillis) + ";" + getDataString();
  writeFile(filedir, dataString);
  printBoth(dataString);
}

void handleParachute(float altitude, unsigned long currentMillis)
{
  if (!parachuteDeployed)
  {
    if (altitude <= maxAltitude - ALTITUDE_DROP_THRESHOLD && altitude < ALTITUDE_THRESHOLD)
    {
      digitalWrite(BUZZER_PIN, HIGH);

      escotilha.write(MAXPOS);
      unsigned long startTime = millis();
      while (millis() - startTime < 500)
      {
        if (escotilha.read() == MAXPOS)
        {
          break;
        }
      }
      if (escotilha.read() != MAXPOS)
      {
        printBoth("ERRO: Servo não abriu a escotilha!");
      }

      printBoth("Paraquedas acionado. Altitude: " + String(altitude) + ", Velocidade de queda: " + String(rateOfDescent) + " m/s");

      parachuteDeployed = true;
    }
  }
  previousAltitude = altitude;
}

void checkHighest(float altitude)
{
  if (altitude > maxAltitude)
  {
    maxAltitude = altitude;
  }
}

void writeFile(const String &path, const String &dataString)
{
  File dataFile = SPIFFS.open(path, FILE_APPEND);
  if (dataFile)
  {
    dataFile.println(dataString);
    dataFile.close();
  }
  else
  {
    printBoth("Erro ao abrir o arquivo.");
  }
}

void printBoth(const String &message)
{
  Serial.println(message);
  sendLoRa(message);
}

void sendLoRa(const String &message)
{
  LoRa.beginPacket();
  LoRa.print(message);
  if (LoRa.endPacket())
  {
    Serial.println("Mensagem LoRa enviada.");
  }
  else
  {
    Serial.println("ERRO ao enviar mensagem LoRa!");
  }
}

String gpsData()
{
  String locationData = "N/A;N/A;N/A;N/A;N/A";
  if (gps.location.isValid())
  {
    locationData = String(gps.location.lat(), 6) + ";" +
                   String(gps.location.lng(), 6) + ";" +
                   String(gps.satellites.value()) + ";" +
                   String(gps.speed.kmph()) + ";" +
                   String(gps.altitude.meters());
  }

  String dateData = ";N/A";
  if (gps.date.isValid())
  {
    dateData = ";" + String(gps.date.year()) + "/" +
               String(gps.date.month()) + "/" +
               String(gps.date.day());
  }

  String timeData = ";N/A";
  if (gps.time.isValid())
  {
    timeData = ";" + String(gps.time.hour()) + ":" +
               String(gps.time.minute()) + ":" +
               String(gps.time.second());
  }

  return locationData + dateData + timeData;
}

String bmpData()
{
  return String(bmp.readAltitude(PRESSAO_NIVEL_MAR)) + ";" +
         String(bmp.readPressure() / 100.0F) + ";" +
         String(bmp.readTemperature());
}

String getDataString()
{
  return gpsData() + ";" + bmpData();
}