#include "WiFi.h"
#include <PubSubClient.h>

// Parametros de conexão WiFi
const char *ssid = "(nome da rede)";
const char *password = "(senha da rede)";

// Parametros de conexão MQTT
const char *mqtt_broker = "0.tcp.sa.ngrok.io";
const char *mqtt_topic = "toqueInfo";
const char *mqtt_username = "sensorDeToque";
const char *mqtt_password = "(senha do topico)";
const int mqtt_port = 12272;//porta do servidor MQTT

// Pino do sensor
const int pinSensorDigital = 35;

// Objetos
WiFiClient espClient;
PubSubClient client(espClient);

// Variáveis
bool mqttStatus = false;

long runTime = 0;

int isTouched = LOW;

void setup() {
  Serial.begin(9600);

  // Conectar ao WiFi
  WiFi.begin(ssid, password);
  Serial.println("Conectando ao WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi conectado");

  // Conectar ao MQTT
  mqttStatus = connectMQTT();

  // Configurar pino do sensor
  pinMode(pinSensorDigital, INPUT);
}

void loop() {
  if (mqttStatus) {
    // Leitura do sensor digital
    int estadoDigital = digitalRead(pinSensorDigital);

    // Publicar estado do sensor no tópico MQTT
    if(estadoDigital==HIGH) {
      if(estadoDigital!=isTouched) {
        isTouched = HIGH;
        String payload = "Sensor acionado";
        client.publish(mqtt_topic, payload.c_str());
      }
      runTime = millis();
    } 
    else if (millis() - runTime > 50) {
      if(estadoDigital!=isTouched) {
        isTouched = LOW;
        String payload = "Sensor não acionado";
        client.publish(mqtt_topic, payload.c_str());
      }
    }

    // Aguardar 1 segundo para evitar mensagens repetidas
    //delay(5);
  }

  // Manter a conexão MQTT
  client.loop();
}

bool connectMQTT() {
  byte tentativa = 0;
  client.setServer(mqtt_broker, mqtt_port);

  do {
    //definição do ID do cliente MQTT
    String client_id = "BOBSIEN-";
    client_id += String(WiFi.macAddress());

    if (client.connect(client_id.c_str(), mqtt_username, mqtt_password)) {
      Serial.println("Conexão MQTT estabelecida");
    } else {
      Serial.print("Falha ao conectar ao MQTT. Estado: ");
      Serial.println(client.state());
      delay(2000);
    }

    tentativa++;
  } while (!client.connected() && tentativa < 5);

  if (tentativa < 5) {
    // Inscrever no tópico MQTT
    client.subscribe(mqtt_topic);
    return true;
  } else {
    Serial.println("Não foi possível estabelecer a conexão MQTT");
    return false;
  }
}
