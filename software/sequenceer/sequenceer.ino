#include <Adafruit_TCS34725.h>

/* Connect SCL    to analog 5
   Connect SDA    to analog 4
   Connect VDD    to 3.3V DC
   Connect GROUND to common ground */


Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_4X);

#define stp 2
#define dir 3
#define MS1 4
#define MS2 5
#define EN  6

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

  // Setup motor
  pinMode(stp, OUTPUT);
  pinMode(dir, OUTPUT);
  pinMode(MS1, OUTPUT);
  pinMode(MS2, OUTPUT);
  pinMode(EN, OUTPUT);
  resetEDPins(); //Set step, direction, microstep and enable pins to default states

  if (tcs.begin()) {
  } else {
    Serial.println("No TCS34725 found ... check your connections");
    while (1); // halt!
  }

  digitalWrite(EN, LOW); // Enable motor
  digitalWrite(dir, LOW); // Forward motor
  digitalWrite(MS1, HIGH); //Pull MS1, and MS2 high to set logic to 1/8th microstep resolution
  digitalWrite(MS2, HIGH);
  for (x = 1; x < INIT_STEP; x++) {
    digitalWrite(stp, HIGH); //Trigger one step forward
    delay(1);
    digitalWrite(stp, LOW); //Pull step pin low so it can be triggered again
    delay(1);
  }

  delay(1000);
  digitalWrite(dir, LOW); // Forward motor
  digitalWrite(MS1, HIGH); //Pull MS1, and MS2 high to set logic to 1/8th microstep resolution
  digitalWrite(MS2, HIGH);

  do_sequence();
}

void eject() {

  digitalWrite(dir, HIGH); // Reverse motor
  delay(500);
  digitalWrite(MS1, HIGH); //Pull MS1, and MS2 high to set logic to 1/8th microstep resolution
  digitalWrite(MS2, HIGH);
  for (x = 1; x < REVERSE_STEP; x++) {
    digitalWrite(stp, HIGH);
    delay(1);
    digitalWrite(stp, LOW);
    delay(1);
  }
  //digitalWrite(EN, HIGH); // Disable motor
}

void do_sequence() {
  Serial.println("C\tR\tG\tB\tGene\tBase");
  Serial.println("--------------------------------------------");
  digitalWrite(EN, LOW); // Enable motor
  digitalWrite(dir, LOW); // Forward motor
  digitalWrite(MS1, HIGH); //Pull MS1, and MS2 high to set logic to 1/8th microstep resolution
  digitalWrite(MS2, HIGH);
  sequence_base(0); //First base
  for (y = 1; y < NUM_BLOCKS; y++) {
    for (x = 1; x < BLOCK_STEP; x++) {
      digitalWrite(stp, HIGH); //Trigger one step forward
      delay(1);
      digitalWrite(stp, LOW); //Pull step pin low so it can be triggered again
      delay(1);
    }
    delay(500);
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
  }
  else {
    if(green > blue && green > red){
        Serial.print("A");
    }
    else if(red > green && red > blue){
        Serial.print("G");
    }
    else if(blue > green){
        Serial.print("T");
    }
    else{
        Serial.print("N");
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
  digitalWrite(MS1, LOW);
  digitalWrite(MS2, LOW);
  digitalWrite(EN, HIGH);
}
