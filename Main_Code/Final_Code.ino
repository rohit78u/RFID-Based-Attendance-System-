#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <SPI.h>
#include <MFRC522.h>
#include <HTTPSRedirect.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <WiFiUdp.h>
#include <NTPClient.h>

// ✅ Updated for 20x4 LCD
LiquidCrystal_I2C lcd(0x27, 20, 4);

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 19800, 60000); // IST (UTC +5:30)

const char* GScriptId = "AKfycbxf9DL4rwMymamATl1dV39O-mHlIGkdd8w4PM6SZup9SzgEsHQvf5rRgetLFvPveKr9YQ";
const char* ssid = "Batman_PC";
const char* password = "12345678";

String payload_base = "{\"command\": \"insert_row\", \"sheet_name\": \"Sheet1\", \"values\": ";
String payload = "";
const char* host = "script.google.com";
const int httpsPort = 443;
HTTPSRedirect* client = nullptr;

String member_id;
int blocks[] = {4, 5, 6, 8, 9};
#define total_blocks (sizeof(blocks) / sizeof(blocks[0]))
#define RST_PIN D3
#define SS_PIN D4
#define BUZZER_PIN D8

MFRC522 mfrc522(SS_PIN, RST_PIN);
MFRC522::MIFARE_Key key;
MFRC522::StatusCode status;
byte readBlockData[18];

//---------------- Buzzer Functions ----------------
void beepOnce(int duration = 100) {
  digitalWrite(BUZZER_PIN, HIGH);
  delay(duration);
  digitalWrite(BUZZER_PIN, LOW);
}

void beepTwice() {
  for (int i = 0; i < 2; i++) {
    digitalWrite(BUZZER_PIN, HIGH);
    delay(100);
    digitalWrite(BUZZER_PIN, LOW);
    delay(100);
  }
}

void beepAlert() {
  for (int i = 0; i < 3; i++) {
    digitalWrite(BUZZER_PIN, HIGH);
    delay(100);
    digitalWrite(BUZZER_PIN, LOW);
    delay(100);
  }
}

//---------------- Time Function ----------------
String getFormattedTime() {
  timeClient.update();
  return timeClient.getFormattedTime();
}

//---------------- Read Data from Block ----------------
String ReadDataFromBlockClean(int blockNum) {
  byte blockBuf[18];
  byte len = 18;
  for (byte i = 0; i < 6; i++) key.keyByte[i] = 0xFF;

  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, blockNum, &key, &(mfrc522.uid));
  if (status != MFRC522::STATUS_OK) {
    Serial.print("Auth fail block "); Serial.println(blockNum);
    beepAlert();
    return String("");
  }

  status = mfrc522.MIFARE_Read(blockNum, blockBuf, &len);
  if (status != MFRC522::STATUS_OK) {
    Serial.print("Read fail block "); Serial.println(blockNum);
    beepAlert();
    return String("");
  }

  String out = "";
  for (int i = 0; i < 16; i++) {
    if (blockBuf[i] == 0x00) break;
    out += (char)blockBuf[i];
  }

  out.trim();
  out.replace("#", "");
  Serial.print("Block "); Serial.print(blockNum); Serial.print(" -> "); Serial.println(out);
  return out;
}

