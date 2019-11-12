#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <SimpleMovingAverage.h> 

SimpleMovingAverage avg;

const char* ssid = "SUA_REDE";
const char* password = "SUA_SENHA";
float suave;

void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.print("connecting to ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(700);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  
  avg.begin();
  suave = avg.update(WiFi.RSSI());
}

int getIndicator() {
  if (WiFi.status() != WL_CONNECTED)
    return -1;
  int dBm = suave;
  if (dBm <= -100)
    return 0;
  if (dBm >= -50)
    return 100;
  return 2 * (dBm + 100);
}

void loop() {
  suave = avg.update(WiFi.RSSI()); 
  static int previousIndicator = -1;
  int indicator = getIndicator();
  if (indicator != previousIndicator) { //If the quality changed since last print, print new quality and RSSI
    if (indicator != -1)
      Serial.print("WiFi Indicator:"); Serial.print(indicator);
      Serial.print("RSSI smoothed:"); Serial.println(suave);
    previousIndicator = indicator;
  }
  delay(100);
}
