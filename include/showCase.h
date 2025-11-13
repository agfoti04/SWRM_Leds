#ifndef showCase_H
#define showCase_H
#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>

void draw_AM();

void write_word(int x, int y, uint16_t color, int size, char* word);

#endif