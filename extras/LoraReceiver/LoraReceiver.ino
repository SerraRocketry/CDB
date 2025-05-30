#include <LoRa.h>
#include <SPI.h>

#define LORA_FREQ 868E6

#define SS 5
#define RST 15
#define DIO0 4

void setup()
{
    Serial.begin(115200);

    if (!setupLoRa())
    {
        Serial.println("Erro na configuração!");
        delay(3000);
        ESP.restart();
    }
    Serial.println("Iniciado.");
}

void loop()
{
    int packetSize = LoRa.parsePacket();
    if (packetSize)
    {
        while (LoRa.available())
        {
            String LoRaData = LoRa.readString();
        }
        String msg = LoraData + "," + String(LoRa.packetRssi());
        Serial.println(msg);
    }
}

bool setupLoRa()
{
    Serial.println("Configurando pinos LoRa...");
    LoRa.setPins(SS, RST, DIO0); // Para ESP32, os pinos SPI (MOSI, MISO, SCK) são definidos automaticamente pela biblioteca SPI.
    Serial.println("Inicializando LoRa...");
    if (!LoRa.begin(LORA_FREQ))
    {
        Serial.println("Falha ao inicializar o LoRa! Verifique a fiação e o módulo.");
        return false;
    }
    Serial.print("LoRa iniciado na frequência: ");
    Serial.println(LORA_FREQ);
    LoRa.setSyncWord(0xF3);
    Serial.println("Palavra de sincronização configurada para 0xF3.");
    return true;
}