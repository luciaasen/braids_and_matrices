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
  Serial.println("Successfuly connected to the reader");
  char route_colour[16] = "";
  char read_colour[16] = "";
  long last_id;
}

void loop() {
  char string[16];
  int finished;
  
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
/**
 * In the future, when we see this works, instead of printing basic info, check the tag id
 * and compare it with the last read id. If they are the same, don't buzz!
 */
  /*Read and parse from the tag*/
  if (readColourInfo(mfrc522, string, 16)){
    colour_parse(string, read_colour, finished);
  }
  /*If it is the first read tag, mark it as the route colour*/
  if (strcmp(route_colour, "") == 0){
    strcpy(route_colour, read_colour);
  }
  
  Serial.print("Read string is %s, parsed colour is %s and finished variable is %d\n", string, colour, finished);
  
  mfrc522.PICC_HaltA(); // Halt PICC
  mfrc522.PCD_StopCrypto1();  // Stop encryption on PCD

}

void colour_parse(char* string, char* colour, int finished){
  sscanf(string, "%s#%d", colour, &finished);
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

int readColourInfo(MFRC522, char *string, int len){
  /*Reading from the second block*/
  int block = 1;
  /*This seems to be the default key*/
  MFRC522::MIFARE_Key key;
  for (byte i = 0; i < 6; i++) key.keyByte[i] = 0xFF;

  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, 4, &key, &(mfrc522.uid)); //line 834 of MFRC522.cpp file
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("Authentication failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return 0;
  }

  status = mfrc522.MIFARE_Read(block, (byte *)string, &len);
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("Reading failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return 0;
  }  
  return 1;
}


/*Writes a string with the colour on the tag*/
void writeColourInfo(MFRC522 mfrc522, char *colour, int size){
  /*Writing on the second block*/
  int block = 1;
  /*This seems to be the default key*/
  MFRC522::MIFARE_Key key;
  for (byte i = 0; i < 6; i++) key.keyByte[i] = 0xFF;
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
