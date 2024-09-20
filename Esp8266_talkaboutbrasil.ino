#include <WiFi.h>
#include <HTTPClient.h>
#include <DFPlayerMini_Fast.h>
#include <TimeLib.h>
#include <Wire.h>
#include <SPI.h>
#include <ESP32WebServer.h>

// Definição de pinos e variáveis
#define RX_PIN 16
#define TX_PIN 17
DFPlayerMini_Fast myDFPlayer;

const char* ssid = "NOME_DO_WIFI";     // Coloque o nome da sua rede WiFi
const char* password = "SENHA_DO_WIFI"; // Coloque a senha da sua rede WiFi

String apiKey = "SUA_API_KEY_OPENWEATHER";  // Chave da API de Clima
String city = "SUA_CIDADE";                 // Cidade desejada
String units = "metric";                    // Unidades (métricas = Celsius)
String weatherUrl = "";

// Controle do tempo e anúncios
unsigned long previousMillis = 0;
const long interval = 1800000;  // Atualiza a cada 30 minutos (em milissegundos)

ESP32WebServer server(80);

void setup() {
  Serial.begin(115200);
  
  // Iniciar WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Conectando ao WiFi...");
  }
  Serial.println("WiFi conectado!");

  // Configurar DFPlayer Mini
  Serial2.begin(9600, SERIAL_8N1, RX_PIN, TX_PIN); 
  if (!myDFPlayer.begin(Serial2)) {  // Use Serial2 para DFPlayer
    Serial.println("DFPlayer não encontrado.");
    while(true);
  }
  myDFPlayer.volume(10);  // Definir volume entre 0 e 30

  weatherUrl = "http://api.openweathermap.org/data/2.5/weather?q=" + city + "&units=" + units + "&appid=" + apiKey;

  // Iniciar o servidor
  server.on("/", HTTP_GET, handleRoot);
  server.on("/upload", HTTP_POST, handleUpload);
  server.begin();
}

void loop() {
  server.handleClient();  // Escuta por clientes
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    obterClima();  // Obter clima e reproduzir o áudio
    tocarAnuncio(); // Reproduz anúncios de arquivos enviados
  }
}

void handleRoot() {
  String html = "<html><body><h2>Envio de Anúncios para o ESP32</h2>";
  html += "<form action=\"/upload\" method=\"post\" enctype=\"multipart/form-data\">";
  html += "Selecione o anúncio para upload: ";
  html += "<input type=\"file\" name=\"fileToUpload\" id=\"fileToUpload\">";
  html += "<input type=\"submit\" value=\"Carregar Anúncio\" name=\"submit\">";
  html += "</form></body></html>";
  server.send(200, "text/html", html);
}

void handleUpload() {
  if (server.args() > 0) { // Se o arquivo foi enviado
    HTTPUpload& upload = server.upload();
    if (upload.status == UPLOAD_FILE_START) {
      String filename = upload.filename;
      Serial.printf("Upload de arquivo: %s\n", filename.c_str());
      // Aqui você pode adicionar lógica para salvar o arquivo
      // Exemplo: se estiver usando SPIFFS, salve o arquivo em SPIFFS
    } else if (upload.status == UPLOAD_FILE_WRITE) {
      // Continue escrevendo o arquivo
      Serial.printf("Escrevendo arquivo: %s\n", upload.currentSize);
    } else if (upload.status == UPLOAD_FILE_END) {
      Serial.printf("Upload completo: %s (%u bytes)\n", upload.filename.c_str(), upload.totalSize);
      server.send(200, "text/plain", "Arquivo recebido com sucesso!");
    }
  }
}

void obterClima() {
  if(WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(weatherUrl);
    int httpCode = http.GET();
    
    if (httpCode > 0) {
      String payload = http.getString();
      Serial.println(payload);
      
      // Processar os dados JSON da API
      myDFPlayer.play(1);  // Toca o primeiro arquivo de clima gravado
    } else {
      Serial.println("Erro na requisição HTTP");
    }
    
    http.end();
  }
}

void tocarAnuncio() {
  myDFPlayer.play(2);  // Exemplo: Toca o segundo arquivo de anúncio carregado
}
