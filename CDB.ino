// Inclusão de bibliotecas
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_BMP280.h>
#include <ESP32Servo.h>
#include <TinyGPS++.h>
#include "FS.h"
#include "SPIFFS.h"
#include <LoRa.h>
#include <TinyGsmClient.h>

// Definições de pinos e constantes
#define PRESSAO_NIVEL_MAR 1013.25

#define BUZZER_PIN 0
#define SERVO_PIN 16
#define LED_PIN 2

#define INTERVALO_LEITURAS 100

#define RX_GPS 16
#define TX_GPS 17

#define RX_SIM800 26
#define TX_SIM800 27

#define LORA_FREQ 868E6

// Instanciação de objetos
Adafruit_BMP280 bmp;
Servo escotilha;
HardwareSerial neogps(1);
HardwareSerial sim800(2);
TinyGsm modemGSM(sim800);
TinyGsmClient gsmClient(sim800);
TinyGPSPlus gps;

// Variáveis globais
unsigned long previousMillis = 0;
unsigned long previousMillisSMS = 0;
float previousAltitude = 0, maxAltitude = 0;
const int MAXPOS = 180, MINPOS = 0;
const String TEL = "+552199999999";
String filedir = "/Dados.txt";
bool sim800_ready = false;

const float ALTITUDE_DROP_THRESHOLD = 10.0;
const float MINIMUM_DESCENT_RATE = 2.0;

bool parachuteDeployed = false;

// Configurações do GPRS (Operadora)
const char apn[] = ""; 
const char user[] = "";
const char pass[] = "";

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

  sim800_ready = setupSIM800();

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
  if (sim800_ready && currentMillis - previousMillisSMS >= INTERVALO_LEITURAS * 600) // A cada 1min
  {
    String gpsInfo = gpsData();
    if (gps.location.isValid())
    {
      sendSMS(gpsInfo);
      previousMillisSMS = currentMillis;
    }
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
  return true;
}

bool setupLoRa()
{
  if (!LoRa.begin(LORA_FREQ))
  {
    Serial.println("Falha ao inicializar o LoRa!");
    return false;
  }
  return true;
}

bool setupSIM800()
{
  sim800.begin(9600, SERIAL_8N1, RX_SIM800, TX_SIM800);

  // Mostra informação sobre o modem
  Serial.println(modemGSM.getModemInfo());

  // Inicializa o modem
  if (!modemGSM.restart())
  {
    Serial.println("Falha ao reiniciar o modem.");
    return false;
  }

  // Espera pela rede
  if (!modemGSM.waitForNetwork())
  {
    Serial.println("Falha ao conectar à rede.");
    return false;
  }

  // Conecta à rede gprs (APN, usuário, senha)
  if (!modemGSM.gprsConnect(apn, user, pass))
  {
    Serial.println("Falha ao conectar ao GPRS.");
    return false;
  }

  if (!checkSIM800())
  {
    Serial.println("SIM800 não conectado.");
    return false;
  }

  Serial.println("SIM800 pronto.");
  return true;
}

bool checkSIM800()
{
  sim800.print("AT+CREG?\r");
  unsigned long start = millis();
  String response = "";

  while (millis() - start < 1000)
  { // Aguarda até 1s sem travar o código
    if (sim800.available())
    {
      response += char(sim800.read());
    }
  }

  Serial.println("Resposta SIM800: " + response);
  return response.indexOf(",1") != -1 || response.indexOf(",5") != -1;
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
    float rateOfDescent = 0;

    if (currentMillis - previousMillis > 0)
    {
      rateOfDescent = (previousAltitude - altitude) / ((currentMillis - previousMillis) / 1000.0);
    }

    if (altitude <= maxAltitude - ALTITUDE_DROP_THRESHOLD &&
        rateOfDescent > MINIMUM_DESCENT_RATE)
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

      sendSMS("Paraquedas acionado. Altitude: " + String(altitude) + ", Taxa: " + String(rateOfDescent) + " m/s");
      printBoth("Paraquedas acionado. Altitude: " + String(altitude) + ", Taxa: " + String(rateOfDescent) + " m/s");

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

void sendSMS(const String &message)
{
  sim800.print("AT+CMGS=\"" + TEL + "\"\r");
  delay(1000);
  sim800.print(message);
  delay(100);
  sim800.write(0x1A);
  printBoth("SMS enviado.");
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