#include <ESP8266WiFi.h>
#include <coap_server.h>
#include <ESP8266mDNS.h>
#include <PubSubClient.h>


const char *ssid = "Minicurso_SBrT";            // SSID da sua rede Wi-Fi
const char *password = "sbrt2018";              // Senha da sua rede Wi-Fi
const char *idESP = "nodemcu1";                 // Identificação para o dispositivo
const char *mqtt_server = "raspberrypi.local";  // Endereço do servidor MQTT
bool LAMPADA1;                                  // Guarda o status da lâmpada 1


// Instâncias
WiFiClient espClient;
PubSubClient client(espClient);
coapServer coap;


// CoAP
void callback_sensor_luz(coapPacket *packet, IPAddress ip, int port, int obs);
void callback_lampada1(coapPacket *packet, IPAddress ip, int port, int obs);
// MQTT
void callback_mqtt(char* topic, byte* payload, unsigned int length);
void reconnect();


void setup() {
  Serial.begin(115200);       // Inicia a comunicação serial

  WiFi.begin(ssid, password); // Conecta ao WiFi
  while (WiFi.status() != WL_CONNECTED) {  //Aguarda até que a conexão seja estabelecida
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());

  //mDNS/DNS-SD
  if (MDNS.begin(idESP)) { // Inicia o mDNS (este dispositivo poderá ser acessado por "esp1.local")
    Serial.println("mDNS iniciado");
  }
  MDNS.addService("coap", "udp", 5683); // Adiciona o serviço CoAP para que seja descoberto pelo DNS-SD

  // Define o callback para cada endpoint (CoAP)
  coap.server(callback_lampada1, "lampada1");
  coap.server(callback_sensor_luz, "sensor1");

  // Inicia o coap server
  coap.start();

  // Lampada 1
  pinMode(D5, OUTPUT);
  digitalWrite(D5, LOW);
  LAMPADA1 = false;
  Serial.println("Lampada apagada");

  // MQTT
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback_mqtt);
}

void loop() {
  coap.loop(); //loop CoAP

  if (!client.connected()) { //Força a conexão MQTT
    reconnect();
  }
  client.loop(); //loop MQTT
}



//================CoAP==================
void callback_sensor_luz(coapPacket *packet, IPAddress ip, int port, int obs) {
  // Não implementado
}

// Callback para o endpoint lampada1
void callback_lampada1(coapPacket *packet, IPAddress ip, int port, int obs) {

  // Recupera o payload da mensagem CoAP
  char p[packet->payloadlen + 1];
  memcpy(p, packet->payload, packet->payloadlen);
  p[packet->payloadlen] = NULL;
  String message(p);

  // Muda o status da variável auxiliar
  if (message.equals("0")) {
    LAMPADA1 = false;
    Serial.println("Lampada1: Apagada");
  } else if (message.equals("1")) {
    LAMPADA1 = true;
    Serial.println("Lampada1: Acesa");
  }

  // Faz a modificação requisitada e envia a resposta
  if (LAMPADA1) {
    digitalWrite(D5, HIGH);
    if (obs == 1)
      coap.sendResponse("1");
    else
      coap.sendResponse(ip, port, "1");
  } else {
    digitalWrite(D5, LOW) ;
    if (obs == 1)
      coap.sendResponse("0");
    else
      coap.sendResponse(ip, port, "0");
  }
}


//================MQTT==================
void callback_mqtt(char* topic, byte* payload, unsigned int length) {
  Serial.print("Mensagem [");
  Serial.print(topic);
  Serial.print("]: ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
  String topico = topic;
  // Troca o status da Lâmpada 1
  if (topico.equals("lampada1") && (char)payload[0] == '1') {
    digitalWrite(D5, HIGH);
    LAMPADA1 = true;
  } else if(topico.equals("lampada1") && (char)payload[0] == '0')  {
    digitalWrite(D5, LOW);
    LAMPADA1 = false;
  }
}

void reconnect() {
  // Fica em loop até a conxão ser estabelecida
  while (!client.connected()) {
    Serial.print("Conectanto ao servidor MQTT...");
    if (client.connect(idESP)) {
        Serial.println("conectado!");
        client.subscribe("lampada1"); //Se inscreve no tópico lampada1
    } else {
      Serial.print("Falhou, rc=");
      Serial.print(client.state());
      delay(5000);
    }
  }
}
