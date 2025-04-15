# Aviônica - Computador de Bordo

Este projeto é o computador de bordo (CDB) para o foguete SR1500 da equipe de foguetemodelismo Serra Rocketry. O CDB utiliza sensores e módulos para monitorar a altitude, localização GPS, e controlar a abertura do paraquedas para garantir um voo seguro.

## Funcionalidades

- Leitura de altitude utilizando o sensor BMP280.
- Monitoramento de localização GPS com o módulo GPS NEO-6m.
- Comunicação via LoRa para transmissão de dados com a base operacional.
- Controle de um servo motor para abertura do paraquedas.
- Armazenamento de dados em SPIFFS.

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

## Configuração de Pinos

- **BUZZER_PIN**: 15
- **SERVO_PIN**: 13
- **LED_PIN**: 2
- **RX_GPS**: 16
- **TX_GPS**: 17
- **SS**: 5
- **RST**: 14
- **DIO0**: 4

## Bibliotecas Utilizadas

- Wire
- SPI
- Adafruit_BMP280
- ESP32Servo
- TinyGPS++
- FS
- SPIFFS
- LoRa

## Estrutura do Código

- **setup()**: Configura os sensores, módulos e inicializa as variáveis.
- **loop()**: Realiza leituras periódicas dos sensores, verifica a altitude e controla a abertura do paraquedas.
- **setupSPIFFS()**: Inicializa o sistema de arquivos SPIFFS.
- **setupBMP()**: Configura o sensor BMP280.
- **setupLoRa()**: Inicializa o módulo LoRa.
- **buzzSignal()**: Controla o buzzer e o LED para sinalização.
- **logData()**: Armazena os dados lidos em um arquivo.
- **handleParachute()**: Controla a abertura do paraquedas com base na altitude.
- **checkHighest()**: Verifica a maior altitude alcançada.
- **writeFile()**: Escreve dados no arquivo SPIFFS.
- **printBoth()**: Imprime mensagens no serial e envia via LoRa.
- **sendLoRa()**: Envia mensagens via LoRa.
- **gpsData()**: Obtém dados do GPS.
- **bmpData()**: Obtém dados do sensor BMP280.
- **getDataString()**: Concatena os dados do GPS e BMP280 em uma string.

## Códigos de apoios

Os códigos contidos em `/Extras`, são apoios ao desenvolvimento. Sendo esses códigos funcionais e testados.
