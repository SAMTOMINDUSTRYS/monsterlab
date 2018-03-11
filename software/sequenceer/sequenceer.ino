#include "SPI.h"
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"
#include "Adafruit_TCS34725.h"

/* 
   RGB Colour Sensor
     Connect SCL    to analog 5
     Connect SDA    to analog 4
     Connect VDD    to 3.3V DC
     Connect GROUND to common ground

   Screen
      GND connects to ground - black wire
      VIN connects to +5V - red wire
      DC (data/clock) connects to digital 9 on Atmega328
      Skip SDCS (SD card chip select - used for SD card interfacing)
      CS (chip select) connects to digital 10 on Atmega328
      MOSI (data out) connects to digital 11 on Atmega328
      SCK (clock) connects to digital 13 on Atmega328
      Skip MISO (data in - used for SD card interfacing)

   Motor
      STEP P3
      DIR P4    ## or do them the wrong way around ffs tom
      VDD and GND to usual spot
*/

// Colour Sensor
Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_4X);

// Screen
#define TFT_DC 9
#define TFT_CS 10
#define TXT_H 8
#define TXT_W 5
#define GRAPH_TOP 17
#define GRAPH_BOTTOM 69
#define GRAPH_MAX 15000.0
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);

// Motor
#define stp 4
#define dir 3
//#define MS1 4
//#define MS2 5
//#define EN  6


int BLOCK_STEP = 165;
int INIT_STEP = 350;
int NUM_BLOCKS = 10;
int REVERSE_STEP = ((NUM_BLOCKS-1) * BLOCK_STEP) + INIT_STEP;

int GRAPH_STEP = 1;
int GRAPH_FLAG = (BLOCK_STEP / (320/(10*GRAPH_STEP)));

int state;
int x;
int y;

char* genes[]={
  "OCU1",
  "OCU2",
  "MEM1",
  "MEM2",
  "FRM1",
  "FRM2",
  "MAG1",
  "PAT10",
  "DWF7",
  "SPEC"
};

void setup() {
  Serial.begin(9600); //Open Serial connection for debugging

  // Setup screen
  tft.begin();
  tft.setRotation(1);
  screenWelcome();

  // Setup motor
  pinMode(stp, OUTPUT);
  pinMode(dir, OUTPUT);
  //pinMode(MS1, OUTPUT);
  //pinMode(MS2, OUTPUT);
  //pinMode(EN, OUTPUT);


  
  resetEDPins(); //Set step, direction, microstep and enable pins to default states

  if (tcs.begin()) {
  } else {
    Serial.println("No TCS34725 found ... check your connections");
    while (1); // halt!
  }

  // LED off to prevent overheating
  tcs.setInterrupt(true);

  //digitalWrite(EN, LOW); // Enable motor
  digitalWrite(dir, LOW); // Forward motor
  //digitalWrite(MS1, HIGH); //Pull MS1, and MS2 high to set logic to 1/8th microstep resolution
  //digitalWrite(MS2, HIGH);
  for (x = 1; x < INIT_STEP; x++) {
    digitalWrite(stp, HIGH); //Trigger one step forward
    delay(1);
    digitalWrite(stp, LOW); //Pull step pin low so it can be triggered again
    delay(1);
  }

  delay(1000);
  digitalWrite(dir, LOW); // Forward motor
  //digitalWrite(MS1, HIGH); //Pull MS1, and MS2 high to set logic to 1/8th microstep resolution
  //digitalWrite(MS2, HIGH);

  do_sequence();
}

void eject() {
  // LED off
  tcs.setInterrupt(true);
  digitalWrite(dir, HIGH); // Reverse motor
  delay(500);
  //digitalWrite(MS1, HIGH); //Pull MS1, and MS2 high to set logic to 1/8th microstep resolution
  //digitalWrite(MS2, HIGH);
  for (x = 1; x < REVERSE_STEP; x++) {
    digitalWrite(stp, HIGH);
    delay(1);
    digitalWrite(stp, LOW);
    delay(1);
  }
  //digitalWrite(EN, HIGH); // Disable motor
}

