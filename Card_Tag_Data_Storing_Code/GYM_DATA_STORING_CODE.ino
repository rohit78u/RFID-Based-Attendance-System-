/* ------------------------------------------------------------------------
 * Created by: Electrical Mandir (Modified by Rohit)
 * 
 * Purpose: Store Gym Member Details in RFID Card
 * Format: Enroll ID | First Name | Last Name | Phone No | Batch
 * 
 * Board: NodeMCU (ESP8266)
 * RFID: MFRC522
 * ------------------------------------------------------------------------
*/

#include <SPI.h>
#include <MFRC522.h>

//--------------------------------------------------
// RFID Pins
const uint8_t RST_PIN = D3;  // GPIO0
const uint8_t SS_PIN  = D4;  // GPIO2
//--------------------------------------------------
MFRC522 mfrc522(SS_PIN, RST_PIN);
MFRC522::MIFARE_Key key;        

//--------------------------------------------------
int blockNum = 4;                 // Start writing from block 4
byte bufferLen = 18;
byte readBlockData[18];
MFRC522::StatusCode status;
//--------------------------------------------------

void setup() 
{
  Serial.begin(9600);
  SPI.begin();
  mfrc522.PCD_Init();

  Serial.println("Scan a MIFARE 1K Tag to write GYM Member Data...");
}

void loop()
{
  // Prepare default key
  for (byte i = 0; i < 6; i++) key.keyByte[i] = 0xFF;

  // Wait for new card
  if (!mfrc522.PICC_IsNewCardPresent()) return;
  if (!mfrc522.PICC_ReadCardSerial()) return;

  Serial.println("\n**Card Detected**");
  Serial.print(F("Card UID: "));
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
    Serial.print(mfrc522.uid.uidByte[i], HEX);
  }
  Serial.println();

  byte buffer[18];
  byte len;
  Serial.setTimeout(20000L);  // 20 seconds timeout for input

  // ------------------------------
  // Enroll ID
  Serial.println(F("---------------------------------------"));
  Serial.println(F("Enter Enroll ID (end with #):"));
  len = Serial.readBytesUntil('#', (char *)buffer, 16);
  for (byte i = len; i < 16; i++) buffer[i] = ' ';
  blockNum = 4;
  WriteDataToBlock(blockNum, buffer);
  ReadDataFromBlock(blockNum, readBlockData);
  dumpSerial(blockNum, readBlockData);

  // ------------------------------
  // First Name
  Serial.println(F("---------------------------------------"));
  Serial.println(F("Enter First Name (end with #):"));
  len = Serial.readBytesUntil('#', (char *)buffer, 16);
  for (byte i = len; i < 16; i++) buffer[i] = ' ';
  blockNum = 5;
  WriteDataToBlock(blockNum, buffer);
  ReadDataFromBlock(blockNum, readBlockData);
  dumpSerial(blockNum, readBlockData);

  // ------------------------------
  // Last Name
  Serial.println(F("---------------------------------------"));
  Serial.println(F("Enter Last Name (end with #):"));
  len = Serial.readBytesUntil('#', (char *)buffer, 16);
  for (byte i = len; i < 16; i++) buffer[i] = ' ';
  blockNum = 6;
  WriteDataToBlock(blockNum, buffer);
  ReadDataFromBlock(blockNum, readBlockData);
  dumpSerial(blockNum, readBlockData);

  // ------------------------------
  // Phone Number
  Serial.println(F("---------------------------------------"));
  Serial.println(F("Enter Phone Number (end with #):"));
  len = Serial.readBytesUntil('#', (char *)buffer, 16);
  for (byte i = len; i < 16; i++) buffer[i] = ' ';
  blockNum = 8;
  WriteDataToBlock(blockNum, buffer);
  ReadDataFromBlock(blockNum, readBlockData);
  dumpSerial(blockNum, readBlockData);

  // ------------------------------
  // Batch (Gold, Silver, Bronze)
  Serial.println(F("---------------------------------------"));
  Serial.println(F("Enter Batch (Gold/Silver/Bronze) (end with #):"));
  len = Serial.readBytesUntil('#', (char *)buffer, 16);
  for (byte i = len; i < 16; i++) buffer[i] = ' ';
  blockNum = 9;
  WriteDataToBlock(blockNum, buffer);
  ReadDataFromBlock(blockNum, readBlockData);
  dumpSerial(blockNum, readBlockData);

  Serial.println(F("✅ Data successfully written to RFID tag."));
  Serial.println(F("Remove card and scan next one if needed.\n"));
}

/****************************************************************************************************
 * WriteDataToBlock() function
 ****************************************************************************************************/
void WriteDataToBlock(int blockNum, byte blockData[]) 
{
  // Authenticate block for write access
  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, blockNum, &key, &(mfrc522.uid));
  if (status != MFRC522::STATUS_OK) {
    Serial.print("Authentication failed for Write: ");
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }

  // Write data to the block
  status = mfrc522.MIFARE_Write(blockNum, blockData, 16);
  if (status != MFRC522::STATUS_OK) {
    Serial.print("Writing to Block failed: ");
    Serial.println(mfrc522.GetStatusCodeName(status));
  }
}

/****************************************************************************************************
 * ReadDataFromBlock() function
 ****************************************************************************************************/
void ReadDataFromBlock(int blockNum, byte readBlockData[]) 
{
  for (byte i = 0; i < 6; i++) key.keyByte[i] = 0xFF;
  
  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, blockNum, &key, &(mfrc522.uid));
  if (status != MFRC522::STATUS_OK) {
    Serial.print("Authentication failed for Read: ");
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }

  status = mfrc522.MIFARE_Read(blockNum, readBlockData, &bufferLen);
  if (status != MFRC522::STATUS_OK) {
    Serial.print("Reading failed: ");
    Serial.println(mfrc522.GetStatusCodeName(status));
  }
}

/****************************************************************************************************
 * dumpSerial() function
 ****************************************************************************************************/
void dumpSerial(int blockNum, byte blockData[]) 
{
  Serial.print("\nData saved on block ");
  Serial.print(blockNum);
  Serial.print(": ");
  for (int j = 0; j < 16; j++) Serial.write(readBlockData[j]);
  Serial.print("\n");

  // Clear buffer
  for (int i = 0; i < sizeof(readBlockData); ++i)
    readBlockData[i] = (char)0;
}
