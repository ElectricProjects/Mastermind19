
/*************************************************************************
 * 
 * Mastermind vs 20
 * example of checking RFID cards
 * 
/**************************************************************************/

#include <Wire.h>
#include <Adafruit_NFCShield_I2C.h>
#define IRQ   (2)
#define RESET (4)  // Not connected by default on the NFC Shield

// change here to match your cards
uint8_t card1[] = {
  0x1B, 0xCB, 0x78, 0x95    }; // red
uint8_t card2[] = {
  0xCA, 0x19, 0xEE, 0x65    }; //green
uint8_t card3[] = {
  0xEA, 0x38, 0x82, 0xB7    }; // blue
int card=0;

Adafruit_NFCShield_I2C nfc(IRQ, RESET);


void setup(void) {
  Serial.begin(9600);
  nfc.begin();
  uint32_t versiondata = nfc.getFirmwareVersion();
  if (! versiondata) {
    Serial.print("Didn't find PN53x board");
    while (1); // halt
  }
  nfc.SAMConfig(); // configure board to read RFID tags
}

uint8_t uid[] = {
  0, 0, 0, 0, 0, 0, 0 };  // Buffer to store the returned UID

void loop(void) {

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
      uint8_t keya[6] = { 
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF            };

      // Start with block 4 (the first block of sector 1)
      success = nfc.mifareclassic_AuthenticateBlock(uid, uidLength, 4, 0, keya);
      if (success)
      {
        uint8_t data[16];
        success = nfc.mifareclassic_ReadDataBlock(4, data);

        if (success)
        {
          // Data seems to have been read
          arrayCompare(); // here we verify card presented
          // Wait a bit before reading the card again
          delay(100);
        }

        else

        {
          Serial.println(F("Unable to read card. again."));

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

  if(memcmp(card1, uid, 4) == 0)
  {
    Serial.println(F(" is red."));
    card=1;
  }

  if(memcmp(card2, uid, 4) == 0)
  {
    Serial.println(F(" is green."));
    card=2;
  }

  if(memcmp(card3, uid, 4) == 0)
  {
    Serial.println(F(" is blue."));
    card=3;
  }
}











