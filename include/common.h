#ifndef COMMON_H
#define COMMON_H
#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>


extern MatrixPanel_I2S_DMA *dma_display;

uint16_t myBLACK = dma_display->color565(0, 0, 0);
uint16_t myWHITE = dma_display->color565(255, 255, 255);
uint16_t myRED = dma_display->color565(255, 0, 0);
uint16_t myGREEN = dma_display->color565(0, 255, 0);
uint16_t myBLUE = dma_display->color565(0, 0, 255);
uint16_t MAROON = dma_display->color565(128, 0 ,0);

#endif