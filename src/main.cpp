/*
 * ESP32 LED Frame Fetcher - URL Test Version
 * Tests HTTP connection to n8n webhook API
 * Displays frame on 64x64 LED matrix
 */


#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>

// WiFi Credentials - UPDATE THESE!
#define WIFI_SSID "MyOptimum c2a9eb"
#define WIFI_PASSWORD "70-pink-7937"

// n8n API Configuration - UPDATE THIS!
#define N8N_WEBHOOK_URL "https://miguelarocha.app.n8n.cloud/webhook/frame/latest"

// LED Matrix Configuration
#define PANEL_RES_X 64
#define PANEL_RES_Y 64
#define PANEL_CHAIN 1

#define R1_PIN 25
#define G1_PIN 26
#define B1_PIN 27
#define R2_PIN 14
#define G2_PIN 12
#define B2_PIN 13

#define E_PIN 17
#define A_PIN 23
#define B_PIN 18
#define C_PIN 5
#define D_PIN 19
#define CLK_PIN 16
#define LAT_PIN 4
#define OE_PIN 15

// Frame buffer configuration
#define FRAME_SIZE (PANEL_RES_X * PANEL_RES_Y * 3)

// Global Variables
MatrixPanel_I2S_DMA *dma_display = nullptr;
uint8_t frame[FRAME_SIZE];  // Frame buffer for pixel data

void displayWaitScreen() {
    // Simple blue screen to indicate waiting
    dma_display->fillScreen(dma_display->color565(0, 0, 255));
}

void displayErrorScreen() {
    // Simple red screen to indicate error
    dma_display->fillScreen(dma_display->color565(255, 0, 0));
}

void displayConnectingScreen() {
    // Simple yellow screen to indicate connecting
    dma_display->fillScreen(dma_display->color565(255, 255, 0));
}

void setupWiFi() {
    dma_display->fillScreen(dma_display->color565(0, 0, 0));
    dma_display->setCursor(0, 0);
    dma_display->setTextColor(dma_display->color565(255, 255, 255));
    dma_display->print("Connecting to");
    dma_display->setCursor(0, 16);
    dma_display->print("WiFi:");
    dma_display->setCursor(0, 32);
    dma_display->print(WIFI_SSID);

    displayConnectingScreen();


    
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
        delay(500);
        dma_display->setCursor(attempts * 6, 48);
        dma_display->print(".");
        attempts++;
    }

    if (WiFi.status() == WL_CONNECTED) {
        dma_display->fillScreen(dma_display->color565(0, 0, 0));
        dma_display->setCursor(0, 0);
        dma_display->setTextColor(dma_display->color565(0, 255, 0));
        dma_display->print("WiFi");
        dma_display->setCursor(0, 16);
        dma_display->print("connected!");
        dma_display->setCursor(0, 32);
        dma_display->print("IP:");
        dma_display->setCursor(0, 48);
        dma_display->print(WiFi.localIP().toString());

        // Display success - green screen
        dma_display->fillScreen(dma_display->color565(0, 255, 0));
        delay(3000);

    } else {
        dma_display->fillScreen(dma_display->color565(0, 0, 0));
        dma_display->setCursor(0, 0);
        dma_display->setTextColor(dma_display->color565(255, 0, 0));
        dma_display->print("WiFi connection");
        dma_display->setCursor(0, 16);
        dma_display->print("failed!");
        displayErrorScreen();
        delay(5000);
    }
}

void setupLEDMatrix() {
    HUB75_I2S_CFG::i2s_pins _pins = {
        R1_PIN, G1_PIN, B1_PIN,
        R2_PIN, G2_PIN, B2_PIN,
        A_PIN, B_PIN, C_PIN, D_PIN, E_PIN,
        LAT_PIN, OE_PIN, CLK_PIN
    };

    HUB75_I2S_CFG mxconfig(PANEL_RES_X, PANEL_RES_Y, PANEL_CHAIN, _pins);
    mxconfig.i2sspeed = HUB75_I2S_CFG::HZ_10M;

    dma_display = new MatrixPanel_I2S_DMA(mxconfig);
    dma_display->begin();
    dma_display->fillScreen(dma_display->color565(0, 0, 0));

    dma_display->setCursor(0, 0);
    dma_display->setTextColor(dma_display->color565(255, 255, 255));
    dma_display->print("LED matrix");
    dma_display->setCursor(0, 16);
    dma_display->print("initialized!");
}

