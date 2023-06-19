//www.elegoo.com
//2016.12.09

/*
 * --------------------------------------------------------------------------------------------------------------------
 * Example to change UID of changeable MIFARE card.
 * --------------------------------------------------------------------------------------------------------------------
 * This is a MFRC522 library example; for further details and other examples see: https://github.com/miguelbalboa/rfid
 * 
 * This sample shows how to set the UID on a UID changeable MIFARE card.
 * NOTE: for more informations read the README.rst
 * 
 * @author Tom Clement
 * @license Released into the public domain.
 *
 * Typical pin layout used:
 * -----------------------------------------------------------------------------------------
 *             MFRC522      Arduino       Arduino   Arduino    Arduino          Arduino
 *             Reader/PCD   Uno           Mega      Nano v3    Leonardo/Micro   Pro Micro
 * Signal      Pin          Pin           Pin       Pin        Pin              Pin
 * -----------------------------------------------------------------------------------------
 * RST/Reset   RST          9             5         D9         RESET/ICSP-5     RST
 * SPI SS      SDA(SS)      10            53        D10        10               10
 * SPI MOSI    MOSI         11 / ICSP-4   51        D11        ICSP-4           16
 * SPI MISO    MISO         12 / ICSP-1   50        D12        ICSP-1           14
 * SPI SCK     SCK          13 / ICSP-3   52        D13        ICSP-3           15
 */

#include <SPI.h>
#include <MFRC522.h>

// Define Pins FOR RFID
#define RST_PIN   26    // Configurable, see typical pin layout above
const int NUMBER_OF_SS_PINS = 2;//4;
int SS_PINS[] = {42, 43};//, 44, 45};

//For RGB LED array
#define dataPin 12
#define enablePin 11
#define latchPin 10
#define clockPin 9

#define number_of_74hc595s 5
#define numOfRegisterPins number_of_74hc595s * 8
#define numRGBLeds 12

boolean registers[numOfRegisterPins];
int redPin[]   = {0, 3, 6, 9, 12, 15, 18, 21, 24, 27, 30, 33 };
int greenPin[] = {1, 4, 7, 10, 13, 16, 19, 22, 25, 28, 31, 34 };
int bluePin[]  = {2, 5, 8, 11, 14, 17, 20, 23, 26, 29, 32, 35 };

// Define Pins for potentiometer
#define POT1_PIN   A1
#define POT2_PIN   A2
#define POT3_PIN   A3

// Define Pins for mode (leds)
// green
#define MODE_PLAY_LED 22 
// red
#define MODE_TEAM1_LED 23
// blue
#define MODE_TEAM2_LED 24

// Define Pins for button
#define BUTTON     25

MFRC522 mfrc522Array[NUMBER_OF_SS_PINS];

#define FOB_UUID {0x03, 0xC4, 0xF1, 0xB6}
#define CARD_UUID {0x73, 0x31, 0x07, 0x0F}

//VALUES FOR DISPLAY
int rfidDetectedArray[NUMBER_OF_SS_PINS];
#define RFID_NOTHING_DETECTED  0
#define RFID_TEAM1_DETECTED    1 //FOB
#define RFID_TEAM2_DETECTED    2 //CARD

//Mode
#define MODE_PLAY               0
#define MODE_TEAM_1             1
#define MODE_TEAM_2             2
int ledDisplayMode = MODE_PLAY;
bool buttonPressStarted = false;
//For the dumb toggling on new card
int newCardNumberArray[NUMBER_OF_SS_PINS];
#define RGB_FULL_COLOR LOW
#define RGB_MIN_COLOR  HIGH

int team1Green = RGB_MIN_COLOR;
int team1Red = RGB_MIN_COLOR;
int team1Blue = RGB_FULL_COLOR;
int team2Green = RGB_MIN_COLOR;
int team2Red = RGB_FULL_COLOR;
int team2Blue = RGB_MIN_COLOR;