void do_sequence() {
  // Blank the screen and set font
  resetScreen();

  tft.setTextColor(ILI9341_YELLOW);  tft.setTextSize(2);
  tft.print("Monster Lab");

  tft.setTextSize(1);
  tft.setCursor(TXT_W*2*15, 0);
  tft.println("Legogen Sequenceer 9002");
  tft.setCursor(TXT_W*2*15, TXT_H);
  tft.println("Hoot hoot fuckers");

  // Graph space
  tft.drawFastHLine(0, GRAPH_TOP-1, 320, ILI9341_WHITE);
  tft.drawFastHLine(0, GRAPH_BOTTOM+1, 320, ILI9341_WHITE);

  // Footer
  tft.setCursor(0, 240-TXT_H);
  tft.println("Sample ID: 105-118-674-00399");
  tft.drawFastHLine(0, 240-TXT_H-2, 320, ILI9341_WHITE);

  tft.setTextSize(2);
  tft.setTextColor(ILI9341_RED);  tft.setTextSize(2);
  tft.setCursor(TXT_W*3, (TXT_H*2)+(TXT_H)+(TXT_H*2)+(TXT_H*2)+(TXT_H*2));
  tft.println("OCU1 OCU2 MEM1 MEM2 FRM1");
  
  tft.setCursor(TXT_W*3, (TXT_H*2)+(TXT_H*2)+(TXT_H*10)+(TXT_H*2)+(TXT_H*2)+(TXT_H));
  tft.println("FRM2 MAG1 PT10 DWF7 SPEC");

  tft.setCursor(TXT_W*1, (TXT_H*2)+(TXT_H*2)+(TXT_H*2)+(TXT_H*2)+(TXT_H*2)+(TXT_H));
  tft.setTextColor(ILI9341_WHITE);  tft.setTextSize(8);
  
  Serial.println("C\tR\tG\tB\tGene\tBase");
  Serial.println("--------------------------------------------");
  tcs.setInterrupt(false);      // turn on LED
  //digitalWrite(EN, LOW); // Enable motor
  digitalWrite(dir, LOW); // Forward motor
  //digitalWrite(MS1, HIGH); //Pull MS1, and MS2 high to set logic to 1/8th microstep resolution
  //digitalWrite(MS2, HIGH);


  int graph_pos = 0;
  float r,g,b,c;
  uint16_t clear_l = GRAPH_BOTTOM-GRAPH_TOP;
  uint16_t red_l = GRAPH_BOTTOM-GRAPH_TOP;
  uint16_t green_l = GRAPH_BOTTOM-GRAPH_TOP;
  uint16_t blue_l = GRAPH_BOTTOM-GRAPH_TOP;
  uint16_t clear, red, green, blue;
  
  //sequence_base(0); //First base
  for (y = 0; y < NUM_BLOCKS; y++) {
    for (x = 1; x < BLOCK_STEP; x++) {
      digitalWrite(stp, HIGH); //Trigger one step forward
      delay(1);
      digitalWrite(stp, LOW); //Pull step pin low so it can be triggered again
      delay(1);

      if (x % GRAPH_FLAG == 0) {
        delay(12);  // takes 50ms to read but fuck it
        tcs.getRawData(&red, &green, &blue, &clear);
        
        if (red > GRAPH_MAX){ red = GRAPH_MAX; }; r = 1.0-(red/GRAPH_MAX); if (r < 0){ r = 0; };
        if (green > GRAPH_MAX){ green = GRAPH_MAX; }; g = 1.0-(green/GRAPH_MAX); if (g < 0){ g = 0; };
        if (blue > GRAPH_MAX){ blue = GRAPH_MAX; }; b = 1.0-(blue/GRAPH_MAX); if (b < 0){ b = 0; };
        if (clear > GRAPH_MAX){ clear = GRAPH_MAX; }; c = 1.0-(clear/GRAPH_MAX); if (c < 0){ c = 0; };

        red = r * (GRAPH_BOTTOM-GRAPH_TOP);
        green = g * (GRAPH_BOTTOM-GRAPH_TOP);
        blue = b * (GRAPH_BOTTOM-GRAPH_TOP);
        clear = c * (GRAPH_BOTTOM-GRAPH_TOP);

        tft.drawLine((graph_pos-1)*GRAPH_STEP, GRAPH_TOP+red_l, graph_pos*GRAPH_STEP, GRAPH_TOP+red, ILI9341_RED);
        tft.drawLine((graph_pos-1)*GRAPH_STEP, GRAPH_TOP+green_l, graph_pos*GRAPH_STEP, GRAPH_TOP+green, ILI9341_GREEN);
        tft.drawLine((graph_pos-1)*GRAPH_STEP, GRAPH_TOP+blue_l, graph_pos*GRAPH_STEP, GRAPH_TOP+blue, ILI9341_BLUE);
        tft.drawLine((graph_pos-1)*GRAPH_STEP, GRAPH_TOP+clear_l, graph_pos*GRAPH_STEP, GRAPH_TOP+clear, ILI9341_WHITE);

        red_l = red;
        green_l = green;
        blue_l = blue;
        clear_l = clear;
        graph_pos = graph_pos + 1;
    }
      
    }
    delay(500);

    if (y == 5){
        tft.setCursor(TXT_W, (TXT_H*2)+(TXT_H*2)+(TXT_H*10)+(TXT_H*2)+(TXT_H*2)+(TXT_H)+(TXT_H*2));
    }
    tft.setTextSize(2);tft.print(" ");tft.setTextSize(8);
    sequence_base(y);

  } 
  eject();
}

