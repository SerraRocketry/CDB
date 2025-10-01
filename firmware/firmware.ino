// Inclusão de bibliotecas
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_BMP280.h>
#include <ESP32Servo.h>
#include <TinyGPS++.h>
#include "LittleFS.h"
#include <LoRa.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>

// Definições de pinos e constantes
#define INTERVAL 200

#define BMP_ADDR 0x76 // Endereço I2C padrão do BMP280

#define LORA_FREQ 868E6 // Frequência de operação
#define SS_LORA 7
#define RST_LORA 1
#define DIO0_LORA 2
#define SYNC_WORD 0xF3 // Código de sincronização

#define SERVO_PIN 10 // Pino do servo motor
#define BUZZER_PIN 0 // Pino do buzzer

#define RX_GPS 20 // RX do GPS
#define TX_GPS 21 // TX do GPS

// Instanciação de objetos
Adafruit_BMP280 BMP;  // Objeto do BMP280
Servo ParachuteServo; // Objeto do servo
TinyGPSPlus GPS;      // Objeto do GPS
Adafruit_MPU6050 MPU; // Objeto do MPU6050

// Variáveis globais
int packet_count = 0; // Contador de pacotes
unsigned long previous_millis = 0;
float previous_altitude = 0, max_altitude = 0, base_altitude = 0; // Altitudes variáveis e estáticas
float base_pressure = 0;                                          // Pressão na base
String file_name = "Dados.csv";                                   // Nome do arquivo para salvar os dados
String file_dir = "";                                             // Diretório do arquivo
String state = "";                                                // Estado do sistema
bool parachute_deployed = false;                                  // Verificação da liberação do paraquedas
const int MAXPOS = 180, MINPOS = 0;                               // Posição máxima e mínima do servo
const float ALTITUDE_DROP_THRESHOLD = 10.0;                       // Ao menos 10m abaixo do referencial máximo (ajustar se necessário)
const float ALTITUDE_THRESHOLD = 200.0;                           // Altura mínima para liberar o paraquedas (ajustar se necessário)
const float VELOCITY_THRESHOLD = 5.0;                             // Velocidade de descida mínima para liberar o paraquedas (ajustar se necessário)
const String TEAM_ID = "100";                                     // ID da equipe
sensors_event_t acc, gyr, temp;                                   // Variáveis para armazenar os dados do MPU6050

// Setup da memória LittleFS
bool setupLittleFS()
{
  if (!LittleFS.begin(true))
  {
    Serial.println("Erro ao montar LittleFS.");
    return false;
  }
  return true; // Retorna true se tudo ocorreu bem
}

// Setup do módulo BMP280
bool setupBMP()
{
  if (!BMP.begin(BMP_ADDR))
  {
    Serial.println("Falha no BMP280.");
    return false;
  }
  base_pressure = BMP.readPressure() / 100;
  previous_altitude = BMP.readAltitude(base_pressure);
  max_altitude = previous_altitude;
  base_altitude = previous_altitude;
  return true; // Retorna true se tudo ocorreu bem
}

// Setup do módulo LoRa
bool setupLoRa()
{
  LoRa.setPins(SS_LORA, RST_LORA, DIO0_LORA);
  if (!LoRa.begin(LORA_FREQ))
  {
    Serial.println("Falha ao inicializar o LoRa.");
    return false;
  }
  LoRa.setSyncWord(SYNC_WORD);
  return true; // Retorna true se tudo ocorreu bem
}

// Setup do MPU6050
bool setupMPU()
{
  if (!MPU.begin())
  {
    Serial.println("Falha no MPU6050.");
    return false;
  }
  return true; // Retorna true se tudo ocorreu bem
}

// Setup do servo motor
void setupServo()
{
  ParachuteServo.attach(SERVO_PIN);
  ParachuteServo.write(MINPOS);
}

