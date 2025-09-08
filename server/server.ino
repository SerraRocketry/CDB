#include <ESPAsyncWebServer.h>
#include <LittleFS.h>
#include <WiFi.h>

// Credenciais de acesso à rede.
const char *ssid = "Servidor ESP32";
const char *password = "12345678";

// Instancia um servidor http que escutará na porta 80.
AsyncWebServer server(80);

void setServerRoutes() {
  // Rotas básicas de acesso ao site para o usuário
  server.on("/", HTTP_GET, [](AsyncWebServerRequest* request) {
    request->send(LittleFS, "/index.html", "text/html");
  });

  server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest* request) {
    request->send(LittleFS, "/style.css", "text/css");
  });
}

void setup() {
  // Inicializa comunicação serial.
  Serial.begin(115200);

  // Inicializa do Sistema de Arquivos Interno.
  if (!LittleFS.begin()) {
    Serial.println("Um erro ocorreu ao montar LittleFS");
    return;
  }

  // Cria ponto de acesso wireless.
  WiFi.softAP(ssid, password);
  Serial.println("Criando ponto de acesso WiFi..");
  Serial.println(WiFi.softAPIP());

  // Configura as rotas do servidor e o inicializa.
  setServerRoutes();
  server.begin();
}

void loop() {
}