//---------------- Setup ----------------
void setup() {
  Serial.begin(9600);
  delay(10);
  SPI.begin();

  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);

  lcd.init();
  lcd.backlight();
  lcd.clear();

  lcd.setCursor(2, 0);
  lcd.print("GYM ATTENDANCE");
  lcd.setCursor(1, 1);
  lcd.print("Connecting WiFi...");
  Serial.println("LCD: GYM ATTENDANCE - Connecting WiFi...");

  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi: ");
  Serial.print(ssid);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi! IP: ");
  Serial.println(WiFi.localIP());
  beepOnce();

  delay(2000);
  timeClient.begin();
  timeClient.update();

  client = new HTTPSRedirect(httpsPort);
  client->setInsecure();
  client->setPrintResponseBody(true);
  client->setContentTypeHeader("application/json");

  lcd.clear();
  lcd.setCursor(2, 0);
  lcd.print("WiFi Connected");
  lcd.setCursor(1, 1);
  lcd.print("Connecting Google...");
  Serial.println("LCD: WiFi Connected - Connecting Google Sheets...");
  delay(3000);

  Serial.print("Connecting to host: ");
  Serial.println(host);
  bool flag = false;
  for (int i = 0; i < 5; i++) {
    int retval = client->connect(host, httpsPort);
    if (retval == 1) {
      flag = true;
      Serial.println("Connected to Google host successfully.");
      lcd.clear();
      lcd.setCursor(3, 1);
      lcd.print("Google Sheets");
      lcd.setCursor(4, 2);
      lcd.print("Connected!");
      Serial.println("LCD: Google Sheets Connected");
      beepOnce();
      delay(2000);
      break;
    } else {
      Serial.println("Connection failed. Retrying...");
    }
  }

  if (!flag) {
    lcd.clear();
    lcd.setCursor(2, 1);
    lcd.print("Connection Failed");
    Serial.println("LCD: Connection to Google Failed");
    delay(5000);
    return;
  }

  delete client;
  client = nullptr;
}

//---------------- Loop ----------------
void loop() {
  static bool showScanMsg = true;

  if (!client) {
    client = new HTTPSRedirect(httpsPort);
    client->setInsecure();
    client->setPrintResponseBody(true);
    client->setContentTypeHeader("application/json");
  }

  if (client && !client->connected()) {
    client->connect(host, httpsPort);
  }

  if (showScanMsg) {
    lcd.clear();
    lcd.setCursor(3, 0);
    lcd.print("WELCOME TO GYM");
    lcd.setCursor(4, 2);
    lcd.print("Scan Your Card");
    Serial.println("LCD: WELCOME TO GYM - Scan your Card");
    showScanMsg = false;
  }

  mfrc522.PCD_Init();

  if (!mfrc522.PICC_IsNewCardPresent()) return;
  if (!mfrc522.PICC_ReadCardSerial()) {
    beepAlert();
    Serial.println("Error: Failed to read RFID card serial.");
    return;
  }

  beepOnce();
  Serial.println("RFID Card Detected!");

  String currentTime = getFormattedTime();

  member_id = ReadDataFromBlockClean(blocks[0]);
  if (member_id.length() == 0) {
    Serial.println("Warning: member_id empty after read.");
  } else {
    Serial.print("Member ID read: "); Serial.println(member_id);
  }

  String values_array = "\"" + currentTime + "\"";
  for (byte i = 0; i < total_blocks; i++) {
    String data = ReadDataFromBlockClean(blocks[i]);
    values_array += ",\"" + data + "\"";
  }

  payload = payload_base + "[" + values_array + "]}";
  Serial.println("JSON Sent: " + payload);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Member ID: " + member_id);
  lcd.setCursor(2, 2);
  lcd.print("Checking In...");
  Serial.println("LCD: MEMBER " + member_id + " Checking In...");

  if (client->POST(String("/macros/s/") + GScriptId + "/exec", host, payload)) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Enroll ID: " + member_id);
    lcd.setCursor(2, 1);
    lcd.print("CHECK-IN Success!");
    Serial.println("LCD: CHECK-IN Success for " + member_id);
    beepTwice();
  } else {
    Serial.println("Error while sending data to Google Sheets.");
    lcd.clear();
    lcd.setCursor(3, 1);
    lcd.print("CHECK-IN Failed");
    lcd.setCursor(4, 2);
    lcd.print("Try Again");
    Serial.println("LCD: CHECK-IN Failed. Try Again.");
    beepAlert();
  }

  delay(5000);
  showScanMsg = true;
}