// Sinalização com o buzzer
void buzzSignal(String signal)
{
  int frequency = 1000;   // Frequência do tom
  if (signal == "Alerta") // Alerta de erro em alguma configuração
  {
    for (int i = 0; i < 5; i++)
    {
      tone(BUZZER_PIN, frequency, 200);
      delay(200 + 150);
    }
  }
  else if (signal == "Sucesso") // Sinal de sucesso na configuração
  {
    for (int i = 0; i < 3; i++)
    {
      tone(BUZZER_PIN, frequency, 100);
      delay(100 + 100);
    }
  }
  else if (signal == "Ativado")
  {
    tone(BUZZER_PIN, frequency, 500);
  }
  else if (signal == "Beep") // Beep de funcionamento padrão
  {
    tone(BUZZER_PIN, frequency, 50);
  }
  else
  {
    Serial.println("Sinal inválido!");
  }
}

// Registra e imprime os dados do momento
// Formato: TEAM_ID,millis,count,hora,data,alt,lat,lon,sat,altp,temp,p,gp,gr,gy,ap,ar,ay
void logData(unsigned long current_millis)
{
  String readings = getDataString();                                                                                 // Obtém os dados do GPS, BMP280 e MPU6050
  String data_string = TEAM_ID + String(current_millis) + "," + String(packet_count) + "," + state + "," + readings; // String com os dados atuais
  printBoth(data_string);
  appendFile(file_dir, data_string);
  packet_count++;
}

// Verifica se a altura atual é a máxima já atingida
void checkHighest(float altitude)
{
  if (altitude > max_altitude)
  {
    max_altitude = altitude;
  }
}

// Lida com a abertura do paraquedas
void handleParachute(float altitude, float velocity)
{
  if (!parachute_deployed) // Confere se o paraquedas já foi acionado
  {
    if (altitude <= max_altitude - ALTITUDE_DROP_THRESHOLD && (altitude < ALTITUDE_THRESHOLD || abs(velocity) > VELOCITY_THRESHOLD)) // Se a altitude cair 10m abaixo do referencial máximo e for menor que 100m
    {
      ParachuteServo.write(MAXPOS);
      unsigned long startTime = millis();
      while (millis() - startTime < 500) // Aguarda 500ms para conferir se o servo motor abriu
      {
        if (ParachuteServo.read() == MAXPOS)
        {
          break;
        }
      }
      if (ParachuteServo.read() != MAXPOS)
      {
        printBoth("ERRO: Servo não abriu!");
      }
      printBoth("Paraquedas acionado. Altitude: " + String(altitude));
      parachute_deployed = true;
    }
  }
  else
  {
    buzzSignal("Ativado");
  }
  previous_altitude = altitude;
}

// Escreve os dados no arquivo - escrita
bool writeFile(const String &path, const String &data_string)
{
  File file = LittleFS.open(path, FILE_WRITE);
  if (!file) // Se houver falha ao abrir o arquivo
  {
    Serial.println("Falha ao abrir arquivo para gravação.");
    return false;
  }
  if (file.println(data_string)) // Se a escrita no arquivo for bem-sucedida
  {
    Serial.println("Arquivo escrito.");
  }
  else // Se houver falha na escrita
  {
    Serial.println("Falha na gravação do arquivo.");
    file.close();
    return false;
  }
  file.close();
  return true; // Retorna true se tudo ocorreu bem
}

// Escreve os dados no arquivo - anexação
void appendFile(const String &path, const String &message)
{
  File file = LittleFS.open(path, FILE_APPEND);
  if (!file) // Se houver falha ao abrir o arquivo
  {
    Serial.println("Falha ao abrir arquivo para anexar.");
  }
  if (file.print(message + "\n")) // Se a escrita no arquivo for bem-sucedida
  {
    Serial.println("Mensagem anexada.");
  }
  else // Se houver falha na escrita
  {
    Serial.println("Falha ao anexar mensagem.");
    file.close();
  }
  file.close();
}

// Imprime a mensagem no Serial e no LoRa
void printBoth(const String &message)
{
  Serial.println(message);
  sendLoRa(message);
}

// Processa o envio de mensagens LoRa
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
  LoRa.endPacket();
}

