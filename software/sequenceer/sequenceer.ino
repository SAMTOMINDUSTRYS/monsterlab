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
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);
#define TXT_H 8
#define TXT_W 5

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
  testText();

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
  tft.drawFastHLine(0, TXT_H*2, 320, ILI9341_WHITE);

  tft.setCursor(0, 240-TXT_H);
  tft.println("Sample ID: 105-118-674-00399");
  tft.drawFastHLine(0, 240-TXT_H-2, 320, ILI9341_WHITE);

  tft.setTextSize(2);
  tft.setTextColor(ILI9341_RED);  tft.setTextSize(2);
  tft.setCursor(TXT_W*3, (TXT_H*2)+(TXT_H));
  tft.println("OCU1 OCU2 MEM1 MEM2 FRM1");
  
  tft.setCursor(TXT_W*3, (TXT_H*2)+(TXT_H*2)+(TXT_H*10)+(TXT_H*2));
  tft.println("FRM2 MAG1 PT10 DWF7 SPEC");

  tft.setCursor(TXT_W*2, (TXT_H*2)+(TXT_H*2)+(TXT_H*2));
  tft.setTextColor(ILI9341_WHITE);  tft.setTextSize(10);
  
  Serial.println("C\tR\tG\tB\tGene\tBase");
  Serial.println("--------------------------------------------");
  tcs.setInterrupt(false);      // turn on LED
  //digitalWrite(EN, LOW); // Enable motor
  digitalWrite(dir, LOW); // Forward motor
  //digitalWrite(MS1, HIGH); //Pull MS1, and MS2 high to set logic to 1/8th microstep resolution
  //digitalWrite(MS2, HIGH);
  sequence_base(0); //First base
  for (y = 1; y < NUM_BLOCKS; y++) {
    for (x = 1; x < BLOCK_STEP; x++) {
      digitalWrite(stp, HIGH); //Trigger one step forward
      delay(1);
      digitalWrite(stp, LOW); //Pull step pin low so it can be triggered again
      delay(1);
    }
    delay(500);

    if (y == 5){
        tft.setCursor(TXT_W*2, (TXT_H*2)+(TXT_H*2)+(TXT_H*10)+(TXT_H*2)+(TXT_H*2)+(TXT_H)+(TXT_H*2));
    }
    sequence_base(y);


  } 
  eject();
}

void sequence_base(int gene_i) {
  uint16_t clear, red, green, blue;
  tcs.setInterrupt(false);      // turn on LED

  delay(60);  // takes 50ms to read
  tcs.getRawData(&red, &green, &blue, &clear);
  tcs.setInterrupt(true);  // turn off LED
  
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


unsigned long testText() {
  tft.fillScreen(ILI9341_BLACK);
  unsigned long start = micros();
  tft.setCursor(0, 0);
  tft.setTextColor(ILI9341_WHITE);  tft.setTextSize(1);
  tft.println("Hello World!");
  tft.setTextColor(ILI9341_YELLOW); tft.setTextSize(2);
  tft.println(1234.56);
  tft.setTextColor(ILI9341_RED);    tft.setTextSize(3);
  tft.println(0xDEADBEEF, HEX);
  tft.println();
  tft.setTextColor(ILI9341_GREEN);
  tft.setTextSize(5);
  tft.println("Monster Lab");
  tft.setTextSize(2);
  tft.println("I implore thee,");
  tft.setTextSize(1);
  tft.println("my foonting turlingdromes.");
  tft.println("And hooptiously drangle me");
  tft.println("with crinkly bindlewurdles,");
  tft.println("Or I will rend thee");
  tft.println("in the gobberwarts");
  tft.println("with my blurglecruncheon,");
  tft.println("see if I don't!");
  return micros() - start;
}

void resetScreen(){
  tft.fillScreen(ILI9341_BLACK);
  tft.setCursor(0, 0);
}

unsigned long screenBase() {
  tft.fillScreen(ILI9341_BLACK);
  unsigned long start = micros();
  tft.setCursor(0, 0);
  tft.setTextColor(ILI9341_WHITE);  tft.setTextSize(1);
  tft.println("Hello World!");
  tft.setTextColor(ILI9341_YELLOW); tft.setTextSize(2);
  tft.println(1234.56);
  tft.setTextColor(ILI9341_RED);    tft.setTextSize(3);
  tft.println(0xDEADBEEF, HEX);
  tft.println();
  tft.setTextColor(ILI9341_GREEN);
  tft.setTextSize(5);
  tft.println("Monster Lab");
  tft.setTextSize(2);
  tft.println("I implore thee,");
  tft.setTextSize(1);
  tft.println("my foonting turlingdromes.");
  tft.println("And hooptiously drangle me");
  tft.println("with crinkly bindlewurdles,");
  tft.println("Or I will rend thee");
  tft.println("in the gobberwarts");
  tft.println("with my blurglecruncheon,");
  tft.println("see if I don't!");
  return micros() - start;
}
