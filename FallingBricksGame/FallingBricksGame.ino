//Setup Libraries
#include "LedControl.h"   //Library that adds control to the LED Matrix
#include <SevSegShift.h>  //Library that allows control of a 7 segment display using a shift register

//Setup Controller for Seven Segment Display
#define SHIFT_PIN_SHCP 7
#define SHIFT_PIN_STCP 6
#define SHIFT_PIN_DS 5



SevSegShift sevseg(SHIFT_PIN_DS, SHIFT_PIN_SHCP, SHIFT_PIN_STCP, 1, true);  //Initialise the Seven Segment Display
//Setup Buzzer
#define BuzzerPin A0
//Setup Controller for LED MATRIX
const int CLK = 12;  //Clock Pin for Matrix
const int DIN = 10;  //Data Input Pin for Matrix
const int CS = 11;   //CS Pin for the Matrix

LedControl lc = LedControl(DIN, CLK, CS, 2);  //Initialise the Controller for the LED Matrix

//Pin Connections
const byte RIGHTBTN = 3;  //Pin for the button to move Right
const byte LEFTBTN = 2;   //Pin For the button to move Left

//Game Variables
int playerPos = 3;    //The Current Position of the Player
int score = 0;        //Current Score
int lives = 3;        //Starting Lives
int currentRow = 7;   //The Current Row of the falling brick
int selectedCol = 0;  //The Selected Column that the falling brick will fall from
bool gameStarted = false;

//Debounce Delay for button press
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 50;

int dropInterval = 350;  // Interval between drops in milliseconds


void ResetGame(bool death = false) {
  if (death) {
    Death();
  }
  lc.clearDisplay(0);
  lc.shutdown(0, false);
  lc.setIntensity(0, 5);
  dropInterval = 350;
  playerPos = 3;
  lc.setLed(0, playerPos, 0, HIGH);
  gameStarted = false;
  lives = 3;
  score = 0;
}


void setup() {
  selectedCol = random(8);
  //Initialise Settings for Seven Seg Display
  SevSegSetup();
  PinsSetup();
  ResetGame();
}

void PinsSetup() {
  Serial.begin(9600);               //Setup Serial Monitor
  pinMode(RIGHTBTN, INPUT_PULLUP);  //Sets the Buttons pins to input pullup
  pinMode(LEFTBTN, INPUT_PULLUP);
  pinMode(A0, OUTPUT);
  //Setup Interrupt Pins
  attachInterrupt(digitalPinToInterrupt(RIGHTBTN), moveRight, RISING);
  attachInterrupt(digitalPinToInterrupt(LEFTBTN), moveLeft, RISING);
}
void SevSegSetup() {
  byte numDigits = 3;                               //Digits that will be used
  byte digitPins[] = { 8, 13, 4 };                  // Pins connected to the Digit Pins of 7 Seg display. The first second and fourth digit are connected to pins 8 , 13 and 4
  byte segmentPins[] = { 0, 1, 2, 3, 4, 5, 6, 7 };  // these are the connections from the 7 seg display to the PINs of the ** Shift register ** Pins A-G are connected to 0-7 of the shift register
  bool resistorsOnSegments = false;                 // 'false' means resistors are on digit pins
  byte hardwareConfig = COMMON_ANODE;               //Sets configuration settings
  bool updateWithDelays = false;
  bool leadingZeros = true;  // Use 'true' if you'd like to keep the leading zeros
  bool disableDecPoint = false;
  sevseg.begin(hardwareConfig, numDigits, digitPins, segmentPins, resistorsOnSegments, updateWithDelays, leadingZeros, disableDecPoint);
  sevseg.setBrightness(100);
}
void moveRight() {
  if (!gameStarted) {  //If the game hasn't started then a button press will start the game
    gameStarted = true;
  } else {
    lc.setLed(0, playerPos, 0, LOW);

    if (playerPos < 7 && millis() - lastDebounceTime > debounceDelay) {  //Adds in a slight delay so multiple presses are not detected at once
      playerPos++;                                                       //move the player to the right
      lastDebounceTime = millis();
    } else if (playerPos == 7 && millis() - lastDebounceTime > debounceDelay) {
      playerPos = 0;  // when the player reaches one end Loop back to the other end of the matrix
      lastDebounceTime = millis();
    }
    lc.setLed(0, playerPos, 0, HIGH);
  }
}
//Same as MoveRight, Moves in oppisite direction
void moveLeft() {
  if (!gameStarted) {
    gameStarted = true;
  } else {
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
unsigned long moveInterval = 1500;  // Interval between each movement
unsigned long lastDropTime = 0;     //the last time a brick was dropped

int prev = 0;
void loop() {
  if (gameStarted) {

    unsigned long currentTime = millis();

    if (currentTime - lastDropTime >= dropInterval) {
      if (selectedCol == prev)  //If the selected Column is the same as the previous drop then randomise it again
        selectedCol = random(8);
      drop(selectedCol, currentRow);

      lastDropTime = currentTime;
      Serial.println(score);
      // Display score on the leftmost two digits
      float livesDec = (float)lives / 10; //this turns the current lives into a decimal e.g 0.3 so it can be displayed on the rightmost digit
      Serial.println(livesDec);
      float livescore = score + livesDec; //this adds the decimal of lives and score together to display them at same time e.g 5.3 is 5 points and 3 lives and will display as 05 3
      sevseg.setNumberF(livescore, 1); //sets the display at 1dp so it shows in correct positions
    }
  }

  sevseg.refreshDisplay();  // Must run repeatedly
}


void drop(int col, int row) {

  // Turn on the LED at the top row of the specified column
  lc.setLed(0, col, row, HIGH);  // Turn on the brick LED

  if (row < 7) {
    lc.setLed(0, col, row + 1, LOW);  // Turn off the brick LED in the row above
  }
  if (currentRow == 0) {
    currentRow = 8;
    prev = selectedCol; //updates orev to the current selectedCol
    selectedCol = random(8); //randomises selectedCol
    if (col == playerPos) {
      score++;            //increases the score
      dropInterval -= 5;  //decreases the drop interval when the player scores a point (faster movements)

      Score();  //Playe the score sound on the buzzer
    }

    else if (lives > 1) {           //if the player missed the brick and the lives are more than 1
      lives--;                      //decrease lives
      Miss();                       //play the miss sound
      lc.setLed(0, col, row, LOW);  //turn off the selected led
    }


    else                //if the player missed the brick and the player lves are 1
      ResetGame(true);  //Resets the game and pass in true for GameOver
  }
  currentRow--;  //set the current row to the one below it
}


void Miss() {  //this sound will play when the player misses a brick
  for (int i = 1; i < 6; i++) {
    tone(BuzzerPin, 200 / i, 500);  //Play a series of notes decreasing in frequency

    int pauseBetweenNotes = 150;  //pause for 150ms
    delay(pauseBetweenNotes);
    // stop the tone playing:
    noTone(BuzzerPin);
  }
}
void Death() {  //THis sound will play when the player has 0 lives plays lower frequencies to distinguish it from miss()
  for (int i = 1; i < 12; i++) {
    tone(BuzzerPin, 400 / i, 500);
    delay(200);
    // stop the tone playing:
    noTone(BuzzerPin);
  }
}
void Score() {  //this sound will play when the player scores a point
  for (int i = 400; i <= 800; i += 400) {
    tone(BuzzerPin, i, 200);  //play a series of notes in quick succession , increases in frequency
    delay(75);
    noTone(BuzzerPin);
  }
}
