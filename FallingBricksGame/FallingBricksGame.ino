#include "LedControl.h"
#include <SevSegShift.h>


#define SHIFT_PIN_SHCP 7
#define SHIFT_PIN_STCP 6
#define SHIFT_PIN_DS   5

SevSegShift sevseg(SHIFT_PIN_DS, SHIFT_PIN_SHCP, SHIFT_PIN_STCP, 1, true);


const int CLK = 12;
const int DIN = 10;
const int CS = 11;

LedControl lc = LedControl(DIN, CLK, CS, 2);
int playerPin = 3;
const byte LED = 13;
const byte RIGHTBTN = 3;
const byte LEFTBTN = 2;
int score = 0;
int lives = 3;
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 50;
int currentRow = 7;
int selectedCol = 0;



void setup() {
selectedCol = random(8);
   byte numDigits = 2;
  byte digitPins[] = {8,13}; // These are the PINS of the ** Arduino **
  byte segmentPins[] = {0, 1, 2, 3, 4, 5, 6, 7}; // these are the PINs of the ** Shift register **
  bool resistorsOnSegments = false; // 'false' means resistors are on digit pins
  byte hardwareConfig = COMMON_ANODE; // See README.md for options
  bool updateWithDelays = false; // Default 'false' is Recommended
  bool leadingZeros = false; // Use 'true' if you'd like to keep the leading zeros
  bool disableDecPoint = false; // Use 'true' if your decimal point doesn't exist or isn't connected
  sevseg.begin(hardwareConfig, numDigits, digitPins, segmentPins, resistorsOnSegments, updateWithDelays, leadingZeros, disableDecPoint);
  sevseg.setBrightness(100);

  Serial.begin(9600);

  pinMode(LED, OUTPUT);
  pinMode(RIGHTBTN, INPUT_PULLUP);
  pinMode(LEFTBTN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(RIGHTBTN), moveRight, RISING);
  attachInterrupt(digitalPinToInterrupt(LEFTBTN), moveLeft, RISING);

  lc.shutdown(0, false);
  lc.setIntensity(0, 5);
  lc.clearDisplay(0);
  lc.clearDisplay(1);
  lc.setLed(1, 0, 0, HIGH);
  Serial.print(lc.getDeviceCount());
  lc.setLed(0, 3, 0, HIGH);
}

void moveRight() {
  lc.setLed(0, playerPin, 0, LOW);

  if (playerPin < 7 && millis() - lastDebounceTime > debounceDelay) {
    playerPin++;
    lastDebounceTime = millis();
  } else if (playerPin == 7 && millis() - lastDebounceTime > debounceDelay) {
    playerPin = 0;  // Loop back to the other end of the matrix
    lastDebounceTime = millis();
  }
  lc.setLed(0, playerPin, 0, HIGH);
}

void moveLeft() {
  lc.setLed(0, playerPin, 0, LOW);
  if (playerPin > 0 && millis() - lastDebounceTime > debounceDelay) {
    playerPin--;
    lastDebounceTime = millis();
  } else if (playerPin == 0 && millis() - lastDebounceTime > debounceDelay) {
    playerPin = 7;  // Loop back to the other end of the matrix
    lastDebounceTime = millis();
  }
  lc.setLed(0, playerPin, 0, HIGH);
}



unsigned long lastMoveTime = 0;     // Variable to keep track of the last movement time
unsigned long moveInterval = 1500;  // Interval between each movement (2 seconds)






void GameOver() {
  lc.clearDisplay(0);
  lives = 3;
  score = 0;
}



int prev = 0;
unsigned long lastDropTime = 0;
unsigned long dropInterval = 350; // Interval between drops in milliseconds

void loop() {
  lc.setLed(0, playerPin, 0, HIGH);
  
  unsigned long currentTime = millis();

  if (currentTime - lastDropTime >= dropInterval) {
    drop(selectedCol,currentRow);
    
    lastDropTime = currentTime;
    sevseg.setNumber(score);

  }
    sevseg.refreshDisplay(); // Must run repeatedly

}


void drop(int col,int row) {

  Serial.println(currentRow);
  // Turn on the LED at the top row of the specified column
  lc.setLed(0, col, row, HIGH);  // Turn on the brick LED

  // Turn off the LED in the row above
  if (row < 7) {
    lc.setLed(0, col, row + 1, LOW);  // Turn off the brick LED in the row above
  }
  if(currentRow == 0)
  {
    lc.setLed(0, col, row, LOW);
    currentRow = 8;
    selectedCol = random(8);
    if(col == playerPin)
      score++;
    else
      score = 0;
  }
  currentRow--;

}
