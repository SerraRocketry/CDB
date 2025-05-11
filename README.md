# Aviônica - Computador de Bordo

Este projeto é o computador de bordo (CDB) para o foguete SR1500 da equipe de foguetemodelismo Serra Rocketry. O CDB utiliza sensores e módulos para monitorar a altitude, localização GPS, e controlar a abertura do paraquedas para garantir um voo seguro.

## Funcionalidades

- Leitura de altitude utilizando o sensor BMP280.
- Monitoramento de localização GPS com o módulo GPS NEO-6m.
- Comunicação via LoRa para transmissão de dados com a base operacional.
- Controle de um servo motor para abertura do paraquedas.
- Armazenamento de dados em SPIFFS.
- Monitoramento de aceleração e giroscópio utilizando o sensor MPU6050.

## Hardware Utilizado

- ESP32
- Sensor de pressão BMP280
- Módulo GPS
- Módulo LoRa
- Servo motor
- Buzzer
- LED
- Step down
- Baterias Li-Po
- Sensor MPU6050

## Configuração de Pinos

- **BUZZER_PIN**: 15
- **SERVO_PIN**: 13
- **LED_PIN**: 2
- **RX_GPS**: 16
- **TX_GPS**: 17
- **SS_LORA**: 5
- **RST_LORA**: 14
- **DIO0_LORA**: 4

## Bibliotecas Utilizadas

- Wire
- SPI
- Adafruit_BMP280
- ESP32Servo
- TinyGPS++
- FS
- SPIFFS
- LoRa
- Adafruit_MPU6050
- Adafruit_Sensor

## Estrutura do Código

- **setup()**: Configura os sensores, módulos e inicializa as variáveis.
- **loop()**: Realiza leituras periódicas dos sensores, verifica a altitude e controla a abertura do paraquedas.
- **setupSPIFFS()**: Inicializa o sistema de arquivos SPIFFS.
- **setupBMP()**: Configura o sensor BMP280.
- **setupLoRa()**: Inicializa o módulo LoRa.
- **setupMPU()**: Configura o sensor MPU6050.
- **setupServo()**: Configura o servo motor.
- **buzzSignal()**: Controla o buzzer e o LED para sinalização.
- **logData()**: Armazena os dados lidos em um arquivo.
- **handleParachute()**: Controla a abertura do paraquedas com base na altitude.
- **checkHighest()**: Verifica a maior altitude alcançada.
- **writeFile()**: Escreve dados no arquivo SPIFFS.
- **appendFile()**: Anexa dados ao arquivo SPIFFS.
- **printBoth()**: Imprime mensagens no serial e envia via LoRa.
- **sendLoRa()**: Envia mensagens via LoRa.
- **GPSData()**: Obtém dados do GPS (latitude, longitude, satélites, altitude, data e hora).
- **BMPData()**: Obtém dados do sensor BMP280 (altitude e pressão).
- **MPUData()**: Obtém dados do sensor MPU6050 (aceleração e giroscópio).
- **getDataString()**: Concatena os dados do GPS, BMP280 e MPU6050 em uma string.

## Códigos de Apoio

Os códigos contidos em `/Extras` são apoios ao desenvolvimento. Esses códigos são funcionais e testados.