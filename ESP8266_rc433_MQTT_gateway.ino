#define HOSTNAME "ESP_RF433"          // Set the host name to whatever is appropriate. 

#include <ESP8266mDNS.h>
#include <ArduinoOTA.h>
#include "mqtt.h"
#include "rc.h"
#include "transl.h"

#define LIMIT true
#define RXPIN D2
#define TXPIN D1

long lasttime = millis() + 200L;
unsigned long lastvalue = 0;
TRANSLATE trsl;


/*  Configuration values to be filled in as needed  */
#define SSID  "..";
#define PASS  "..";
#define WSHOST  "..";
#define WSPORT  81;
#define MQTT_CLIENT ESP.getChipId()
#define MQTT_SERVER "...";
#define MQTT_PORT 17332;
#define MQTT_USER "...";
#define MQTT_PASS "..";

#include <init.h> //  I use this library file to redefine the configuration values - remove if config data are defined abowe

const char *ssid = SSID;
const char *pass = PASS;
char* mqtt_client = "ESP123"; //String(ESP.getChipId());
char* mqtt_host = MQTT_SERVER;
uint16_t mqtt_port = MQTT_PORT;
char* mqtt_user = MQTT_USER;
char* mqtt_pass = MQTT_PASS;

/*
   MQTT events
*/
void mqttOnConnecting() {
  Serial.print("Attempting MQTT connection...");
};

void mqttOnConnected() {
  Serial.println("connected");
  mqtt.subscribe("esp8266/#");
};

void mqttOnFailed(int state) {
  Serial.print("failed, rc=");
  Serial.print(state);
  Serial.println(" try again in 5 seconds");
};

void mqttOnMessage(char* topic, byte * payload, unsigned int length) {
  String value = "";
  String sp = " ";
  topicconverter.set(topic);
  for (int i = 0; i < length; i++) value += (char)payload[i];
  //Serial.println(topicconverter.plain + sp + value + sp + topicconverter.cmd);
  if (trsl.encode(topicconverter.plain, atoi(&value[0]))) {
    //Serial.printf("Read: %8d %d %d \n", trsl.code, trsl.bits, trsl.prot);
    if (topicconverter.cmd == "/cmd") {
      rcsend(trsl.prot, trsl.code);
      char pbuf[30];
      sprintf(pbuf, "%s/state", topicconverter.plain.c_str());
      mqtt.publish((const char *)pbuf, (trsl.state ? "1" : "0"), true);
    }
  }
};
void onRcData(unsigned long val, int  numbits, uint8_t prot) {
#if LIMIT
  static uint32_t lastvalue;
  static uint8_t count;
  static uint32_t lasttime;
  bool res = false;
  if (millis()>lasttime) count = 0;
  if (val==lastvalue) count++;
  if (count==2) res=true; //  we got the same twice
  lastvalue = val;
  lasttime = millis()+200L;
  if (!res) return;
#endif
  if (!trsl.decode(val)) return;
  Serial.printf("value: %11d %d %d %s %d\n", (unsigned long)val, numbits, prot, trsl.topic.c_str(), trsl.state);
  if (!trsl.encode(trsl.topic, trsl.state)) return;
  //Serial.printf("Read: %8d %d %d \n", trsl.code, trsl.bits, trsl.prot);
  char pbuf[30];
  sprintf(pbuf, "%s/state", trsl.topic.c_str());
  mqtt.publish((const char *)pbuf, (trsl.state ? "1" : "0"), true);
}
/*
   WiFi begin
*/
void wifibegin() {
  //String routername = HOSTNAME;
  //char* ourname = &routername[0];
  //  wifi_station_set_hostname(HOSTNAME);  //sdk, non-Arduino function
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  if (MDNS.begin(HOSTNAME)) {
    Serial.println("MDNS responder started");
  }
  ArduinoOTA.begin();
}

unsigned long x = -2;
void setup() {
  Serial.begin(115200);
  delay(100);
  wifibegin();
  mqtt.onConnecting(mqttOnConnecting);
  mqtt.onConnected(mqttOnConnected);
  mqtt.onConnectFailed(mqttOnFailed);
  mqtt.onCallback(mqttOnMessage);
  char buf[20];
  sprintf(buf, "EP_%08X",ESP.getChipId());
  mqtt_client = buf;
  Serial.println(mqtt_client);
  mqtt.begin(mqtt_client, mqtt_host, mqtt_port, mqtt_user, mqtt_pass);
  Serial.println("Ready");
  rconData(onRcData);
  rcbegin(RXPIN, TXPIN);
  //rcbegin(D7, D5);
}

unsigned long alive = 0;
unsigned long alivecounter = millis() + 60000;

void loop() {
  ArduinoOTA.handle();
  if (alivecounter < millis()) {
    alivecounter = millis() + 60000;
    Serial.printf("Alive: %4d\n", alive++);
  }
  mqtt.loop();  //  check for mqtt connection and messages.
  rcloop();
}

