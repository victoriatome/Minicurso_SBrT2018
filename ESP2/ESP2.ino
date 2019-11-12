#include <ESP8266WiFi.h>
#include <coap_client.h>
#include <ESP8266mDNS.h>


const char *ssid = "Minicurso_SBrT";            // SSID da sua rede Wi-Fi
const char *password = "sbrt2018";              // Senha da sua rede Wi-Fi
const char *idESP = "nodemcu2";                 // Identificação para o dispositivo
bool LAMPADA1;                                  // Guarda o status da lâmpada 1

// Instância do CoAP client
coapClient coap;


//IP e porta padrão do servidor que receberá a requisição (utilizado apenas se nenhum serviço for encontrado)
IPAddress ip(192,168,10,5);
int port = 5683;


// Respoda de callback do coap client
void callback_response(coapPacket &packet, IPAddress ip, int port) {
    char p[packet.payloadlen + 1];
    memcpy(p, packet.payload, packet.payloadlen);
    p[packet.payloadlen] = NULL;
    String message(p);
   
    if (message.equals("0")) {
      LAMPADA1 = false;
      Serial.println("Lampada1: Apagada");
    } else if (message.equals("1")) {
      LAMPADA1 = true;
      Serial.println("Lampada1: Acesa");
    }

    Serial.println(p); // Imprime o payload da resposta na serial
}

void setup () {
 
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password); // Conecta ao WiFi
  while (WiFi.status() != WL_CONNECTED) {  //Aguarda até que a conexão seja estabelecida
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());

  if (MDNS.begin(idESP)){  // Inicia o mDNS (este dispositivo poderá ser acessado por "esp2.local")
    Serial.println("mDNS iniciado");
  }

  //Descobre qual ip e porta do servidor coap (esp1)
  int n = MDNS.queryService("coap","udp");
  if (n == 0) {
    Serial.println("serviço não encontrado");
  }
  else {
    Serial.print(n);
    Serial.println(" serviço(s) encrontrado(s)");
    for (int i = 0; i < n; ++i) {
      // Mostra na serial o(s) serviço(s) encontrado(s)
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.print(MDNS.hostname(i));
      Serial.print(" (");
      Serial.print(MDNS.IP(i));
      Serial.print(":");
      Serial.print(MDNS.port(i));
      Serial.println(")");
    }
    // Utiliza apenas o endereço do primeiro serviço encontrado para realizar a comunicação.
    ip = MDNS.IP(0);
    port = MDNS.port(0);
  }

  
  // Define o callback de resposta.
  coap.response(callback_response);
  
  // Inicia o coap client
  coap.start();
  
  //Envia requisição de observação para o servidor CoAP
  int msgid= coap.observe(ip,port,"lampada1",0);
}
 
void loop() {
  coap.loop();
  delay(1000);
}

