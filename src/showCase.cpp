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