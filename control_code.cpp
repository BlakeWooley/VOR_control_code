/*
File: control_code.cpp
Author: Blake Wooley
Date: 01-31-2023
*/

/* DEVNOTES
-need to use color palette for sprites or they won't work
-don't try to fillScreen the tft, it flickers terribly
*/

#include <TFT_eSPI.h>
//#include <SPI.h>
//#include <string>

TFT_eSPI tft = TFT_eSPI();
TFT_eSprite background = TFT_eSprite(&tft); // Sprite object background
TFT_eSprite dial = TFT_eSprite(&tft); // Sprite object dial
TFT_eSprite needle = TFT_eSprite(&tft); // Sprite object needle
TFT_eSprite startup_disc = TFT_eSprite(&tft); // Sprite object startup_disc
TFT_eSprite startup_text = TFT_eSprite(&tft); // Sprite object startup_text
// Palette colour table
uint16_t palette[16];
uint16_t DEFLECTION;
uint16_t DEFLECTION_POS = 0;
uint16_t CURRENT_DEFLECTION_POS = 0;
uint16_t SET_RADIAL;
uint16_t TRUE_RADIAL;
uint16_t tempval = 0;
//==========================================================================================
void startup_screen(){
  //draw the disc in the center of the screen
  uint16_t time_delay = 15;
  for(uint16_t i=15; i<100; i++){
    startup_disc.drawCircle(140,100,i,15);
    startup_disc.pushSprite(20,20,0);
    if(time_delay > 1) time_delay--;
    delay(time_delay); //time_delay gets shorter each iteration
  }
  delay(500);
  //move the disc to the left
  for(uint16_t i=0; i<40; i++){
    //background.fillSprite(TFT_BLACK);
    //startup_disc.pushToSprite(&background,60-i,20,TFT_BLACK);
    //background.pushSprite(0,0,TFT_BLACK);
    //tft.fillScreen(TFT_BLACK);
    startup_disc.scroll(-1,0);
    startup_disc.pushSprite(20,20);
    delay(1);
  }
  //draw the startup text
  startup_text.pushSprite(230,100,0);
  delay(2000);
  //blank the screen
  tft.fillScreen(TFT_BLACK);
  //clean up RAM
  startup_disc.deleteSprite();
  startup_text.deleteSprite();
}

void screen_init(){
    //draw white outer circle with no fill, radius=100, center=(160, 120)
    background.drawCircle(160, 120, 100, 15);
    //draw white inner circle with no fill, radius=80, center=(160, 120)
    background.drawCircle(160, 120, 80, 15);
    //draw white center circle for needle
    background.drawCircle(160, 120, 5, 15);
    //draw yellow needle at center position with height=120
    //tft.drawFastVLine(160, 60, 120, TFT_YELLOW);
    //draw the white deflection dashes, each indicating 2 degrees of deflection
    background.drawFastVLine(120, 110, 20, 15);
    background.drawFastVLine(130, 110, 20, 15);
    background.drawFastVLine(140, 110, 20, 15);
    background.drawFastVLine(150, 110, 20, 15);
    background.drawFastVLine(170, 110, 20, 15);
    background.drawFastVLine(180, 110, 20, 15);
    background.drawFastVLine(190, 110, 20, 15);
    background.drawFastVLine(200, 110, 20, 15);
}

void setup() {
  //Serial.begin(250000);
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);

  // Populate the palette table, table must have 16 entries
  palette[0]  = TFT_BLACK;
  palette[1]  = TFT_ORANGE;
  palette[2]  = TFT_DARKGREEN;
  palette[3]  = TFT_DARKCYAN;
  palette[4]  = TFT_MAROON;
  palette[5]  = TFT_PURPLE;
  palette[6]  = TFT_OLIVE;
  palette[7]  = TFT_DARKGREY;
  palette[8]  = TFT_ORANGE;
  palette[9]  = TFT_BLUE;
  palette[10] = TFT_GREEN;
  palette[11] = TFT_CYAN;
  palette[12] = TFT_RED;
  palette[13] = TFT_NAVY;
  palette[14] = TFT_YELLOW;
  palette[15] = TFT_WHITE; 

  // Create a sprite for the background
  background.setColorDepth(4);
  background.createSprite(320, 240); //always want to add 1 to all dimensions
  background.createPalette(palette);
  background.fillSprite(0); // Fill sprite with palette colour 9 (blue in this example)

  // Create a sprite for the needle
  //ok so we could draw the sprite first and then redraw any markers that are within range of CURRENT_DEFLECTION_POS, would prefer markers on top of needle if it's 3 pixels thick
  needle.setColorDepth(4);
  needle.createSprite(82, 120); //gonna need to add two pixels to the width
  needle.createPalette(palette);
  needle.fillSprite(0); // Note: Sprite is filled with palette[0] colour when created
  //needle.drawFastVLine(0,0,120,14); //scroll doesn't like single pixel lines, i guess?
  needle.fillRect(0,0,3,120,14);
  needle.setScrollRect(0,0,3,120,0),
