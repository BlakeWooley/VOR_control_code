/*
File: control_code.cpp
Author: Blake Wooley
Date: 01-31-2023
*/

/* DEVNOTES
-need to use color palette for sprites or they won't work
-don't try to fillScreen the tft, it flickers terribly
-maybe writing that garbage to VSPI with the CS weirdness is a bad idea, probably caused the corruptions
*/

#include <TFT_eSPI.h>
#include <SPI.h>

//#define VSPI_MISO 19
//#define VSPI_MOSI 23
//#define VSPI_SCLK 18
//#define VSPI_SS 5

TFT_eSPI tft = TFT_eSPI();
TFT_eSprite background = TFT_eSprite(&tft); // Sprite object background
TFT_eSprite dial = TFT_eSprite(&tft); // Sprite object dial
TFT_eSprite needle = TFT_eSprite(&tft); // Sprite object needle
TFT_eSprite startup_disc = TFT_eSprite(&tft); // Sprite object startup_disc
TFT_eSprite startup_text = TFT_eSprite(&tft); // Sprite object startup_text
TFT_eSprite sradial_text = TFT_eSprite(&tft); // Sprite object sradial
TFT_eSprite tradial_text = TFT_eSprite(&tft); // Sprite object tradial
TFT_eSprite indicator = TFT_eSprite(&tft); // Sprite object sradial

