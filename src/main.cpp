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
  mxconfig.i2sspeed = HUB75_I2S_CFG::HZ_10M;


  //mxconfig.scan = HUB75_I2S_CFG::SCAN_32;

  //printf("Hello World");
  //Scan config:: come back later and figure this out
  // mxconfig.scan = HUB75_I2S_CFG::SCAN_16;  // <-- Your panel is 1/16 scan
  mxconfig.driver = HUB75_I2S_CFG::FM6124;
  //Display Setup
  dma_display = new MatrixPanel_I2S_DMA(mxconfig);

  dma_display->setBrightness8(25); 
  uint16_t myBLACK = dma_display->color565(0, 0, 0);
uint16_t myWHITE = dma_display->color565(255, 255, 255);
uint16_t myRED = dma_display->color565(255, 0, 0);
uint16_t myGREEN = dma_display->color565(0, 255, 0);
uint16_t myBLUE = dma_display->color565(0, 0, 255);
  dma_display->begin();
  //0-255
    //RGB values for certain color we want

  dma_display->clearScreen();

  dma_display->fillScreen(myWHITE);
  delay(1000);
  dma_display->fillScreen(myRED);
  delay(1000);
  dma_display->fillScreen(myGREEN);
  delay(1000);
  dma_display->fillScreen(myBLUE);
  delay(1000);

  //dma_display->drawBitmap();

  dma_display->clearScreen();
  delay(1000);
}

void loop() {
  


  
}

// put function definitions here:
int myFunction(int x, int y) {
  return x + y;
}