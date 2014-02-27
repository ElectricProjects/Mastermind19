
/*************************************************************************
 * 
 * Mastermind vs 19
 * These chips use I2C to communicate
 * Thanks to Adafruit
 *
 * Changed Arduino core Tone to use timer 4 on Mega.
 * Used same timer as TLC5940
 * #define AVAILABLE_TONE_PINS 1
 * #define USE_TIMER4 */
// * const uint8_t PROGMEM tone_pin_to_timer_PGM[] = { 4 /*, 3, 4, 5, 1, 0 */ };

//static uint8_t tone_pins[AVAILABLE_TONE_PINS] = { 255 /*, 255, 255, 255, 255, 255 */ };

// Thanks to forum moderator Coding Badly help with timer

/**************************************************************************/

#include <Wire.h>
#include <Adafruit_NFCShield_I2C.h>
#include <avr/pgmspace.h>
#include "pitches.h"
#include "Tlc5940.h"
#include <LiquidCrystal.h>
#define IRQ   (2)
#define RESET (4)  // Not connected by default on the NFC Shield

int guessNum=0; // 4 guesses per turn
int turn =0; 
int correctAnsw = 0;
int almostCorAnsw = 0;
int secretCode[4];
int secretGuess[4];
int card=0;
int randNumber=0;
int channel=4;
int guess=1;
int wins=0;
int Button=6;
int turns;
int x;
int i;
int buttonState = HIGH;             
int lastButtonState = LOW; 
long lastDebounceTime = 0; 
long debounceDelay = 100;   
int melodyCard0[] = {NOTE_B2, NOTE_B3, NOTE_B4};
int melodyCard1[] = {NOTE_C4};
int melodyCard2[] = {NOTE_B5};
int melodyCard3[] = {NOTE_A3};
int melodyWin[] = {NOTE_C2, NOTE_D3, NOTE_E4,NOTE_C2, NOTE_D3, NOTE_E4,NOTE_C2, NOTE_D3, NOTE_E4,NOTE_E5};
int melodyLose[] = {NOTE_B4, NOTE_B2};
int melodyStartup[] = {NOTE_C2, NOTE_D3, NOTE_E4,NOTE_F4};
int melodyTurnGreen[] = {NOTE_B2,};
int melodyTurnRed[] = {NOTE_A2,};
int melodyTurnBlue[] = {NOTE_C4,};

// note durations: 4 = quarter note, 8 = eighth note, etc.:

int noteDurationsTurn[] = {4};
int noteDurationsCard1[] = {2};
int noteDurationsCard0[] = {8,8,4};
int noteDurationsCardWin[] = {4,4,2,4,4,2,4,4,4,2};
int noteDurationsCardLose[] = {4,2};
int noteDurationsStartup[] = {2,2,2,1};

Adafruit_NFCShield_I2C nfc(IRQ, RESET);
//LiquidCrystal lcd(7, 8, 9, 10, 11, 12);
//LiquidCrystal lcd(22, 23, 24, 25, 26, 27);
LiquidCrystal lcd(23, 25, 27, 29, 31, 33);

void setup(void) {
  Serial.begin(9600);
  lcd.begin(16, 2);
  lcd.print(F("** Mastermind **"));
  pinMode(Button, INPUT);
  digitalWrite(Button, HIGH);  
  Tlc.init();
  Tlc.clear(); 
  playToneStartup();
  startupLED();
  randomSeed(analogRead(0));
  Serial.println(F("*** MASTERMIND ***"));
  lcd.setCursor(0,1);
  lcd.clear();
  lcd.print(F("Can you"));
  lcd.setCursor(0,1);
  lcd.print(F("outsmart me?"));
  delay(1000);
  secretCodeMaker();
  lcd.clear();
  lcd.print("Push the button");
  lcd.setCursor(0,1);
  lcd.print("for directions.");
  Serial.println(F("Push the button for directions."));
  buttonPush3();
  nfc.begin();

  Serial.println(F("Starting NFC"));
  uint32_t versiondata = nfc.getFirmwareVersion();

  if (! versiondata) {
    Serial.print("Didn't find PN53x board");
    while (1); // halt
  }
  nfc.SAMConfig(); // configure board to read RFID tags
}

uint8_t uid[] = {0, 0, 0, 0, 0, 0, 0 };  // Buffer to store the returned UID

