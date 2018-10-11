#include <SPI.h>
#include <MFRC522.h>

#define RST_PIN 9
#define SS_PIN 10

MFRC522 mfrc522(SS_PIN, RST_PIN);

void setup() {
  Serial.begin(9600);
  /*This is the data bus fir connecting peripherials to the board (MISO, MOSI, ...)*/
  SPI.begin(); 
  /*Init NFC reader*/
  mfrc522.PCD_Init(); 
  /*I wonder what this does if no serial is connected*/
  Serial.println("Successfuly connected to the card");
}

void loop() {
  /*I wrote 16 hoping our cards are 16 byte/block and our colours are not > 16-1-1-1 characters*/
  char colour[16] = "colour#0";
  
   /* Look for new cards */
  if ( ! mfrc522.PICC_IsNewCardPresent()) {
    return;
  }
  /* Select one of the cards */
  if ( ! mfrc522.PICC_ReadCardSerial()) {
    return;
  }
  /*This is not necessary, just will give come info for testing the tags*/
  printBasicInfo(mfrc522);
  /*Write on the tag. This should be done by asking the user? :)*/
  writeColourInfo(mfrc522, colour, 16);
  
  mfrc522.PICC_HaltA(); // Halt PICC
  mfrc522.PCD_StopCrypto1();  // Stop encryption on PCD

}

/*Prints uid and picc
  Don't know if MFRC522 is a valid type, but we'll see*/
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
￼
/*Writes a string with the colour on the tag*/
void writeColourInfo(MFRC522 mfrc522, char *colour, int size){
  /*This seems to be the default key*/
  MFRC522::MIFARE_Key key;
  for (byte i = 0; i < 6; i++) key.keyByte[i] = 0xFF;
  /*Writing on the second block*/
  block = 1;
  /*Authentication using the key*/
  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, block, &key, &(mfrc522.uid));
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("PCD_Authenticate() failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }
  else Serial.println(F("PCD_Authenticate() success: "));
  /* Write block */
  status = mfrc522.MIFARE_Write(block, (byte*)colour, size);
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("MIFARE_Write() failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }
  else Serial.println(F("MIFARE_Write() success: "));
  
}
