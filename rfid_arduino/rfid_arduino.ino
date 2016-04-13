#include <SoftwareSerial.h>
#include <SPI.h>
#include <MFRC522.h>
#include <EEPROM.h>
#define SS 10
#define RST 9
#define relPin A0 
#define R 3
#define G 5
#define B 6

MFRC522 rfid(SS, RST);


boolean match = false;

int rootStart = 1018;
int rootCheckAdr = 1022;

int tagNumAdr = 1023;


byte readTag[4];
byte root[4];



void setup() {
  Serial.begin(9600);
  SPI.begin();
  SPI.setClockDivider(SPI_CLOCK_DIV128); 
  Serial.println("Initializing...");
  //setLed(R, true);
  Serial.print("There are ");
  Serial.print(EEPROM.read(tagNumAdr));
  Serial.println(" tags in EEPROM.");
  pinMode(relPin, OUTPUT);
  digitalWrite(relPin, HIGH);
  rfid.PCD_Init();
  rfid.PCD_SetAntennaGain(rfid.RxGain_max);
  checkRoot();  
  loadRoot();
  Serial.println("Init done.");
  //setLed(R, false);
}

void loop() {
  if (isThereTag()) {
    Serial.println("Tag read. You have 1 second to remove it.");
    //setLed(B, true);
    delay(1000);
    //setLed(B, false);
    getUid();
    isRoot();
    if (isInMemory()) {
      Serial.println("Welcome home!");
      openDoor();
    } 
    else {
      Serial.println("Who are you?");
      for (int i = 0;i<rfid.uid.size;i++) {
        Serial.print(rfid.uid.uidByte[i]);
        Serial.print('-');
      }
      Serial.println();
      //setLed(R, true);
      delay(300);
      //setLed(R, false);
    }
  }  
}


boolean isThereTag() {
  if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {
    return true;
  }
  else {
    return false;
  }
}

void getUid() {
  for (int i = 0;i<4;i++) {
    readTag[i] = rfid.uid.uidByte[i];
  } 
}

boolean checkMatch(byte test[]) {
  for (int i = 0;i<4;i++) {
    if (readTag[i] != test[i]) {
      return false;
    }
  }
  return true; 
}

void openDoor() {
  digitalWrite(relPin, LOW);
  //setLed(G, true);
  delay(300);
  digitalWrite(relPin, HIGH);
  delay(300);
  //setLed(G, false);
  match = false;
}

void checkRoot() {
  Serial.println("Checking for root tag...");
  if (EEPROM.read(rootCheckAdr) != 1) {
    Serial.println("Root tag not defined! Waiting for a tag...");
    //setLed(R, true);
    //setLed(G, true);
    do {
      if (isThereTag()) {
        openDoor();
        Serial.println("Tag found.");
        delay(1000);
        for (int i = 0;i<4;i++) {
          EEPROM.write(rootStart+i, rfid.uid.uidByte[i]);
        }
        EEPROM.write(rootCheckAdr, 1);
        //setLed(R, false);
        //setLed(G, false);
      }
    } 
    while (!isThereTag());
  } 
  else {
    Serial.println("Root already defined.");
  }
}

void loadRoot() {
  Serial.println("Loading root tag into RAM...");
  Serial.println();
  for (int i = 0;i<4;i++) {
    root[i] = EEPROM.read(rootStart + i);
    Serial.print(root[i]);
    Serial.print('-');
  }
  Serial.println("Done.");
}


void isRoot() {
  if (checkMatch(root)) {
    Serial.println("Hello, root! Place a Tag.");
    //setLed(R, true);
    delay(1000);
    do {
      if (isThereTag()) {
        getUid();
        if (!isInMemory()) {        
        Serial.println("Welcome tag! Adding you.");
        addTag(readTag, EEPROM.read(tagNumAdr)*4);
        EEPROM.write(tagNumAdr, EEPROM.read(tagNumAdr)+1);
        //setLed(R, false);
        return;
        } else {
         Serial.println("Oh Tag! Goodbye! Wiping you.");
         int i = getReadAdr();
         if (i!=-1) {
          removeTag(i);
         }
        }
      }
    } 
    while (!isThereTag());
  }
} 

void addTag(byte tag[], int adr) {
  Serial.println("Adding Tag...");
  for (int i = 0;i<4;i++) {
    EEPROM.write(adr+i, tag[i]);
    Serial.print(EEPROM.read(adr+i));
    Serial.print('-');
  }
  Serial.println();
  Serial.println("Done.");
}

boolean isInMemory() {
  for (int i = 0;i<EEPROM.read(tagNumAdr);i++) {
    byte buff[4];
    for (int j = 0;j<4;j++) {
      buff[j] = EEPROM.read(i*4+j);
    }
    if (checkMatch(buff)) {
      return true;
    }
  }
  return false;
}

int getReadAdr() {
   for (int i = 0;i<EEPROM.read(tagNumAdr);i++) {
    byte buff[4];
    for (int j = 0;j<4;j++) {
      buff[j] = EEPROM.read(i*4+j);
    }
    if (checkMatch(buff)) {
      return i;
    }
  }
  return -1;
}
/*void setLed(int pin, boolean on) {
 analogWrite(pin, on?1023:0);
 }*/

void removeTag(int i) {
  for (int j=i+4;j<EEPROM.read(tagNumAdr*4);j++) {
    EEPROM.write(j-4, EEPROM.read(j));
  }
  EEPROM.write(tagNumAdr, EEPROM.read(tagNumAdr)-1);
}






