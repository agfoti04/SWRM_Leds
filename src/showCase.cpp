#include "common.h"
#include "showCase.h"

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

void animateSpinningSquare() {
    static float angle = 0;

    dma_display->fillScreen(0);

    int cx = 32;
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

    angle += 0.15;
}

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
    
  }
}