// Draw frame from buffer to display
void drawFrame(uint8_t* frameData, int width, int height) {
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int i = (y * width + x) * 3;
            uint8_t r = frameData[i];
            uint8_t g = frameData[i + 1];
            uint8_t b = frameData[i + 2];
            uint16_t color = dma_display->color565(r, g, b);
            dma_display->drawPixel(x, y, color);
        }
    }
}

// Fetch frame from webhook
bool showError(const char* msg) {
    dma_display->fillScreen(dma_display->color565(0, 0, 0));
    dma_display->setTextSize(1);
    dma_display->setTextColor(dma_display->color565(255, 0, 0));

    dma_display->setCursor(0, 10);
    dma_display->print("ERROR:");

    dma_display->setCursor(0, 25);
    dma_display->print(msg);

    Serial.println(msg);

    delay(3000);  // long readable delay
    return false;
}

bool fetchFrame() {

    WiFiClientSecure client;
    client.setInsecure();
    client.setTimeout(10000);

    HTTPClient http;
    http.setReuse(false);
    http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);

    http.begin(client, N8N_WEBHOOK_URL);

    int code = http.GET();
    Serial.printf("HTTP code: %d\n", code);

    if (code != 200) {
        http.end();
        return showError("HTTP FAIL");
    }

    String payload = http.getString();
    http.end();

    Serial.print("Payload len: ");
    Serial.println(payload.length());

    if (payload.length() < 50) {
        return showError("SHORT RESP");
    }

    // Parse CSV string
    int i = 0;
    int last = 0;

    while (i < FRAME_SIZE) {

        int comma = payload.indexOf(',', last);

        String val;

        if (comma == -1) {
            val = payload.substring(last);
        } else {
            val = payload.substring(last, comma);
        }

        int pixel = val.toInt();

        if (pixel < 0 || pixel > 255) {
            return showError("BAD PIXEL");
        }

        frame[i++] = (uint8_t)pixel;

        if (comma == -1) break;
        last = comma + 1;
    }

    if (i != FRAME_SIZE) {
        Serial.printf("Frame size: %d\n", i);
        return showError("FRAME SIZE");
    }

    // Render
    int idx = 0;

    for (int y = 0; y < 64; y++) {
        for (int x = 0; x < 64; x++) {

            uint8_t r = frame[idx++];
            uint8_t g = frame[idx++];
            uint8_t b = frame[idx++];

            dma_display->drawPixel(
                x, y,
                dma_display->color565(r, g, b)
            );
        }
    }
    delay(5000); // Show frame for 5 seconds
    return true;
}


bool testHTTPConnection() {
    if (WiFi.status() != WL_CONNECTED) {
        dma_display->fillScreen(dma_display->color565(0, 0, 0));
        dma_display->setCursor(0, 0);
        dma_display->setTextColor(dma_display->color565(255, 0, 0));
        dma_display->print("WiFi not");
        dma_display->setCursor(0, 16);
        dma_display->print("connected!");
        return false;
    }

    static WiFiClientSecure secure_client;
    HTTPClient http;
    http.setConnectTimeout(5000);
    http.setTimeout(10000);

    dma_display->fillScreen(dma_display->color565(0, 0, 0));
    dma_display->setCursor(0, 0);
    dma_display->setTextColor(dma_display->color565(255, 255, 255));
    dma_display->print("Testing URL:");
    dma_display->setCursor(0, 16);
    dma_display->print("miguelarocha.app");
    dma_display->setCursor(0, 32);
    dma_display->print(".n8n.cloud");

    http.begin(secure_client, N8N_WEBHOOK_URL);
    //http.addHeader("Content-Type", "application/json");

    int httpCode = http.GET();
    Serial.printf("HTTP code: %d\n", httpCode);

    dma_display->setCursor(0, 48);
    dma_display->print("Code: ");
    dma_display->print(httpCode);

    if (httpCode > 0) {
        String payload = http.getString();
        dma_display->fillScreen(dma_display->color565(0, 0, 0));
        dma_display->setCursor(0, 0);
        dma_display->setTextColor(dma_display->color565(0, 255, 0));
        dma_display->print("Response OK!");
        dma_display->setCursor(0, 16);
        dma_display->print("Code: ");
        dma_display->print(httpCode);
        http.end();
        return true;
    } else {
        dma_display->fillScreen(dma_display->color565(0, 0, 0));
        dma_display->setCursor(0, 0);
        dma_display->setTextColor(dma_display->color565(255, 0, 0));
        dma_display->print("HTTP Error!");
        dma_display->setCursor(0, 16);
        dma_display->print("Code: ");
        dma_display->print(httpCode);
        http.end();
        return false;
    }
}

