#include "SPI.h"
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"
#include "Adafruit_TCS34725.h"
#include "BasicStepperDriver.h"

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
#define TFT_RST 7
#define TXT_H 8
#define TXT_W 5
#define GRAPH_TOP 17
#define GRAPH_BOTTOM 69
#define GRAPH_MAX 15000.0
#define TFT_WIDTH 320
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);

// Motor
#define STP 3
#define DIR 4
#define EN  5

#include "A4988.h"
#define MS1 6
#define MS2 8 
#define MS3 2 // lol it cant be 7 ffs

#define MOTOR_STEPS 200
#define RPM 120
A4988 stepper(MOTOR_STEPS, DIR, STP, EN, MS1, MS2, MS3);

#define RST_BTN 12

int BLOCK_STEP = 165*2;
int INIT_STEP = 270*2;
int NUM_BLOCKS = 10;
int REVERSE_STEP = ((NUM_BLOCKS) * BLOCK_STEP) + INIT_STEP;

int GRAPH_STEP = 1;
int GRAPH_FLAG = (BLOCK_STEP / (TFT_WIDTH/(10*GRAPH_STEP)));

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

  // Setup motor
  stepper.begin(RPM);
  stepper.setMicrostep(16);

  // Setup screen
  tft.begin();
  tft.setRotation(3);
  screenWelcome();

  // Setup buttan
  pinMode(RST_BTN, INPUT);

  if (tcs.begin()) {
  } else {
    Serial.println("No TCS34725 found ... check your connections");
    while (1); // halt!
  }

  // LED off to prevent overheating
  tcs.setInterrupt(true);

  while(true){
    screenWaiting();
    while(digitalRead(RST_BTN) == HIGH); // wait for buttan press
    
    screenMiniWelcome();
    stepper.enable();
    stepper.move(-INIT_STEP);
    delay(1000); // pause for dramatic effect
    do_sequence();
    stepper.disable();

    tft.setTextSize(1);
    tft.setTextColor(ILI9341_GREEN, ILI9341_BLACK);
    tft.setCursor(0, 240-TXT_H);
    tft.println("STATUS: SEQUENCING COMPLETE! WAITING FOR BUTTAN PRESS...");

    unsigned long time_called = millis();
    while(digitalRead(RST_BTN) == HIGH){ if (millis() - time_called > 90000UL){ break; }}
  }
}

void eject() {
  // LED off
  tcs.setInterrupt(true);
  delay(500);

  stepper.move(REVERSE_STEP);

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
  tft.println("h_v1.9 s_v20180312");

  // Graph space
  tft.drawFastHLine(0, GRAPH_TOP-1, TFT_WIDTH, ILI9341_WHITE);
  tft.drawFastHLine(0, GRAPH_BOTTOM+1, TFT_WIDTH, ILI9341_WHITE);

  // Footer
  tft.setCursor(0, 240-TXT_H);
  tft.println("STATUS: SEQUENCING MONSTER GENOME...");
  tft.drawFastHLine(0, 240-TXT_H-2, TFT_WIDTH, ILI9341_WHITE);

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



  int graph_pos = 0;
  float r,g,b,c;
  uint16_t clear_l = GRAPH_BOTTOM-GRAPH_TOP;
  uint16_t red_l = GRAPH_BOTTOM-GRAPH_TOP;
  uint16_t green_l = GRAPH_BOTTOM-GRAPH_TOP;
  uint16_t blue_l = GRAPH_BOTTOM-GRAPH_TOP;
  uint16_t clear, red, green, blue;
  
  for (y = 0; y < NUM_BLOCKS; y++) {
    //stepper.move(-BLOCK_STEP);
    
    for (x = 1; x < BLOCK_STEP; x++) {
      stepper.move(-1);
      if (x % GRAPH_FLAG == 0) {
        //delay(10);  // takes 50ms to read but fuck it
        tcs.getRawData(&red, &green, &blue, &clear);
        red = red + random(-1000, 1000);
        green = green + random(-1000, 1000);
        blue = blue + random(-1000, 1000);
        clear = clear + random(-1000, 1000);

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

  delay(60);  // takes 50ms to read
  tcs.getRawData(&red, &green, &blue, &clear);
  
  Serial.print(clear);
  Serial.print("\t"); Serial.print(red);
  Serial.print("\t"); Serial.print(green);
  Serial.print("\t"); Serial.print(blue);
  Serial.print("\t"); Serial.print(genes[gene_i]);
  Serial.print("\t");
  if(clear > 15000){
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
  int message_block = TFT_WIDTH/n_messages;
  int curr_msg = 0;
  
  for (x = 0; x < TFT_WIDTH; x++) {
      tft.drawLine(x-1, 180, x, 180, ILI9341_BLUE);
      delay(20);

      if(x == 0 || x % message_block == 0){
          tft.setCursor(0, offset+90);
          tft.println(messages[curr_msg]);

          if (curr_msg + 1 < n_messages){
            curr_msg = curr_msg + 1;
          }
      }
  }
}


void screenMiniWelcome() {
  tft.setTextWrap(false);
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
  tft.setTextColor(ILI9341_YELLOW, ILI9341_BLACK);


  char* messages[]={
  "    Starting Sequencer         ",
  };

  int n_messages = 1;
  int message_block = TFT_WIDTH/n_messages;
  int curr_msg = 0;
  
  for (x = 0; x < TFT_WIDTH; x++) {
      tft.drawLine(x-1, 180, x, 180, ILI9341_BLUE);
      delay(10);

      if(x == 0 || x % message_block == 0){
          tft.setCursor(0, offset+90);
          tft.println(messages[curr_msg]);

          if (curr_msg + 1 < n_messages){
            curr_msg = curr_msg + 1;
          }
      }
  }
  tft.setTextWrap(true);
}




void screenWaiting() {
 
  tft.setTextWrap(false);
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
  tft.setTextColor(ILI9341_GREEN, ILI9341_BLACK);

  tft.setCursor(0, offset+90);
  tft.println("    WAITING TO SEQUENCE       ");
  tft.setTextWrap(true);

}