// Retorna os dados de hora, data, altitude, latitude, longitude, satélites
String GPSData()
{
  while (Serial1.available() > 0)
  {
    GPS.encode(Serial1.read());
  }

  String date_data = "N/A"; // String da data
  if (GPS.date.isValid())
  {
    date_data = "," + String(GPS.date.year()) + "/" +
                String(GPS.date.month()) + "/" +
                String(GPS.date.day());
  }

  String time_data = ",N/A"; // String do horário
  if (GPS.time.isValid())
  {
    time_data = "," + String(GPS.time.hour()) + ":" +
                String(GPS.time.minute()) + ":" +
                String(GPS.time.second());
  }

  String location_data = ",N/A,N/A,N/A,N/A,N/A"; // String da localização lat-long-satélites-altitude
  if (GPS.location.isValid())
  {
    location_data = String(GPS.altitude.meters()) + "," +
                    String(GPS.location.lat(), 8) + "," +
                    String(GPS.location.lng(), 8) + "," +
                    String(GPS.satellites.value());
  }

  return time_data + date_data + location_data;
}

// Retorna os dados de altitude, temperatura, pressão
String BMPData()
{
  return String(BMP.readAltitude(base_pressure)) + "," +
         String(BMP.readTemperature()) + "," +
         String(BMP.readPressure() / 100.0F);
}

// Retorna os dados de giroscópio, acelerômetro
String MPUData()
{
  MPU.getEvent(&acc, &gyr, &temp); // Lê os dados do MPU6050

  float ap = acc.acceleration.x; // Aceleração em x - pitch
  float ar = acc.acceleration.y; // Aceleração em y - roll
  float ay = acc.acceleration.z; // Aceleração em z - yaw

  float gp = gyr.gyro.x; // Giroscópio em x - pitch
  float gr = gyr.gyro.y; // Giroscópio em y - roll
  float gy = gyr.gyro.z; // Giroscópio em z - yaw

  return String(gp) + "," +
         String(gr) + "," +
         String(gy) + "," +
         String(ap) + "," +
         String(ar) + "," +
         String(ay);
}

// Retorna a string com os dados do BMP280, MPU6050 e GPS
String getDataString()
{
  return BMPData() + "," + MPUData() + "," + GPSData();
}

void setup()
{
  Serial.begin(115200);
  setupServo(); // Inicia o servo motor

  pinMode(BUZZER_PIN, OUTPUT);
  for (int i = 0; i < 5; i++)
  {
    Serial.println("Inicializando...");
    delay(1000);
  }

  Serial1.begin(9600, SERIAL_8N1, RX_GPS, TX_GPS); // Inicia o GPS (precisa ser o Serial1)
  if (!Serial1)
  {
    Serial.println("Falha ao iniciar o GPS.");
    delay(3000);
    ESP.restart();
  }
  delay(1000); // Aguarda o GPS inicializar

  while (true)
  {
    while (Serial1.available() > 0)
    {
      GPS.encode(Serial1.read());
    }
    Serial.println("Aguardando GPS...");
    if (GPS.location.isValid())
    {
      break;
    }
    delay(500);
  }

  String time_data = ""; // String do horário
  time_data = String(GPS.time.hour()) + ":" +
              String(GPS.time.minute()) + ":" +
              String(GPS.time.second());

  file_dir = "/" + time_data + "-" + file_name; // Diretório do arquivo de dados
  Serial.print("Salvando dados em: ");
  Serial.println(file_dir);
  String data_header = "TEAM_ID,millis,count,hora,data,alt,lat,lon,sat,altp,temp,p,gp,gr,gy,ap,ar,ay"; // Cabeçalho do arquivo
  if (!(setupLittleFS() && writeFile(file_dir, data_header)))                                          // Inicia a memória interna
  {
    Serial.println("Erro no sistema de arquivos!");
    buzzSignal("Alerta");
    delay(3000);
    ESP.restart();
  }

  if (!(setupBMP() && setupMPU() && setupLoRa())) // Inicia os módulos BMP, MPU6050 e LoRa
  {
    Serial.println("Erro na configuração dos módulos!");
    buzzSignal("Alerta");
    delay(3000);
    ESP.restart();
  }

  buzzSignal("Sucesso");
}

void loop()
{
  unsigned long current_millis = millis();
  if (current_millis - previous_millis >= INTERVAL) // A cada 200ms
  {
    logData(current_millis);
    float altitude = BMP.readAltitude(base_pressure);
    float velocity = (altitude - previous_altitude) / ((current_millis - previous_millis) / 1000.0); 
    checkHighest(altitude);
    handleParachute(altitude, velocity);
    previous_millis = current_millis;
  }
}