void setup() {
    Serial.begin(115200);
    delay(1000);

    // Initialize LED matrix FIRST
    setupLEDMatrix();
    
    // Set text size for better visibility on 64x64 display
    dma_display->setTextSize(2);
    
    dma_display->fillScreen(dma_display->color565(0, 0, 0));
    dma_display->setCursor(0, 0);
    dma_display->setTextColor(dma_display->color565(255, 255, 255));
    dma_display->print("ESP32 LED");
    dma_display->setCursor(0, 16);
    dma_display->print("Frame Fetcher");
    dma_display->setCursor(0, 32);
    dma_display->print("URL Test");
    dma_display->setCursor(0, 48);
    dma_display->print("Starting...");
    delay(2000);

    displayWaitScreen();
    delay(1000);

    // Connect to WiFi
    setupWiFi();

    // Test HTTP connection
    dma_display->fillScreen(dma_display->color565(0, 0, 0));
    dma_display->setCursor(0, 0);
    dma_display->setTextColor(dma_display->color565(255, 255, 255));
    dma_display->print("");
    dma_display->setCursor(0, 16);
    dma_display->print("");
    delay(1000);
    /*
    if (testHTTPConnection()) {
        dma_display->fillScreen(dma_display->color565(0, 0, 0));
        dma_display->setCursor(0, 0);
        dma_display->setTextColor(dma_display->color565(0, 255, 0));
        dma_display->print("HTTP test");
        dma_display->setCursor(0, 16);
        dma_display->print("successful!");
        delay(3000);
        dma_display->fillScreen(dma_display->color565(0, 255, 0)); // Green for success
    } else {
        dma_display->fillScreen(dma_display->color565(0, 0, 0));
        dma_display->setCursor(0, 0);
        dma_display->setTextColor(dma_display->color565(255, 0, 0));
        dma_display->print("womp womp pal");
        dma_display->setCursor(0, 16);
        dma_display->print("failed");
        delay(5000);
        dma_display->fillScreen(dma_display->color565(255, 0, 0)); // Red for failure
    }
*/
    dma_display->fillScreen(dma_display->color565(0, 0, 0));
    dma_display->setCursor(0, 0);
    dma_display->setTextColor(dma_display->color565(255, 255, 255));
    dma_display->print("Setup");
    dma_display->setCursor(0, 16);
    dma_display->print("complete!");
}

void loop() {
    
    
    
    
    
    
    
    bool success = fetchFrame();

    if (!success) {
        // Show fallback color pattern to indicate device is alive
        static int state = 0;
        static unsigned long lastChange = 0;

        if (millis() - lastChange > 500) {
            if (state == 0)
                dma_display->fillScreen(dma_display->color565(255, 0, 0));   // Red
            else if (state == 1)
                dma_display->fillScreen(dma_display->color565(0, 255, 0));   // Green
            else
                dma_display->fillScreen(dma_display->color565(0, 0, 255));   // Blue

            state = (state + 1) % 3;
            lastChange = millis();
        }
    }

    delay(2000);  // Poll every 2 seconds
}