uint16_t* sequence_base(int gene_i) {
  uint16_t clear, red, green, blue;
  //tcs.setInterrupt(false);      // turn on LED

  delay(60);  // takes 50ms to read
  tcs.getRawData(&red, &green, &blue, &clear);
  //tcs.setInterrupt(true);  // turn off LED
  
  Serial.print(clear);
  Serial.print("\t"); Serial.print(red);
  Serial.print("\t"); Serial.print(green);
  Serial.print("\t"); Serial.print(blue);
  Serial.print("\t"); Serial.print(genes[gene_i]);
  Serial.print("\t");
  if(clear > 10000){
    Serial.print("C");
    tft.print("C");
  }
  else {
    if(green > blue && green > red){
        Serial.print("A");
        tft.print("A");
    }
    else if(red > green && red > blue){
        Serial.print("G");
        tft.print("G");
    }
    else if(blue > green){
        Serial.print("T");
        tft.print("T");
    }
    else{
        Serial.print("N");
        tft.print("N");
    }
  }
  Serial.println();

}

//Main loop
void loop() {

}

//Reset Easy Driver pins to default states
void resetEDPins()
{
  digitalWrite(stp, LOW);
  digitalWrite(dir, LOW);
  //digitalWrite(MS1, LOW);
  //digitalWrite(MS2, LOW);
  //digitalWrite(EN, HIGH);
}

void resetScreen(){
  tft.fillScreen(ILI9341_BLACK);
  tft.setCursor(0, 0);
}

void screenWelcome() {
  tft.fillScreen(ILI9341_WHITE);
  tft.setTextColor(ILI9341_BLACK);

  int offset = 55;
  tft.setCursor(95, offset);
  tft.setTextSize(1);
  tft.println("Sam and Tom Industrys");

  tft.setCursor(27, offset+20);
  tft.setTextSize(4);
  tft.println("MONSTER LAB");

  tft.setCursor(20, offset+55);
  tft.setTextSize(2);
  tft.println("Legogen Sequenceer 9002");

  tft.setCursor(0, offset+90);
  tft.setTextSize(2);
  tft.setTextColor(ILI9341_RED, ILI9341_WHITE);


  char* messages[]={
  "  Preparing Pyrosequences",
  "       Opening Pores        ",
  "   Simulating Evolution   ",
  "     Hunting Monsters   ",
  "  Stirring Genetic Pool  ",
  "   Reticulating Splines",
  "  OMG THIS ACTUALLY WORKS"
  };

  int n_messages = 7;
  int message_block = 320/n_messages;
  int curr_msg = 0;
  
  for (x = 0; x < 320; x++) {
      tft.drawLine(x-1, 180, x, 180, ILI9341_BLUE);
      delay(25);

      if(x == 0 || x % message_block == 0){
          tft.setCursor(0, offset+90);
          tft.println(messages[curr_msg]);

          if (curr_msg + 1 < n_messages){
            curr_msg = curr_msg + 1;
          }
      }
  }
}
