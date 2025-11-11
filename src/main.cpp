#include <Arduino.h>
#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>

//Pin out
#define R1_PIN 25
#define G1_PIN 26
#define B1_PIN 27
#define R2_PIN 14
#define G2_PIN 12
#define B2_PIN 13

#define A_PIN 23
#define B_PIN 19
#define C_PIN 5
#define D_PIN 17
#define E_PIN 18

#define LAT_PIN 4
#define OE_PIN 15
#define CLK_PIN 16

#define PANEL_RES_X 64
#define PANEL_RES_Y 64
#define PANEL_CHAIN 1

MatrixPanel_I2S_DMA *dma_display = nullptr;




// put function declarations here:
int myFunction(int, int);

void setup() {
  delay(1000);
  
  //init pin connections between esp32 and the LED board
  HUB75_I2S_CFG::i2s_pins _pins = {R1_PIN, G1_PIN, B1_PIN, R2_PIN, G2_PIN, B2_PIN, A_PIN, B_PIN, C_PIN, D_PIN, E_PIN, LAT_PIN, OE_PIN, CLK_PIN};
  delay(10);

  //module config
  HUB75_I2S_CFG mxconfig(PANEL_RES_X, PANEL_RES_Y, PANEL_CHAIN, _pins);
  delay(10);

  //set I2S clock speed
  mxconfig.i2sspeed = HUB75_I2S_CFG::HZ_20M;




  //printf("Hello World");
  //Scan config:: come back later and figure this out
  // mxconfig.scan = HUB75_I2S_CFG::SCAN_16;  // <-- Your panel is 1/16 scan

  //Display Setup
  dma_display = new MatrixPanel_I2S_DMA(mxconfig);
  dma_display->begin();
  dma_display->setBrightness8(25); //0-255
    //RGB values for certain color we want
uint16_t myBLACK = dma_display->color565(0, 0, 0);
uint16_t myWHITE = dma_display->color565(255, 255, 255);
uint16_t myRED = dma_display->color565(255, 0, 0);
uint16_t myGREEN = dma_display->color565(0, 255, 0);
uint16_t myBLUE = dma_display->color565(0, 0, 255);
  dma_display->clearScreen();

  dma_display->fillScreen(myWHITE);
  delay(1000);
  dma_display->fillScreen(myRED);
  delay(1000);
  dma_display->fillScreen(myGREEN);
  delay(1000);
  dma_display->fillScreen(myBLUE);
  delay(1000);

  dma_display->clearScreen();
  delay(1000);
}

void loop() {
  dma_display->setTextSize(1);

  dma_display->setTextWrap(false);

  dma_display->setCursor(10, 0);
  dma_display->setTextColor(dma_display->color565(255, 153, 0));
  dma_display->println("UTEH");

  dma_display->setCursor(36, 0);
  dma_display->setTextColor(dma_display->color565(255, 0, 255));
  dma_display->print("STR");

  dma_display->setCursor(11, 8);
  dma_display->setTextColor(dma_display->color565(0, 152, 158));
  dma_display->println("ARDUINO");

  dma_display->setCursor(16, 17);
  dma_display->setTextColor(dma_display->color565(255, 255, 255));
  dma_display->print("P5");

  dma_display->setCursor(30, 17);
  dma_display->setTextColor(dma_display->color565(255, 0, 0));
  dma_display->print("R");

  dma_display->setTextColor(dma_display->color565(0, 255, 0));
  dma_display->print("G");

  dma_display->setTextColor(dma_display->color565(0, 0, 255));
  dma_display->print("B");

  dma_display->setCursor(16, 25);
  dma_display->setTextColor(dma_display->color565(255, 0, 102));
  dma_display->print("ESP");

  dma_display->setCursor(36, 25);
  dma_display->setTextColor(dma_display->color565(241, 197, 7));
  dma_display->print("32");

  delay(1000);
}

// put function definitions here:
int myFunction(int x, int y) {
  return x + y;
}