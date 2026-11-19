#include <WiFi.h>

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);   // initializes the WiFi hardware so the MAC reads correctly
  delay(500);
}

void loop() {
  Serial.print("WiFi MAC address: ");
  Serial.println(WiFi.macAddress());
  delay(2000);
}