void setup() {
  Serial.begin(9600);  // Initialize serial communications with the PC
  Serial.print("In the setup Function");

  //SETUP BUTTON MODE PINS
  pinMode(BUTTON, INPUT_PULLUP);

  //SETUP LED MODE PINS
  // sets the pins as output
  pinMode(MODE_PLAY_LED, OUTPUT);   
  pinMode(MODE_TEAM1_LED, OUTPUT);   
  pinMode(MODE_TEAM2_LED, OUTPUT); 

  //SETUP FOR RFID
  while (!Serial);     // Do nothing if no serial port is opened (added for Arduinos based on ATMEGA32U4)
  SPI.begin();         // Init SPI bus

  for (int i = 0; i < NUMBER_OF_SS_PINS; i++) {
    mfrc522Array[i] = MFRC522(SS_PINS[i], RST_PIN);
    mfrc522Array[i].PCD_Init();
    mfrc522Array[i].PCD_DumpVersionToSerial();

    newCardNumberArray[i] = 0;
    rfidDetectedArray[i] = RFID_NOTHING_DETECTED;
  }

  //setup for RGB LED array
  pinMode(dataPin,  OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  digitalWrite(enablePin, LOW);
  for (int i = numOfRegisterPins - 1; i >=  0; i--) {
    registers[i] = LOW;   
  }
  ledOff(0); 

  displayPlayLeds();
  brigtnessNodelay();
}



void loop() {

  handleButtonPress();
  displayModeLed();
  detectCardArray();

  if (ledDisplayMode == MODE_PLAY) {
    displayPlayLeds();
  } else if (ledDisplayMode == MODE_TEAM_1) {
    handleColorChange();
    displayTeamSelectLed(MODE_TEAM_1);
  } else if (ledDisplayMode == MODE_TEAM_2) {
    handleColorChange();
    displayTeamSelectLed(MODE_TEAM_2);
  }

  //This is needed instead of delay(50) because we are messing with the timer to get the leds to work
  _delay_ms(50);  

}

void handleColorChange() {
  //Value from 0-1023 not 0-255. Convert to 255 by divide by 4
  //red
  int pot1Val = analogRead(POT1_PIN) / 4;
  //green
  int pot2Val = analogRead(POT2_PIN) / 4;
  //blue
  int pot3Val = analogRead(POT3_PIN) / 4;
  if (ledDisplayMode == MODE_TEAM_1) {
    if (pot1Val >= 128) {
      team1Red = HIGH;
    } else {
      team1Red = LOW;
    }
    if (pot2Val >= 128) {
      team1Green = HIGH;
    } else {
      team1Green = LOW;
    }
    if (pot3Val >= 128) {
      team1Blue = HIGH;
    } else {
      team1Blue = LOW;
    }
  } else if (ledDisplayMode == MODE_TEAM_2) {
    if (pot1Val >= 128) {
      team2Red = HIGH;
    } else {
      team2Red = LOW;
    }
    if (pot2Val >= 128) {
      team2Green = HIGH;
    } else {
      team2Green = LOW;
    }
    if (pot3Val >= 128) {
      team2Blue = HIGH;
    } else {
      team2Blue = LOW;
    }
  }
}

void displayModeLed() {
  if (ledDisplayMode == MODE_PLAY) {
    digitalWrite(MODE_PLAY_LED, HIGH);
    digitalWrite(MODE_TEAM1_LED, LOW);
    digitalWrite(MODE_TEAM2_LED, LOW);
  } else if (ledDisplayMode == MODE_TEAM_1) {
    digitalWrite(MODE_PLAY_LED, LOW);
    digitalWrite(MODE_TEAM1_LED, HIGH);
    digitalWrite(MODE_TEAM2_LED, LOW);
  } else if (ledDisplayMode == MODE_TEAM_2) {
    digitalWrite(MODE_PLAY_LED, LOW);
    digitalWrite(MODE_TEAM1_LED, LOW);
    digitalWrite(MODE_TEAM2_LED, HIGH);
  }
}

void handleButtonPress() {
  if (digitalRead(BUTTON) == HIGH && buttonPressStarted) {
    Serial.print("\nHandle Button Down");
    buttonPressStarted = false;
    if (ledDisplayMode == MODE_PLAY) {
      ledDisplayMode = MODE_TEAM_1;
    } else if (ledDisplayMode == MODE_TEAM_1) {      
      ledDisplayMode = MODE_TEAM_2;
    } else if (ledDisplayMode == MODE_TEAM_2) {
      ledDisplayMode = MODE_PLAY;
    }
  } else if (digitalRead(BUTTON) == LOW) {
    
    Serial.print("\nHandle Button Up");
    buttonPressStarted = true;
  }
}

void displayTeamSelectLed(int mode) {
  int redValue = RGB_MIN_COLOR;
  int greenValue = RGB_MIN_COLOR;
  int blueValue = RGB_MIN_COLOR;
  if (mode == MODE_TEAM_1) {
    redValue = team1Red;
    greenValue = team1Green;
    blueValue = team1Blue;
  } else if (mode == MODE_TEAM_2) {
    redValue = team2Red;
    greenValue = team2Green;
    blueValue = team2Blue;
  }
  
  for (int i = 0; i < NUMBER_OF_SS_PINS; i++) {
    if (rfidDetectedArray[i] == RFID_TEAM1_DETECTED && mode == MODE_TEAM_1 || 
        rfidDetectedArray[i] == RFID_TEAM1_DETECTED && mode == MODE_TEAM_2) {
      writeLed(i, redValue, greenValue, blueValue);
    } 
  }
}

void displayPlayLeds() {
  for (int i = 0; i < NUMBER_OF_SS_PINS; i++) {
    int redValue = RGB_MIN_COLOR;
    int greenValue = RGB_MIN_COLOR;
    int blueValue = RGB_MIN_COLOR;
    if (rfidDetectedArray[i] == RFID_TEAM1_DETECTED) {
      redValue = team1Red;
      greenValue = team1Green;
      blueValue = team1Blue;
    } else if (rfidDetectedArray[i] == RFID_TEAM2_DETECTED) {
      redValue = team2Red;
      greenValue = team2Green;
      blueValue = team2Blue;
    }
    
    writeLed(i, redValue, greenValue, blueValue);
  }
}

void detectCardArray() {
  for (int i = 0; i < NUMBER_OF_SS_PINS; i++) {
  
    bool newCard = mfrc522Array[i].PICC_IsNewCardPresent();
    bool readCard = mfrc522Array[i].PICC_ReadCardSerial();

    // We can only trust every other "newCard", because calling it a second time after
    // a true, will ALWAYS return false.
    if (newCard && (newCardNumberArray[i] == 0 || newCardNumberArray[i] == 2)) {
      newCardNumberArray[i] = 1;
    }
    if (!newCard) {
      if (newCardNumberArray[i] == 1) {
        newCardNumberArray[i] = 2;
      } else if (newCardNumberArray[i] == 2) {
        newCardNumberArray[i] = 0;
      }
    }

    // Look for new cards, and select one if present
    if (!newCard || !readCard) {
      if (newCardNumberArray[i] == 0) {
        rfidDetectedArray[i] = RFID_NOTHING_DETECTED;
      }
    } else {
      Serial.print("\n\n");
      Serial.print("\nCard UID:");
      
      // Now a card is selected. The UID and SAK is in mfrc522.uid.
      bool isTeam1 = true;
      byte fobUid[] = FOB_UUID;
      bool isTeam2 = true;
      byte cardUid[] = CARD_UUID;
      for (byte j = 0; j < mfrc522Array[i].uid.size; j++) {
        Serial.print(mfrc522Array[i].uid.uidByte[j] < 0x10 ? " 0" : " ");
        Serial.print(mfrc522Array[i].uid.uidByte[j], HEX);
        if (mfrc522Array[i].uid.uidByte[j] != fobUid[j]) {
          isTeam1 = false;
        }
        if (mfrc522Array[i].uid.uidByte[j] != cardUid[j]) {
          isTeam2 = false;
        }
      } 
      Serial.print("\n");


      if (isTeam1) {
        Serial.print("We are team1!!\n");
        rfidDetectedArray[i] = RFID_TEAM1_DETECTED;
      } else if (isTeam2) {
        Serial.print("We are the Card!!\n");
        rfidDetectedArray[i] = RFID_TEAM2_DETECTED;
      } else {
        Serial.print("No clue what we are!!\n");
        rfidDetectedArray[i] = RFID_NOTHING_DETECTED;
      }


      // I believe this will make it stop scaning
      // We don't want to stop scanning, because we want to try each time we check to see if something is there
      // mfrc522Array[i].PICC_HaltA();
    }
  }
}

void writeLed(int pos, int red, int green, int blue) {
  registersWrite(redPin[pos], red);
  registersWrite(greenPin[pos], green);
  registersWrite(bluePin[pos], blue);
}

void registersWrite(int index, int value) {
  digitalWrite(enablePin, LOW);
  digitalWrite(latchPin, LOW);
  for (int i = numOfRegisterPins - 1; i >=  0; i--) {
    digitalWrite(clockPin, LOW);
    int val = registers[i];
    digitalWrite(dataPin, val);
    digitalWrite(clockPin, HIGH);
  }
  digitalWrite(latchPin, HIGH);
  registers[index] = value;
}

void ledOff(int clearSpeed) {
  for (int i = 0; i <= 35; i++) {
    registersWrite(i, HIGH);
    delay(clearSpeed);
  }
}

void brigtnessNodelay() {
  analogWrite(enablePin, 0);
  // for (int i = 255; i >= 0; i--) {
  //   analogWrite(enablePin, 0);
  // }
}
