#include <WiFi.h>

// Credenciais de acesso à rede.
const char *ssid = "Servidor ESP32";
const char *password = "12345678";

void setup() {
  // Inicializa comunicação serial.
  Serial.begin(115200);

  // Cria ponto de acesso wireless.
  WiFi.softAP(ssid, password);
  Serial.println("Criando ponto de acesso WiFi..");
  Serial.println(WiFi.softAPIP());
}

void loop() {
}