void loop(void) {
  Serial.println(F("Make a guess ..."));
  lcd.clear();
  lcd.print(F("Make a guess..."));
  uint8_t success;
  uint8_t uidLength;
  // Wait for an ISO14443A type cards (Mifare, etc.).  When one is found
  // 'uid' will be populated with the UID, and uidLength will indicate
  // if the uid is 4 bytes (Mifare Classic) or 7 bytes (Mifare Ultralight)

  success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength);

  if (success) {
    Serial.println("Found a card");
    if (uidLength == 4)
    {
      // We probably have a Mifare Classic card ...
      uint8_t keya[6] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

      // Start with block 4 (the first block of sector 1)
      success = nfc.mifareclassic_AuthenticateBlock(uid, uidLength, 4, 0, keya);
      if (success)
      {
        uint8_t data[16];
        success = nfc.mifareclassic_ReadDataBlock(4, data);

        if (success)
        {
          // Data seems to have been read
          arrayCompare();
          guessNum++;
          if(guessNum>3)
          {
            guessNum=0;
            delay(50);
            verfiyAnswer();
          }
          // Wait a bit before reading the card again
          delay(500);
        }

        else

        {
          Serial.println(F("Unable to read card. again."));
          lcd.clear();
          lcd.print(F("Unable to read card."));
        }
      }
      else

      {
        Serial.println(F("Authentication failed?"));
      }
    }
  }
}


void arrayCompare()
{
  lcd.clear();
  Serial.print(F(" Guess # "));
  lcd.print(F("Guess #"));
  lcd.setCursor(9,0);
  Serial.print(guessNum+1);
  lcd.print(guessNum+1, DEC);

  uint8_t card1[] = {0x1B, 0xCB, 0x78, 0x95    }; // red
  uint8_t card2[] = {0xCA, 0x19, 0xEE, 0x65    }; //green
  uint8_t card3[] = {0xEA, 0x38, 0x82, 0xB7    }; // blue

  if(memcmp(card1, uid, 4) == 0)
  {
    Serial.println(F(" is red."));
    lcd.setCursor(0,1);
    lcd.print(F(" is red."));
    card=1;
    secretGuess[guessNum]=1;
    playToneTurnRed();
  }

  if(memcmp(card2, uid, 4) == 0)
  {
    Serial.println(F(" is green."));
    lcd.setCursor(0,1);
    lcd.print(F(" is green."));
    card=2;
    secretGuess[guessNum]=2;
    playToneTurnGreen();
  }

  if(memcmp(card3, uid, 4) == 0)
  {
    Serial.println(F(" is blue."));
    lcd.setCursor(0,1);
    lcd.print(F(" is blue."));
    card=3;
    secretGuess[guessNum]=3;
    playToneTurnBlue();
  }

  if(guessNum==0)
  {
    channel=(card-1);
    Tlc.set(channel, 3095);
    Tlc.update();
  }

  if(guessNum==1)
  {
    channel=(card+2);
    Tlc.set(channel, 3095);
    Tlc.update();
  }

  if(guessNum==2)
  {
    channel=(card+5);
    Tlc.set(channel, 3095);
    Tlc.update();
  }

  if(guessNum==3)
  {
    channel=(card+8);
    Tlc.set(channel, 3095);
    Tlc.update();
  }

  if(guessNum>3)
  {
    guessNum=0;
    delay(500);
    verfiyAnswer();
  }
}


void secretCodeMaker()
{
  for (int i=0; i<4; i++)
  {
    randNumber = random(1,4);
    secretCode[i] = randNumber;
    Serial.print(F("LED "));
    Serial.print(i);
    Serial.print(F(" = "));
    Serial.print(secretCode[i]);
    if (secretCode[i]==1)
    {
      Serial.print(F(" Red"));
    }
    else if (secretCode[i]==2)

    {
      Serial.print(F(" Green"));
    }

    else
    {
      Serial.print(F(" Blue"));
    }

    Serial.println(F(" "));
  }
}

