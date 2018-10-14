#include <SPI.h>
#include <MFRC522.h>
#include <string.h>

#define RST_PIN 9
#define SS_PIN 10
#define BUZZ_PIN 3

MFRC522 mfrc522(SS_PIN, RST_PIN);

void setup() {
  Serial.begin(9600);
  /*This is the data bus fir connecting peripherials to the board (MISO, MOSI, ...)*/
  SPI.begin(); 
  /*Init NFC reader*/
  mfrc522.PCD_Init();
  /*I wonder what this does if no serial is connected*/
  Serial.println("Successfuly connected to the card");
  pinMode(BUZZ_PIN, OUTPUT);
}

unsigned long route_colour = 0;
void loop() {
  unsigned long current_colour, finished;
  /* Look for new cards */
  if ( ! mfrc522.PICC_IsNewCardPresent()) {
    return;
  }
  /* Select one of the cards */
  if ( ! mfrc522.PICC_ReadCardSerial()) {
    return;
  }  

  /*Comment these 2 lines when you want to write, uncomment for usual reading*/
  if (!readColourInfo(mfrc522, &current_colour, &finished)) return;
  manageColourInfo(&route_colour, current_colour, finished);
  /*Comment these 2 lines when you want device normal functionality, uncomment when writing*/
  //askForData(&current_colour, &finished);
  //writeColourInfo(mfrc522, &current_colour, &finished);
  mfrc522.PICC_HaltA(); // Halt PICC
  mfrc522.PCD_StopCrypto1();  // Stop encryption on PCD
}

void buzz(int times){
  for(int i=0; i < times; i++){
    tone(BUZZ_PIN, 100);
    delay(1000);
    noTone(BUZZ_PIN);
    delay(100);
  }
}

void manageColourInfo(unsigned long *route_colour, unsigned long current_colour, unsigned long finished){
  if (! (*route_colour)){
    Serial.println("Assigning route colour");
    buzz(1);
    *route_colour = current_colour;
  }else if (*route_colour != current_colour){
    Serial.println("Wrong colour : Don't buzz");    
  }else if (finished == 1){
    Serial.println("Finished : buzz twice");
    buzz(2);
  }else{
    Serial.println("Right colour : buzz");
    buzz(1);
  }
}
/*Reads the proper pages in tag to fill the info*/
int readColourInfo(MFRC522 mfrc522, unsigned long* current_colour, unsigned long *finished){
  MFRC522::StatusCode status;
  byte buffer[18], size = sizeof(buffer);
  status = mfrc522.MIFARE_Read(6, buffer, &size);
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("Reading failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return 0;
  }  
  /*Now weve read 4 pages, want the first 2 of them*/
  *current_colour = *(unsigned long *)buffer;
  *finished = *(unsigned long *)(buffer + 4);
  return 1;
}

/*Prints uid and picc*/
void printBasicInfo(MFRC522 mfrc522){
  Serial.print(F("Card UID:"));
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
    Serial.print(mfrc522.uid.uidByte[i], HEX);
  }
  Serial.print(F(" PICC type: "));   // Dump PICC type
  MFRC522::PICC_Type piccType = mfrc522.PICC_GetType(mfrc522.uid.sak);
  Serial.println(mfrc522.PICC_GetTypeName(piccType));
  return;
}

void askForData(unsigned long *colour, unsigned long *finished){
  String s;
  char colour_string[16];
  /*I'm gonna trust the user. Not many error control*/
  /*Ask user for a colour*/
  //Serial.println("BEFORE READING A TAG\nWrite a colour identifier number 0 < n <= 4294967296 and press intro");
  //s = Serial.readStringUntil('\n');
  s = "512";
  s.toCharArray(colour_string, 16);   
  sscanf(colour_string, "%lu", colour);
  /*Ask for termination*/
  //Serial.println("Do you want this to be the last rock of your route? y/n and press intro");
  //s = Serial.readString();
  s = "n";
  if (s == "y") *finished = 1;
  else if (s == "n") *finished = 0;
}

/*Writes a string with the colour on the tag*/
void writeColourInfo(MFRC522 mfrc522, unsigned long *colour, unsigned long *finished){
  /*Writing on the second block*/
  int page, status;  
  /*Print before */
  mfrc522.PICC_DumpMifareUltralightToSerial();  
  /* Write block */
  page = 6;
  status = mfrc522.MIFARE_Ultralight_Write(page, (byte*)colour, 4);
  page = 7;
  status = mfrc522.MIFARE_Ultralight_Write(page, (byte*)finished, 4);
  /*Print after */
  mfrc522.PICC_DumpMifareUltralightToSerial();  
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("MIFARE_Write() failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  } else Serial.println(F("MIFARE_Write() success: "));
  
}
