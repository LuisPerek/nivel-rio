#include <WiFi.h>
#include <math.h>
#define PI 3.141592654

// Variáveis ajustáveis
double h_sensor = 21;
double h_total = 0;
double distancia = 0;
double distancia_margem;
double angulo = 20;  // graus
double margem = 0;

// ---- CONFIG Wi-Fi ----
const char* ssid = "marcinhuk-EXT";  
const char* password = "marcinhuk@34228692"; 

// ---- CONFIG HC-SR04 ----
#define TRIG_PIN 17
#define ECHO_PIN 18

WiFiServer server(80);

void setup() {
  Serial.begin(115200);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  // Conecta no Wi-Fi
  Serial.print("Conectando ao Wi-Fi: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConectado!");
  Serial.print("IP do ESP32: ");
  Serial.println(WiFi.localIP());

  server.begin();
}

void loop() {
  WiFiClient client = server.available();
  if (!client) return;

  Serial.println("Novo cliente conectado!");
  String request = client.readStringUntil('\r');
  client.flush();

  // --- Tratamento REST ---
  if (request.indexOf("GET /status") >= 0) {
    enviarStatus(client);
  } 
  else if (request.indexOf("GET /set") >= 0) {
    tratarSet(client, request);
  }
  else {
    enviarPagina(client);  // default: envia página HTML
  }

  client.stop();
  Serial.println("Cliente desconectado.");
}

// ---- Função para leitura do HC-SR04 ----
float lerDistancia() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long duracao = pulseIn(ECHO_PIN, HIGH, 30000); // timeout 30ms
  float distancia = duracao * 0.0343 / 2; 
  return distancia;
}

// ---- Página HTML (visualização) ----
void enviarPagina(WiFiClient &client) {
  distancia = lerDistancia();
  double anguloRad = (angulo * PI) / 180.0;
  distancia_margem = h_sensor / cos(anguloRad);
  h_total = h_sensor * distancia / distancia_margem;
  margem = h_total - h_sensor;

  String html = "<!DOCTYPE html><html>";
  html += "<head><meta http-equiv='refresh' content='2'>";
  html += "<title>HC-SR04 ESP32</title></head>";
  html += "<body style='font-family:Arial; text-align:center;'>";
  html += "<h1>Calculo de Distancia do Nivel do Rio</h1>";
  html += "<h2>Margem: <span id='distancia'>" + String(margem, 1) + "</span> cm</h2>";
  html += "<p>altura do sensor = " + String(h_sensor) + " cm | angulo = " + String(angulo) + " graus</p>";
  html += "<p>(Atualiza a cada 2 segundos)</p>";
  html += "</body></html>";

  client.println("HTTP/1.1 200 OK");
  client.println("Content-type:text/html");
  client.println("Connection: close");
  client.println();
  client.println(html);
}

// ---- API: /status ----
void enviarStatus(WiFiClient &client) {
  distancia = lerDistancia();
  double anguloRad = (angulo * PI) / 180.0;
  distancia_margem = h_sensor / cos(anguloRad);
  h_total = h_sensor * distancia / distancia_margem;
  margem = h_total - h_sensor;

  String json = "{";
  json += "\"h_sensor\":" + String(h_sensor,1) + ",";
  json += "\"angulo\":" + String(angulo,1) + ",";
  json += "\"distancia\":" + String(distancia,1) + ",";
  json += "\"margem\":" + String(margem,1);
  json += "}";

  client.println("HTTP/1.1 200 OK");
  client.println("Content-type:application/json");
  client.println("Connection: close");
  client.println();
  client.println(json);
}

// ---- API: /set ----
void tratarSet(WiFiClient &client, String request) {
  String resposta = "Parametro invalido";

  if (request.indexOf("name=h_sensor") >= 0) {
    int pos = request.indexOf("value=");
    if (pos > 0) {
      h_sensor = request.substring(pos+6).toDouble();
      resposta = "h_sensor atualizado para " + String(h_sensor);
    }
  } else if (request.indexOf("name=angulo") >= 0) {
    int pos = request.indexOf("value=");
    if (pos > 0) {
      angulo = request.substring(pos+6).toDouble();
      resposta = "angulo atualizado para " + String(angulo);]
    }
  }

  client.println("HTTP/1.1 200 OK");
  client.println("Content-type:text/plain");
  client.println("Connection: close");
  client.println();
  client.println(resposta);
}
