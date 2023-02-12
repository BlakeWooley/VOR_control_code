#include library_for_LCD

TFT_eSPI tft; //make object representing the tft LCD
TFT_eSprite background; //Sprite object background
TFT_eSprite dial; // Sprite object dial
TFT_eSprite startup_disc; // Sprite object startup_disc
TFT_eSprite startup_text; // Sprite object startup_text
TFT_eSprite sradial_text; // Sprite object set radial
TFT_eSprite tradial_text; // Sprite object for the true radial
TFT_eSprite indicator; // Sprite object for the TO/FROM indicator

uint16_t DEFLECTION; //the true deflection from the set radial
uint16_t DEFLECTION_POS; //the intermediate value used to determine the deflection position on the LCD
uint16_t CURRENT_DEFLECTION_POS; //holds where the deflection position is at
int16_t SET_RADIAL; //the radial the user wishes to be on
uint16_t TRUE_RADIAL; //this value is updated via the signal processing code, it's the radial we're currently on
uint16_t R_ENCODER_LD; //the rotary encoder for setting the desired radial
uint16_t* DSIG_PROCESSING; //the spot in memory for the true radial output from the digital signal processing
uint16_t ENCODER_MODE; //holds an integer representing the mode the encoder is in
uint16_t TARGET_FREQUENCY; //the frequency on which we hope to receive our variable signals
SPIClass* vspi; //setup a SPI object
//==========================================================================================
void startup_screen(){
  startup_disc.drawCircle(); //draw the startup sprite
  startup_disc.pushSprite();
  delay(500); //500 millisecond delay
  startup_disc.scroll(); //move the disc to the left
  startup_disc.pushSprite();
  //draw the startup text
  startup_text.pushSprite();
  delay(2000); //2 second delay
  tft.fillScreen(); //blank the screen
  startup_disc.deleteSprite(); //clean up RAM
  startup_text.deleteSprite(); //clean up RAM
}

void background_setup(){
  //draw white outer circle
  background.fillCircle();
  //draw white inner circle
  background.fillCircle();
  //draw white center circle
  background.drawCircle();
  //draw the white deflection dashes, each indicating 2 degrees of deflection
  background.drawFastVLine();
}

void update_set_radial(){
  sradial_text.fillRect(); //erase the old set radial text
  sradial_text.drawString(); //draw the new set radial text
  sradial_text.pushSprite(); //push the updated set radial text to the tft object
}

void update_true_radial(){
  TRUE_RADIAL = *DSIG_PROCESSING; //fetch the current true radial from memory
  DEFLECTION = SET_RADIAL - TRUE_RADIAL; //find the deflection
  tradial_text.fillRect(); //erase the old true radial text
  tradial_text.drawString(); //draw the new true radial text
  sradial_text.pushSprite(); //push the updated true radial text to the tft object
}

void sprite_setup(){
  tft.init(); //initialize the display object
  tft.setRotation(); //set the orientation of the x and y coordinates
  tft.fillScreen(TFT_BLACK); //blank the screen
  background.setup(); // Create a sprite for the background
  startup_disc.setup(); // Create a sprite for the startup animation
  startup_text.setup(); // Create a sprite for the startup text
  sradial_text.setup(); // Create a sprite for the set radial text
  tradial_text.setup(); // Create a sprite for the true radial text
  indicator.setup(); // Create a sprite for the TO/FROM indicator
}

void set_target_frequency(){
  //manipulate whatever pin the mixer is connected to
}

void get_user_input(){
  if(pin == HIGH) ENCODER_MODE = 1; //figures out what mode the rotary encoder should be in
  else ENCODER_MODE = 0; //0 is frequency mode, 1 is radial mode
  static uint8_t rotary_data; //holds the serial data
  vspi->beginTransaction(SPISettings(spiClk, MSBFIRST, SPI_MODE0)); //begin SPI comms
  rotary_data = vspi->transfer(); //get the rotary encoder data
  vspi->endTransaction(); //end SPI comms
  if(ENCODER_MODE == 1) SET_RADIAL = ?;//change the set radial
  else TARGET_FREQUENCY = ?;//change the target frequency
}
//==========================================================================================
void setup(){ //this function is called first
  pinMode(R_ENCODER_LD, OUTPUT); //setup pin for rotary encoder
  vspi->begin(); //start up SPI
  sprite_setup(); //setup the sprites
  background_setup(); //setup the background
  startup_screen(); //show the startup screen
}
//==========================================================================================
void loop(){ //this loop will continue executing forever
  get_user_input(); //fetches any new information supplied by the user
  if(ENCODER_MODE == 1) update_set_radial(); //use SPI to read what the set radial should be
  else update_true_radial(); //find out what the true radial is
  set_target_frequency(); //adjust the frequency of the mixer

  tft.fillRect(); //erase the deflection needle with a black rectangle
  indicator.pushSprite(180,60,0); //redraw the TO/FROM indicator

  //make the needle move to the left or right
  if(DEFLECTION_POS > CURRENT_DEFLECTION_POS) CURRENT_DEFLECTION_POS++;
  else if(DEFLECTION_POS < CURRENT_DEFLECTION_POS) CURRENT_DEFLECTION_POS--;

  tft.fillRect(); //redraw the deflection needle at the correct position
  background.pushSprite(); //redraw the background
  delay(1); //delay 1 millisecond
}