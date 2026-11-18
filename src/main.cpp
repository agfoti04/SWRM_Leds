#include <WiFi.h>
#include <PubSubClient.h>

const char* WIFI_SSID = "TAMU_IoT";
const char* WIFI_PASS = "";   // use "" for an open network
const char* MQTT_HOST = "10.251.20.11";     // laptop's IPv4 from ipconfig
const int   MQTT_PORT = 1883;
const char* ROBOT_ID  = "robot1";

#define LED_PIN 2   // onboard LED on most dev boards

WiFiClient wifiClient;
PubSubClient mqtt(wifiClient);
String cmdTopic, statusTopic;

void onMessage(char* topic, byte* payload, unsigned int len) {
  String msg;
  for (unsigned int i = 0; i < len; i++) msg += (char)payload[i];
  Serial.printf("[%s] %s\n", topic, msg.c_str());
  if (msg == "on")  digitalWrite(LED_PIN, HIGH);
  if (msg == "off") digitalWrite(LED_PIN, LOW);
  mqtt.publish(statusTopic.c_str(), ("ack:" + msg).c_str());
}

void connectWiFi() {
  Serial.print("Connecting to WiFi");
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }
  Serial.println();
  Serial.print("IP: "); Serial.println(WiFi.localIP());
}

void connectMQTT() {
  while (!mqtt.connected()) {
    Serial.print("Connecting to MQTT... ");
    if (mqtt.connect(ROBOT_ID)) {
      Serial.println("connected");
      mqtt.subscribe(cmdTopic.c_str());
      mqtt.publish(statusTopic.c_str(), "online");
    } else {
      Serial.printf("failed, rc=%d, retrying\n", mqtt.state());
      delay(2000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  cmdTopic    = String("robots/") + ROBOT_ID + "/cmd";
  statusTopic = String("robots/") + ROBOT_ID + "/status";
  Serial.print("MAC: "); Serial.println(WiFi.macAddress());
  connectWiFi();
  mqtt.setServer(MQTT_HOST, MQTT_PORT);
  mqtt.setCallback(onMessage);
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) connectWiFi();
  if (!mqtt.connected()) connectMQTT();
  mqtt.loop();
}