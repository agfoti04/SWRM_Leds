#include <Arduino.h>
#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>

#include "common.h"
#include "showCase.h"

//Pin out
//these are right
#define R1_PIN 25
#define G1_PIN 26
#define B1_PIN 27
#define R2_PIN 14
#define G2_PIN 12
#define B2_PIN 13

#define A_PIN 23  //yes
#define B_PIN 19 //yes
#define C_PIN 5 //yes
#define D_PIN 18 //yes
#define E_PIN 17 //yes

//issue with these pins
#define LAT_PIN 4 //moved
#define OE_PIN 15//moved
#define CLK_PIN 16 //moved

#define PANEL_RES_X 64
#define PANEL_RES_Y 64
#define PANEL_CHAIN 1

MatrixPanel_I2S_DMA *dma_display = nullptr;

void setup() {

  Serial.begin(115200);
  delay(1000);
  
  //init pin connections between esp32 and the LED board
  HUB75_I2S_CFG::i2s_pins _pins = {R1_PIN, G1_PIN, B1_PIN, R2_PIN, G2_PIN, B2_PIN, A_PIN, B_PIN, C_PIN, D_PIN, E_PIN, LAT_PIN, OE_PIN, CLK_PIN};
  delay(10);

  //module config
  HUB75_I2S_CFG mxconfig(PANEL_RES_X, PANEL_RES_Y, PANEL_CHAIN, _pins);
  delay(10);

  //set I2S clock speed
  mxconfig.i2sspeed = HUB75_I2S_CFG::HZ_10M;

  mxconfig.mx_height= 64;
  mxconfig.mx_width = 64;
  mxconfig.driver = HUB75_I2S_CFG::FM6124;

  mxconfig.clkphase = false;
  //Display Setup
  dma_display = new MatrixPanel_I2S_DMA(mxconfig);
  

  

  dma_display->begin();
  //0-255
    //RGB values for certain color we want
  dma_display->setBrightness8(25); 
  dma_display->clearScreen();

  dma_display->fillScreen(MAROON);
  delay(1000);
  

  //dma_display->drawBitmap();

  dma_display->clearScreen();
  delay(1000);
}

void loop() {
  

  
  dma_display->clearScreen();

  // dma_display->fillScreen(0);
  // dma_display->drawLine(0, 0, 63, 63, dma_display->color565(0, 255, 255));
  // dma_display->drawCircle(32, 32, 15, dma_display->color565(255, 0, 255));
  // dma_display->drawRect(5, 5, 54, 54, dma_display->color565(255, 255, 0));


  delay(2000);
  animateSpinningSquare();

  delay(20000);
  
  
}

// put function definitions here:
int draw(int x, int y) {
  return x + y;
}