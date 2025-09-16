#include <WiFi.h>
#include <math.h>
#define PI 3.141592654

double h_sensor = 20;
double h_total = 0;
double distancia = 0;
double distancia_margem;
double angulo = 45;
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

  angulo = (angulo * PI)/180;

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

  // Lê distância
  distancia = lerDistancia();
  distancia_margem = h_sensor/cos(angulo);


  h_total = h_sensor * distancia / distancia_margem;

  margem = h_total - h_sensor;

  // Resposta HTML
  String html = "<!DOCTYPE html><html>";
  html += "<head><meta http-equiv='refresh' content='1'>";
  html += "<title>HC-SR04 ESP32</title></head>";
  html += "<body style='font-family:Arial; text-align:center;'>";
  html += "<h1>Calculo de Distancia do Nivel do Rio</h1>";
  html += "<h2>Distancia da agua ate a margem:  <span id='distancia'>" + String(margem, 1) + "</span> cm</h2>";
  html += "<p>(Atualiza a cada 1 segundo)</p>";
  html += "</body></html>";

  client.println("HTTP/1.1 200 OK");
  client.println("Content-type:text/html");
  client.println("Connection: close");
  client.println();
  client.println(html);
  client.stop();
  Serial.println("Cliente desconectado.");
}

// ---- Função para leitura do HC-SR04 ----
float lerDistancia() {
  // Gera pulso no TRIG
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  // Mede tempo de resposta no ECHO
  long duracao = pulseIn(ECHO_PIN, HIGH, 30000); // timeout 30ms (~5m)

  // Converte em cm
  float distancia = duracao * 0.0343 / 2; // velocidade som ~343m/s
  return distancia;
}