// Palette colour table
uint16_t palette[16];
int16_t DEFLECTION = 0; //has a range of -90 to 90 degrees
int16_t DEFLECTION_POS = 40; //has a range of 0 to 80 pixels, where the needle should be
uint16_t CURRENT_DEFLECTION_POS = 40; //where the needle is now
int16_t SET_RADIAL = 0; //must be signed for comparisons
int16_t TRUE_RADIAL = 0; //must be signed for comparisons
int16_t RADIAL_COMPARE = 0; //compares TRUE_RADIAL and SET_RADIAL
uint16_t tempval = 0;
uint16_t tempval2 = 0;
uint16_t indicator_lock = 0;
uint16_t R_ENCODER_LD = 21;
//const int spiClk = 1000000; // 1 MHz
const int spiClk = 50000; // 50 kHz spi clock speed
SPIClass * vspi = NULL;
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
//==========================================================================================
void get_user_input(){
  //now read the rotation of the encoder from SPI
  digitalWrite(R_ENCODER_LD, LOW); //set GPIO21 low
  digitalWrite(R_ENCODER_LD, HIGH); //set GPIO21 high again, completing the parallel shift into the HC165
  static uint8_t rotary_data; //holds the serial data from the HC165
  static uint8_t rotary_data_lock; //captures the first rising edge from the rotary encoder

  vspi->beginTransaction(SPISettings(spiClk, MSBFIRST, SPI_MODE0));
  rotary_data = vspi->transfer(0xFF); //0xFF is just a dummy
  vspi->endTransaction();
  rotary_data &= 0x03; //keep the last two bits, the rest is garbage

  if (rotary_data_lock == 0x00 || rotary_data == 0x00) rotary_data_lock = rotary_data; //resets on 0x00, otherwise gets the first rising edge
  static const uint16_t rotary_increment = 5;
  //increment/decrement the count based on the direction of rotation of the encoder
  if(rotary_data == 0x02 && rotary_data_lock == 0x01){
    SET_RADIAL -= rotary_increment; //decrement the set radial
    if(SET_RADIAL < 0) SET_RADIAL += 360; //range of 0-359, allow wrapping around
	rotary_data_lock = 0x00; //reset the lock to prevent more than one decrement when hung on 0x02
  }
  else if(rotary_data == 0x01 && rotary_data_lock == 0x02){
    SET_RADIAL += rotary_increment; //increment the set radial
    if(SET_RADIAL > 359) SET_RADIAL = 0; //range of 0-359, allow wrapping around
	rotary_data_lock = 0x00; //reset the lock to prevent more than on increment when hung on 0x01
  }
}
//==========================================================================================
//this function gets every sprite set up
void sprite_setup(){
  //get the tft object set up
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);

  //create a sprite for the background
  background.setColorDepth(4);
  background.createSprite(320, 240); //always want to add 1 to all dimensions
  background.createPalette(palette);
  background.fillSprite(0); // Fill sprite with palette colour 9 (blue in this example)
  //draw white outer circle with no fill, radius=100, center=(160, 120)
  background.fillCircle(160, 120, 100, 13);
  //draw white inner circle with no fill, radius=80, center=(160, 120)
  background.fillCircle(160, 120, 80, 0);
  //draw white center circle for needle
  background.drawCircle(160, 120, 5, 15);
  //draw the white deflection dashes, each indicating 2 degrees of deflection
  background.drawFastVLine(120, 110, 20, 15);
  background.drawFastVLine(130, 110, 20, 15);
  background.drawFastVLine(140, 110, 20, 15);
  background.drawFastVLine(150, 110, 20, 15);
  background.drawFastVLine(170, 110, 20, 15);
  background.drawFastVLine(180, 110, 20, 15);
  background.drawFastVLine(190, 110, 20, 15);
  background.drawFastVLine(200, 110, 20, 15);

  //create a sprite for the needle
  needle.setColorDepth(4);
  needle.createSprite(82, 120); //gonna need to add two pixels to the width
  needle.createPalette(palette);
  needle.fillSprite(0); // Note: Sprite is filled with palette[0] colour when created
  //needle.drawFastVLine(0,0,120,14); //scroll doesn't like single pixel lines, i guess?
  needle.fillRect(0,0,3,120,14);
  needle.setScrollRect(0,0,3,120,0);

  //create a sprite for the scrolling numbers
  startup_disc.setColorDepth(4);
  startup_disc.createSprite(241, 201); //always want to add 1 to all dimensions
  startup_disc.createPalette(palette);
  startup_disc.fillSprite(0); // Fill sprite with palette colour 9 (blue in this example)

  //create a sprite for the scrolling numbers
  startup_text.setColorDepth(4);
  startup_text.createSprite(101, 64);
  startup_text.createPalette(palette);
  startup_text.fillSprite(0); // Fill sprite with black
  startup_text.setTextColor(4); // maroon text, no background
  startup_text.drawString("VOR",0,0,4); // Draw string using font 4
  startup_text.drawString("Reader",0,30,4);

  //create the set radial text
  sradial_text.setColorDepth(4);
  sradial_text.createSprite(50, 50); //always want to add 1 to all dimensions
  sradial_text.createPalette(palette);
  sradial_text.fillSprite(0); // Fill sprite with palette colour 9 (blue in this example)
  sradial_text.setTextColor(11); // cyan text, no background
  sradial_text.drawString("SET",0,0,4); // Draw string using font 4
  sradial_text.drawString(String(SET_RADIAL),0,25,4); // Draw string using font 4

  //create the true radial text
  tradial_text.setColorDepth(4);
  tradial_text.createSprite(50, 50); //always want to add 1 to all dimensions
  tradial_text.createPalette(palette);
  tradial_text.fillSprite(0); // Fill sprite with palette colour 9 (blue in this example)
  tradial_text.setTextColor(11); // cyan text, no background
  tradial_text.drawString("ON",0,0,4); // Draw string using font 4
  tradial_text.drawString(String(TRUE_RADIAL),0,25,4); // Draw string using font 4
  
  //create the to/from indicator
  indicator.setColorDepth(4);
  indicator.createSprite(25, 21);
  indicator.createPalette(palette);
  indicator.fillSprite(0); // Fill sprite with black
  indicator.fillTriangle(0,20,24,20,12,0,12);
}
//==========================================================================================
void indicator_deflection_update(){
  //compare the two radial values
  RADIAL_COMPARE = SET_RADIAL - TRUE_RADIAL;
  //set bounds from 180 to -180 for the comparison
  if(RADIAL_COMPARE < -180) RADIAL_COMPARE += 360;
  else if(RADIAL_COMPARE > 180) RADIAL_COMPARE -= 360;
  //create the FROM arrow
  if(RADIAL_COMPARE > 90 || RADIAL_COMPARE < -90){
    if(indicator_lock == 1){
      indicator.fillSprite(0); //erase the indicator
      indicator.fillTriangle(0,0,24,0,12,20,12); //draw down arrow
      indicator.pushSprite(180,60); //push the indicator to the display
      indicator_lock = 0;
    }
    if(RADIAL_COMPARE > 90) DEFLECTION = -1 * RADIAL_COMPARE + 180;
    else if(RADIAL_COMPARE < -90) DEFLECTION = -1* RADIAL_COMPARE - 180;
  }
  //create the TO arrow
  else if(RADIAL_COMPARE < 90 && RADIAL_COMPARE > -90){
    if(indicator_lock == 0){
      indicator.fillSprite(0); //erase the indicator
      indicator.fillTriangle(0,20,24,20,12,0,12); //draw up arrow
      indicator.pushSprite(180,60); //push the indicator to the display
      indicator_lock = 1;
    }
    DEFLECTION = RADIAL_COMPARE;
  }
  //calculate the pixel location for the deflection
  DEFLECTION_POS = (DEFLECTION * 5) + 40; //5 pixels per degree, centered on pixel 40
  //place bounds on the maximum and minimum deflection position
  if(DEFLECTION_POS > 80) DEFLECTION_POS = 80;
  else if(DEFLECTION_POS < 0) DEFLECTION_POS = 0;
}
//==========================================================================================
void testing(){
  tempval2++;
  if(tempval2 == 10){
    if(tempval<180) TRUE_RADIAL++;
    else if(tempval>=200 && tempval<380) SET_RADIAL++;
    else if(tempval>=400 && tempval<580) TRUE_RADIAL++;
    else if(tempval>=580 && tempval<760) SET_RADIAL--;
    
    if(tempval<760) tempval++;
    else if(tempval >= 760) tempval = 0;
    
    tempval2 = 0;
  }
  if(TRUE_RADIAL >= 360) TRUE_RADIAL = 0;
  else if(SET_RADIAL >= 360) SET_RADIAL = 0;
}
//==========================================================================================
void setup() {
  //set GPIO21 as an output for rotary encoder
  pinMode(R_ENCODER_LD, OUTPUT);

  //get vspi ready
  vspi = new SPIClass(VSPI);
  vspi->begin();

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

  //Serial.begin(250000);
  sprite_setup(); 
  startup_screen();
}
//==========================================================================================
void loop() {
  //for testing
  testing();

  //get_user_input(); //fetches any new information supplied by the user

  indicator_deflection_update();
  
  //make the needle move to the left or right
  if(DEFLECTION_POS > CURRENT_DEFLECTION_POS){
    //needle.scroll(1,0);
    tft.fillRect(119 + CURRENT_DEFLECTION_POS,60,3,120,TFT_BLACK); //erase the needle with a black rectangle
    indicator.pushSprite(180,60); //push the indicator to the display whenever the needle changes position
    CURRENT_DEFLECTION_POS++;
  }
  else if(DEFLECTION_POS < CURRENT_DEFLECTION_POS){
    //needle.scroll(-1,0);
    tft.fillRect(119 + CURRENT_DEFLECTION_POS,60,3,120,TFT_BLACK); //erase the needle with a black rectangle
    indicator.pushSprite(180,60); //push the indicator to the display whenever the needle changes position
    CURRENT_DEFLECTION_POS--;
  }

  //redraw the needle at the correct position
  tft.fillRect(119 + CURRENT_DEFLECTION_POS,60,3,120,TFT_YELLOW);

  //update some sprites
  background.pushSprite(0,0,0);
  sradial_text.fillRect(0,25,50,25,0); //erase the old set radial text
  sradial_text.drawString(String(SET_RADIAL),0,25,4); // Draw string using font 4
  sradial_text.pushSprite(270,0);
  tradial_text.fillRect(0,25,50,25,0); //erase the old set radial text
  tradial_text.drawString(String(TRUE_RADIAL),0,25,4); // Draw string using font 4
  tradial_text.pushSprite(0,0);
  
  //delay(1);
}
//==========================================================================================