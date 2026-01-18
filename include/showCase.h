#ifndef showCase_H
#define showCase_H
#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>

void draw_AM();

void write_word(int x, int y, uint16_t color, int size, char* word);

//Works
void animateSpinningSquare(bool clockwise, int off);

//Works, add color parameter
void draw_sawtoothWave(int x_s, int y_s, int period);

//Works
void drawRipples(int startX, int startY, int maxRipples);

//WIP
void randomRipples();

//Single ripple rain drops
void drawRainRipples(uint16_t color, int spawnChance, int maxRadius, int speed);

//Works
void drawBitMap(int startx, int starty, int width, int height, uint64_t *bitmap);

void drawFlippedBitMap(int startx, int starty, int width, int height, uint64_t *bitmap);

void drawFireworks();

#endif