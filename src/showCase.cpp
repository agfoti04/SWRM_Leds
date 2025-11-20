#include "common.h"
#include "showCase.h"
#include <cstdlib>


uint16_t myBLACK = dma_display->color565(0, 0, 0);
uint16_t myWHITE = dma_display->color565(255, 255, 255);
uint16_t myRED = dma_display->color565(255, 0, 0);
uint16_t myGREEN = dma_display->color565(0, 255, 0);
uint16_t myBLUE = dma_display->color565(0, 0, 255);
uint16_t MAROON = dma_display->color565(128, 0 ,0);
//Draws A&M's Logo
void draw_AM(){
dma_display->fillScreen(0); // clear screen

  uint16_t maroon = dma_display->color565(128, 0, 0); // deep maroon color
  uint16_t white  = dma_display->color565(255, 255, 255);

  // Draw the "A"
  dma_display->fillRect(8, 20, 8, 30, maroon);   // left leg
  dma_display->fillRect(24, 20, 8, 30, maroon);  // right leg
  dma_display->fillRect(8, 20, 24, 6, maroon);   // top bar
  dma_display->fillRect(8, 35, 24, 5, maroon);   // mid bar

  // Draw the "T"
  dma_display->fillRect(36, 20, 20, 6, maroon);  // top bar
  dma_display->fillRect(44, 20, 6, 30, maroon);  // vertical stem

  // Optional: Add white border or accent lines
  dma_display->drawRect(6, 18, 52, 34, white);

  // Optional: Add “©TAMU” text at bottom
  write_word(10,56,white,1,"TAMU");

  delay(10000);
}

void write_word(int x, int y, uint16_t color, int size, char* word){
    dma_display->setCursor(x,y);
    dma_display->setTextColor(color);
    dma_display->setTextSize(size);
    dma_display->print(word);
}

void draw_spiral(){
  
  dma_display->drawRect(32,32,3,3,MAROON);
}

//Works
void animateSpinningSquare(bool clockwise, int off) {
    static float angle = 0;
    static float x_offset =0;

    dma_display->fillScreen(0);

    int cx = 32+x_offset + off;
    int cy = 32;
    int size = 20;

    for(int i=0; i<4; i++) {
        float a1 = angle + i * (PI/2);
        float a2 = angle + (i+1) * (PI/2);

        int x1 = cx + size * cos(a1);
        int y1 = cy + size * sin(a1);
        int x2 = cx + size * cos(a2);
        int y2 = cy + size * sin(a2);

        dma_display->drawLine(x1, y1, x2, y2, dma_display->color565(255,255,255));
    }


    if(clockwise){
      angle += 0.15;
      x_offset++;
    }
    else{
      angle-= 0.15;
      x_offset--;
    }
    
    delay(50);
}

//Works. Add color parameter
void draw_sawtoothWave(int x_s, int y_s, int period){

  int c=0;
  for(int i = x_s; i < 63; i+=period){
    if(c %2==0){
      dma_display->drawLine(x_s, y_s, x_s+period, y_s+10, MAROON);
      x_s+=period;
      y_s+=10;
    }
    else{
      dma_display->drawLine(x_s, y_s, x_s+period, y_s-10, MAROON);
      x_s+=period;
      y_s-=10;
    }
    c++;
    delay(50);
  }
}



void drawRipples(int startX, int startY, int maxRipples) {
    static int frame = 0;

    dma_display->fillScreen(0);

    // Each frame corresponds to one pixel of radius growth
    for (int i = 0; i < maxRipples; i++) {
        int radius = frame - i * 8;  // spacing between rings

        if (radius > 0) {
            dma_display->drawCircle(startX, startY, radius, dma_display->color565((std::rand()%255)+1, (std::rand()%255)+1,(std::rand()%255)+1));
            dma_display->drawCircle(startX, startY, radius+1, dma_display->color565((std::rand()%255)+1, (std::rand()%255)+1,(std::rand()%255)+1));
        }
    }

    frame++;

    // once ripples move off-screen, reset
    if (frame > 100) frame = 0;

    delay(100);
}

void randomRipples(){
  int random_x = (std::rand() % 63) +1;
  int random_y = (std::rand() % 63) +1;

  for(int i =0; i < 10; i++){
  drawRipples(random_x,random_y, 10);
}

  
}

// Each raindrop ripple emitter
struct Ripple {
    int x, y;        // center
    int radius;      // current radius
    bool active;     // whether ripple is alive
};

const int MAX_RIPPLES = 12;   // how many raindrops at once
Ripple ripples[MAX_RIPPLES];

void drawRainRipples(uint16_t color, int spawnChance, int maxRadius, int speed) {

    dma_display->fillScreen(dma_display->color565(0,0,10));

    // 1. Possibly spawn a new ripple
    if (random(0, spawnChance) == 0) {  
        for (int i = 0; i < MAX_RIPPLES; i++) {
            if (!ripples[i].active) {
                ripples[i].active = true;
                ripples[i].radius = 1;
                ripples[i].x = random(0, 128);
                ripples[i].y = random(0, 64);
                break;
            }
        }
    }

    // 2. Draw and update all active ripples
    for (int i = 0; i < MAX_RIPPLES; i++) {
        if (ripples[i].active) {

            // Fade color as radius grows
            uint8_t fade = 255 - (ripples[i].radius * 4);
            if (fade < 0) fade = 0;

            uint16_t fadeColor = dma_display->color565(0,0,255);

            dma_display->drawCircle(
                ripples[i].x,
                ripples[i].y,
                ripples[i].radius,
                fadeColor
            );

            ripples[i].radius++;

            // end ripple once it expands too far
            if (ripples[i].radius > maxRadius) {
                ripples[i].active = false;
            }
        }
    }

    delay(speed);
}

uint16_t color565(uint32_t rgb) {
  return (((rgb>>16) & 0xF8) << 8) | 
    (((rgb>>8) & 0xFC) << 3) | 
    ((rgb & 0xFF) >> 3);
};

void drawBitMap(int startx, int starty, int width, int height, uint64_t *bitmap){
   int counter = 0;
  for (int yy = 0; yy < height; yy++) {
    for (int xx = 0; xx < width; xx++) {
      dma_display->drawPixel(startx+xx, starty+yy, color565(bitmap[counter]));
      counter++;
    }
  }
}

void drawFlippedBitMap(int startx, int starty, int width, int height, uint64_t *bitmap){
  int counter = 0;
  for (int yy = 0; yy < height; yy++) {
    for (int xx = 0; xx < width; xx++) {
      dma_display->drawPixel(startx+ (width-1-xx), starty+yy, color565(bitmap[counter]));
      counter++;
    }
  }
}

void drawBitAnimation(int startx, int starty, int width, int height, uint64_t *bitmap){

}