void verfiyAnswer()
{
  for (int i=0; i<4; i++)
  {
    Serial.print(F("Code "));
    Serial.print(i);
    Serial.print(" = ");
    Serial.print(secretCode[i]);
    Serial.print(F(" Guess =  "));
    Serial.println(secretGuess[i]);
    if (secretCode[i] == secretGuess[i])
    {
      almostCorAnsw++;
    }


    if (secretCode[i] == secretGuess[i])
    {
      correctAnsw++;

      if (i==0)
      {
        channel=i+secretCode[i];
      }
      else if (i==1)

      {
        channel=i+2+secretCode[i];
      }
      else if (i==2)

      {
        channel=i+5+secretCode[i];
      }

      else  

      {
        channel=i+7+secretCode[i];
      }

      // delay(100);

    }
  }

  Serial.print(F("You have "));
  Serial.print(correctAnsw);
  Serial.println(F(" correct answers"));
  ansLED();
  lcd.clear();
  lcd.print(F("You have "));
  lcd.print(correctAnsw,DEC);
  lcd.setCursor(0,1);
  lcd.print(F("correct "));
  if(correctAnsw ==1)
    lcd.print(F("answer"));
  else

      lcd.print(F("answers"));

  if (correctAnsw==4)
  {
    wins++;
    Serial.println(F("You won!"));
    playToneWin();
    lcd.clear();
    lcd.print(F("You won!"));
    lcd.setCursor(0,1);
    lcd.print(F("You won "));
    lcd.print(wins, DEC);
    if(wins<=1)
      lcd.print(" time");
    else
      lcd.print(" times");
    delay(2000);
    lcd.clear();
    lcd.print(F("Push the button."));
    lcd.setCursor(0,1);
    lcd.print(F(" For a new game."));
    Serial.print(wins);
    Serial.println(F(" time(s)."));
    correctAnsw=0;
    almostCorAnsw=0;
    Serial.println(F("Push the button for a new game."));
    buttonPush();
    startOver();
  }

  else
  {
    correctAnsw=0;
    if (turns>=4)
    {
      Serial.println(F("Sorry, you lost."));
      Serial.println(F("Push the button for a new game."));
      playToneLose();
      lcd.clear();
      lcd.print(F("Sorry, you lost."));
      lcd.setCursor(0,1);
      lcd.print(F("push button"));
      buttonPush();
      startOver();     
    }

    else

    {
      Serial.println(F("Sorry, try again"));
      turns++;
      Tlc.clear();
      Tlc.update();
    }
  }

  delay(3000);
}

void startOver()
{
  lcd.clear();
  lcd.print(F("New game"));
  Tlc.clear();
  Tlc.update();
  turns=0;
  Serial.println(F("New Game - in start Over loop"));
  secretCodeMaker();
}

void startupLED()
{
  Serial.println(F("Setup LED"));
  //LED1 0 = red 1= green 2 = blue
  delay(100);
  channel=2;
  Tlc.set(channel, 3095);
  Tlc.update();
  delay(250);
  channel=5;
  Tlc.set(channel, 3095);
  Tlc.update();
  delay(250);
  channel=8;
  Tlc.set(channel, 3095);
  Tlc.update();
  delay(250);
  channel=11;
  Tlc.set(channel, 3095);
  Tlc.update();
  delay(250);
  channel=19;
  Tlc.set(channel, 3095);
  Tlc.update();
  delay(250);
  channel=22;
  Tlc.set(channel, 3095);
  Tlc.update();
  delay(250);
  channel=24;
  Tlc.set(channel, 3095);
  Tlc.update();
  delay(250);
  channel=27;
  Tlc.set(channel, 3095);
  Tlc.update();
  delay(800);
  Tlc.clear();
  Tlc.update();
}

void buttonPush()

{
  while(digitalRead(Button) == HIGH);

  {
    delay(25);//do nothing until button pressed and goes LOW then start game over or go to next page in directions
  }

}
void buttonCheck()

{
  delay(100);
  while (digitalRead(Button) == 1)
  {
  }

  int reading = digitalRead(Button);
  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    buttonState = reading;
  }
  lastButtonState = reading;

}

void buttonPush3()

{
  Serial.println("In Button loop");
  Serial.println(x);
  for ( i=0; i <= 500; i++){

    if (digitalRead(Button) == LOW)
    {
      Serial.println(F("Directions"));
      directions();
    }
    delay(5);
    Serial.println(i);
  }
  Serial.println("over... ");
}

void blink()

{
  startOver();
}

