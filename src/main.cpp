#include <Arduino.h>
#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>

#include "common.h"
#include "showCase.h"

// LED Panel pins
#define R1_PIN 25
#define G1_PIN 26
#define B1_PIN 27
#define R2_PIN 14
#define G2_PIN 12
#define B2_PIN 13
#define A_PIN 23
#define B_PIN 19
#define C_PIN 5
#define D_PIN 18
#define E_PIN 17
#define LAT_PIN 4
#define OE_PIN 15
#define CLK_PIN 16

#define PANEL_RES_X 64
#define PANEL_RES_Y 64
#define PANEL_CHAIN 1



// LED display object
MatrixPanel_I2S_DMA *dma_display = nullptr;

// Extern frame data from frame1.cpp
extern "C" const uint64_t frame1[];

// Test frame: simple red square
const uint64_t testFrame[4096] = {0xFF0000}; // All red, but actually need to fill all

void setup()
{
    Serial.begin(115200);
    delay(1000);

    // ----- LED Panel setup -----
    HUB75_I2S_CFG::i2s_pins _pins = {R1_PIN, G1_PIN, B1_PIN, R2_PIN, G2_PIN, B2_PIN, A_PIN, B_PIN, C_PIN, D_PIN, E_PIN, LAT_PIN, OE_PIN, CLK_PIN};
    HUB75_I2S_CFG mxconfig(PANEL_RES_X, PANEL_RES_Y, PANEL_CHAIN, _pins);
    mxconfig.i2sspeed = HUB75_I2S_CFG::HZ_10M;
    mxconfig.mx_height = 64;
    mxconfig.mx_width = 64;
    mxconfig.driver = HUB75_I2S_CFG::FM6124;
    mxconfig.clkphase = false;
    dma_display = new MatrixPanel_I2S_DMA(mxconfig);
    dma_display->begin();
    dma_display->setBrightness8(255);  // Max brightness
    dma_display->clearScreen();

    Serial.println("LED Panel Ready");
}

void loop()
{
    // Display a red screen
    dma_display->fillScreen(dma_display->color565(255, 0, 0));
    delay(1000);

    // Display a green screen
    dma_display->fillScreen(dma_display->color565(0, 255, 0));
    delay(1000);

    // Display a blue screen
    dma_display->fillScreen(dma_display->color565(0, 0, 255));
    delay(1000);

    // Clear screen
    dma_display->clearScreen();
    delay(1000);

    // Test: draw a white rectangle at 0,0
    dma_display->fillRect(0, 0, 20, 20, dma_display->color565(255, 255, 255));
    delay(2000);

    // Clear screen
    dma_display->clearScreen();
    delay(1000);

    // Display the frame from frame1.cpp
    drawBitMap(0, 0, 64, 64, frame1);
    delay(10000);  // Show frame for 10 seconds

    // Clear screen again
    dma_display->clearScreen();
    delay(1000);
}