/*
  // Create a sprite for the needle
  //ok so we could draw the sprite first and then redraw any markers that are within range of CURRENT_DEFLECTION_POS, would prefer markers on top of needle if it's 3 pixels thick
  needle.setColorDepth(4);
  needle.createSprite(3, 121); //gonna need to add two pixels to the width
  needle.createPalette(palette);
  needle.fillSprite(0); // Note: Sprite is filled with black
  needle.drawFastVLine(1,0,120,14);
*/
  // Create a sprite for the scrolling numbers
  startup_disc.setColorDepth(4);
  startup_disc.createSprite(241, 201); //always want to add 1 to all dimensions
  startup_disc.createPalette(palette);
  startup_disc.fillSprite(0); // Fill sprite with palette colour 9 (blue in this example)

  // Create a sprite for the scrolling numbers
  startup_text.setColorDepth(4);
  startup_text.createSprite(101, 64);
  startup_text.createPalette(palette);
  startup_text.fillSprite(0); // Fill sprite with black
  startup_text.setTextColor(4); // maroon text, no background
  startup_text.drawString("VOR",0,0,4); // Draw string using font 4
  startup_text.drawString("Reader",0,30,4);

  startup_screen();
  screen_init();
}

//==========================================================================================
void loop() {
  //DEFLECTION = SET_RADIAL - TRUE_RADIAL;
  
  //for testing
  tempval++;
  if(tempval<80) DEFLECTION_POS++;
  else if(tempval<160) DEFLECTION_POS--;
  else tempval = 0;

  //place bounds on the maximum and minimum deflection position
  if(DEFLECTION_POS > 80) DEFLECTION_POS = 80;
  else if(DEFLECTION_POS < 0) DEFLECTION_POS = 0;
  tft.fillRect(119 + CURRENT_DEFLECTION_POS,60,3,120,TFT_BLACK);
  //make the needle move to the left or right
  if(DEFLECTION_POS > CURRENT_DEFLECTION_POS){
    //needle.scroll(1,0);
    CURRENT_DEFLECTION_POS++;
  }
  else if(DEFLECTION_POS < CURRENT_DEFLECTION_POS){
    //needle.scroll(-1,0);
    CURRENT_DEFLECTION_POS--;
  }
  tft.fillRect(119 + CURRENT_DEFLECTION_POS,60,3,120,TFT_YELLOW);

  //make the sprite show up
  //needle.pushSprite(119,60);
  //needle.pushSprite(119 + CURRENT_DEFLECTION_POS,60);

  //redraw any affected deflection indicators
  /*
  if(CURRENT_DEFLECTION_POS <= 1) background.drawFastVLine(120, 110, 20, 15);
  else if(CURRENT_DEFLECTION_POS >= 9 || CURRENT_DEFLECTION_POS <= 11) background.drawFastVLine(130, 110, 20, 15);
  else if(CURRENT_DEFLECTION_POS = 19 || CURRENT_DEFLECTION_POS <= 21) background.drawFastVLine(140, 110, 20, 15);
  else if(CURRENT_DEFLECTION_POS = 29) background.drawFastVLine(150, 110, 20, 15);
  else if(CURRENT_DEFLECTION_POS = 39)
  else if(CURRENT_DEFLECTION_POS = 49) background.drawFastVLine(170, 110, 20, 15);
  else if(CURRENT_DEFLECTION_POS = 59) background.drawFastVLine(180, 110, 20, 15);
  else if(CURRENT_DEFLECTION_POS = 69) background.drawFastVLine(190, 110, 20, 15);
  else if(CURRENT_DEFLECTION_POS >= 79) background.drawFastVLine(200, 110, 20, 15);
  */
  background.pushSprite(0,0,0);

  delay(200);
}

  /*
  // Draw point in needle sprite at far right edge (this will scroll left later)
  dial.drawCircle(100,100,80,15);
  needle.drawFastVLine(160,80,120,);
  //Push the sprites onto the TFT at specified coordinates
  //needle.pushToSprite(&startup_disc, 0, 0, TFT_BLACK);
  needle.pushSprite(0,0,0);
  dial.pushSprite(0,0,0);

  THIS WORKED
  needle.drawCircle(100,100,80,TFT_WHITE); 
  startup_disc.drawCircle(100,100,50,TFT_WHITE);
  needle.pushSprite(0,0,TFT_BLACK);
  startup_disc.pushSprite(0,0,TFT_BLACK);

  THIS WORKED
  needle.drawCircle(100,100,100,TFT_WHITE); 
  startup_disc.drawCircle(100,100,80,TFT_WHITE);
  startup_disc.pushSprite(0,0,TFT_BLACK);
  needle.pushSprite(0,0,TFT_BLACK);

  THIS WORKED
  needle.drawCircle(100,100,100,TFT_WHITE); 
  startup_disc.drawCircle(100,100,DEFLECTION_POS,TFT_WHITE);
  startup_disc.pushSprite(0,0,TFT_BLACK);
  needle.pushSprite(0,0,TFT_BLACK);

  THIS DIDN'T WORK
  needle.drawCircle(100,100,80,TFT_WHITE); 
  startup_disc.drawCircle(100,100,60,TFT_WHITE);
  startup_disc.pushToSprite(&needle,0,0,TFT_BLACK);
  needle.pushSprite(0,0,TFT_BLACK);
  */