void directions()
{
  i=900;
  lcd.clear();
  lcd.print(F("Mastermind"));
  lcd.setCursor(0,1);
  lcd.print(F("Directions"));
  delay(1000);
  lcd.clear();
  lcd.print(F("Push button"));
  lcd.setCursor(0,1);
  lcd.print(F("for next page"));
  buttonCheck();
  buttonCheck();
  lcd.clear();
  lcd.print(F("Guess the 4"));
  lcd.setCursor(0,1);
  lcd.print(F("secret colors"));
  buttonCheck();
  buttonCheck();
  lcd.clear();
  lcd.print(F("by placing one"));
  lcd.setCursor(0,1);
  lcd.print(F("card at a time"));
  buttonCheck();
  buttonCheck();
  lcd.clear();
  lcd.print(F("over the X"));
  lcd.setCursor(0,1);
  lcd.print(F("in the correct"));
  buttonCheck();
  buttonCheck();
  lcd.clear();
  lcd.print(F("order."));
  lcd.setCursor(0,1);
  lcd.print(F("It may be"));
  buttonCheck();
  buttonCheck();
  lcd.clear();
  lcd.print(F("helpful to write"));
  lcd.setCursor(0,1);
  lcd.print(F("down guesses."));
  buttonCheck();
  buttonCheck();
  lcd.clear();
  lcd.print(F("You have up to"));
  lcd.setCursor(0,1);
  lcd.print(F("5 turns to"));
  buttonCheck();
  buttonCheck();
  lcd.clear();
  lcd.print(F("guess the right"));
  lcd.setCursor(0,1);
  lcd.print(F("answer."));
  buttonCheck();
  buttonCheck();
  lcd.clear();
  lcd.print(F("Push button"));
  lcd.setCursor(0,1);
  lcd.print(F("To start"));
  buttonCheck();
  buttonCheck();
}

void ansLED()

{
  if (correctAnsw >=1)

  {
    Serial.println(F("light up ans"));
    channel=18;
    Tlc.set(channel, 3095);
    Tlc.update();
  }

  if (correctAnsw >=2)
  {
    Serial.println("light up ans");
    channel=21;
    Tlc.set(channel, 3095);
    Tlc.update();
  }

  if (correctAnsw >=3)
  {
    Serial.println("light up ans");
    channel=25;
    Tlc.set(channel, 3095);
    Tlc.update();
  }

  if (correctAnsw >=4)

  {
    Serial.println("light up ans");
    channel=28;
    Tlc.set(channel, 3095);
    Tlc.update();
  }
  delay(500);
}

void playToneWin()
{
  for (int thisNote =0; thisNote<10; thisNote++)
  {
    int noteDurations = 1000/noteDurationsCardWin[thisNote];
    tone(8, melodyWin[thisNote],noteDurations);
    int pauseBetweenNotes = noteDurations * 1.30;
    delay(pauseBetweenNotes);
    noTone(8);  // stop the tone playing:
  }
}

void playToneLose()
{
  for (int thisNote =0; thisNote<2; thisNote++)
  {
    int noteDurations = 1000/noteDurationsCardLose[thisNote];
    tone(8, melodyLose[thisNote],noteDurations);
    int pauseBetweenNotes = noteDurations * 1.30;
    delay(pauseBetweenNotes);
    noTone(8);
  }
}

void playToneStartup()
{
  Serial.println(F("Startup Tone"));
  for (int thisNote =0; thisNote<4; thisNote++)
  {
    int noteDurations = 1000/noteDurationsStartup[thisNote];
    tone(8, melodyStartup[thisNote],noteDurations);
    int pauseBetweenNotes = noteDurations * 1.30;
    delay(pauseBetweenNotes);
    noTone(8);
  }
}

void playToneTurnGreen()
{
  Serial.println(F("Turn Tone"));
  int thisNote =0; 
  int noteDurations = 1000/noteDurationsTurn[thisNote];
  tone(8, melodyTurnGreen[thisNote],noteDurations);
  int pauseBetweenNotes = noteDurations * 1.30;
  delay(pauseBetweenNotes);
  noTone(8);
}



void playToneTurnRed()

{

  Serial.println(F("Turn Tone"));

  int thisNote =0; 

  int noteDurations = 1000/noteDurationsTurn[thisNote];

  tone(8, melodyTurnRed[thisNote],noteDurations);

  int pauseBetweenNotes = noteDurations * 1.30;

  delay(pauseBetweenNotes);

  // stop the tone playing:

  noTone(8);

}

void playToneTurnBlue()

{

  Serial.println(F("Turn Tone"));

  int thisNote =0; 

  int noteDurations = 1000/noteDurationsTurn[thisNote];

  tone(8, melodyTurnBlue[thisNote],noteDurations);

  int pauseBetweenNotes = noteDurations * 1.30;

  delay(pauseBetweenNotes);

  // stop the tone playing:

  noTone(8);

}






