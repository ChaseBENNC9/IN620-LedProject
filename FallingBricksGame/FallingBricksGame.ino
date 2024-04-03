//Setup Libraries
#include "LedControl.h" //Library that adds control to the LED Matrix
#include <SevSegShift.h> //Library that allows control of a 7 segment display using a shift register

//Setup Controller for Seven Segment Display
#define SHIFT_PIN_SHCP 7
#define SHIFT_PIN_STCP 6
#define SHIFT_PIN_DS   5


SevSegShift sevseg(SHIFT_PIN_DS, SHIFT_PIN_SHCP, SHIFT_PIN_STCP, 1, true); //Initialise the Seven Segment Display

//Setup Controller for LED MATRIX
const int CLK =  12; //Clock Pin for Matrix
const int DIN = 10; //Data Input Pin for Matrix
const int CS =  11; //CS Pin for the Matrix

LedControl lc = LedControl(DIN, CLK, CS, 2); //Initialise the Controller for the LED Matrix

//Pin Connections
const byte RIGHTBTN = 3; //Pin for the button to move Right
const byte LEFTBTN = 2; //Pin For the button to move Left

//Game Variables
int playerPos = 3; //The Current Position of the Player
int score = 0; //Current Score
int lives = 3; //Starting Lives
int currentRow = 7; //The Current Row of the falling brick
int selectedCol = 0; //The Selected Column that the falling brick will fall from
bool gameStarted = false;
//Debounce Delay for button press
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 50;





void setup() {
selectedCol = random(8);
  //Initialise Settings for Seven Seg Display
  byte numDigits = 3; //Digits that will be used
  byte digitPins[] = {8,13,4}; // Pins connected to the Digit Pins of 7 Seg display 
  byte segmentPins[] = {0, 1, 2, 3, 4, 5, 6, 7}; // these are the connections from the 7 seg display to the PINs of the ** Shift register **
  bool resistorsOnSegments = false; // 'false' means resistors are on digit pins
  byte hardwareConfig = COMMON_ANODE; //Sets configuration settings
  bool updateWithDelays = false; 
  bool leadingZeros = true; // Use 'true' if you'd like to keep the leading zeros
  bool disableDecPoint = false;
  sevseg.begin(hardwareConfig, numDigits, digitPins, segmentPins, resistorsOnSegments, updateWithDelays, leadingZeros, disableDecPoint);
  sevseg.setBrightness(100);

  Serial.begin(9600); //Setup Serial Monitor

  pinMode(RIGHTBTN, INPUT_PULLUP); //Sets the Buttons pins to input pullup
  pinMode(LEFTBTN, INPUT_PULLUP);
  //Setup Interrupt Pins
  attachInterrupt(digitalPinToInterrupt(RIGHTBTN), moveRight, RISING);
  attachInterrupt(digitalPinToInterrupt(LEFTBTN), moveLeft, RISING);

  lc.shutdown(0, false);
  lc.setIntensity(0, 5);
  lc.clearDisplay(0);
  
  lc.setLed(0, playerPos, 0, HIGH);
}

void moveRight() {
    if (!gameStarted)
  {
    gameStarted = true;
  }
  else
  {
  lc.setLed(0, playerPos, 0, LOW);

  if (playerPos < 7 && millis() - lastDebounceTime > debounceDelay) {
    playerPos++;
    lastDebounceTime = millis();
  } else if (playerPos == 7 && millis() - lastDebounceTime > debounceDelay) {
    playerPos = 0;  // Loop back to the other end of the matrix
    lastDebounceTime = millis();
  }
  lc.setLed(0, playerPos, 0, HIGH);
  }

}

void moveLeft() {
  if (!gameStarted)
  {
    gameStarted = true;
  }
  else
  {
  lc.setLed(0, playerPos, 0, LOW);
  if (playerPos > 0 && millis() - lastDebounceTime > debounceDelay) {
    playerPos--;
    lastDebounceTime = millis();
  } else if (playerPos == 0 && millis() - lastDebounceTime > debounceDelay) {
    playerPos = 7;  // Loop back to the other end of the matrix
    lastDebounceTime = millis();
  }
  lc.setLed(0, playerPos, 0, HIGH);
  }

}



unsigned long lastMoveTime = 0;     // Variable to keep track of the last movement time
unsigned long moveInterval = 1500;  // Interval between each movement (2 seconds)






void GameOver() {
  lc.clearDisplay(0);
  lc.setLed(0, playerPos, 0, HIGH);
  gameStarted = false;
  lives = 3;
  score = 0;
}



unsigned long lastDropTime = 0;
unsigned long dropInterval = 350; // Interval between drops in milliseconds
int prev = 0;
void loop() {
  if(gameStarted)
  {
  unsigned long currentTime = millis();

  if (currentTime - lastDropTime >= dropInterval) {
    if(selectedCol == prev) //If the selected Column is the same as the previous drop then randomise it again
      selectedCol = random(8);
    drop(selectedCol,currentRow);
    
    lastDropTime = currentTime;
    Serial.println(score);
    // Display score on the leftmost two digits
    float livesDec = (float) lives/10;
    Serial.println(livesDec);
    float livescore = score + livesDec;
  sevseg.setNumberF(livescore,1);


  }

  }

    sevseg.refreshDisplay(); // Must run repeatedly

}


void drop(int col,int row) {

  // Turn on the LED at the top row of the specified column
  lc.setLed(0, col, row, HIGH);  // Turn on the brick LED

  // Turn off the LED in the row above
  if (row < 7) {
    lc.setLed(0, col, row + 1, LOW);  // Turn off the brick LED in the row above
  }
  if(currentRow == 0)
  {
    currentRow = 8;
    prev = selectedCol;
    selectedCol = random(8);
    if(col == playerPos)
      score++;
    else
      if(lives > 1)
      {
        lives--;
        lc.setLed(0, col, row, LOW);
      }


      else
        GameOver();
  }
  currentRow--;

}
