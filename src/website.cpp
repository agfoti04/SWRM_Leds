#include <WiFi.h>
#include <WebServer.h>


const char* ssid = "Yggdrasil";
const char* password = "12345678";
//Access the controller at http://192.168.1.143

WebServer server(80);

//The Left Pins will control foward motion
//The right pins will control backward motion



void moveForward() {
  Serial.println("Move Forward command received!");
  // TODO: Motor control here
  
}

void moveLeft(){
  

}
void moveRight(){
 

}
void moveBack(){
  
}
void servoGrab(){

}




void handleRoot() {
  server.send(200, "text/html", R"rawliteral(
    <html>
      <body style='text-align:center; font-family:sans-serif;'>
        <h2>ESP32 Robot Control</h2>
        <button onclick="fetch('/forward')">Forward</button>
        <button onclick="fetch('/left')">Left</button>
        <button onclick="fetch('/right')">Right</button>
        <button onclick="fetch('/back')">Back</button>
        <button onclick="fetch('/stop')">Stop</button>
        <button onclick="fetch('/grab')">Grab</button>
      </body>
    </html>
  )rawliteral");
}

void handleForward() {
  moveForward();
  server.send(200, "text/plain", "Forward");
}

void handleLeft(){
  moveLeft();
  server.send(200, "text/plain", "Left");
}

void handleRight(){
  moveRight();
  server.send(200, "text/plain", "Right");
}

void handleBack(){
  moveBack();
  server.send(200, "text/plain", "Back");
}
void handleGrab(){
  servoGrab();
  server.send(200, "text/plain", "Grab");
}

void setup() {

  Serial.begin(115200);
  WiFi.softAP(ssid, password);
  Serial.println("ESP32 Wi-Fi AP Started");

  server.on("/", handleRoot);
  server.on("/forward", handleForward);
  server.on("/left", handleLeft);
  server.on("/right", handleRight);
  server.on("/back", handleBack);
  server.on("/grab", handleGrab);
  server.begin();
  Serial.println("Web server started");

  Serial.print("AP IP address: ");
  Serial.println(WiFi.softAPIP());

  
}

void loop() {
  server.handleClient();
}