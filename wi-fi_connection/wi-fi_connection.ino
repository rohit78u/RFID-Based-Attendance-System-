#include <ESP8266WiFi.h>

// 🔹 Replace these with your hotspot details
const char* ssid = "Batman_PC";       // Your hotspot name
const char* password = "12345678";    // Your hotspot password

void setup() {
  Serial.begin(115200);
  delay(100);

  Serial.println();
  Serial.println("🔹 Connecting to WiFi...");
  Serial.print("SSID: ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  // Wait until connected
  int retry = 0;
  while (WiFi.status() != WL_CONNECTED && retry < 40) { // 20 sec timeout
    delay(500);
    Serial.print(".");
    retry++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n✅ WiFi connected successfully!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\n❌ Failed to connect to WiFi.");
    Serial.println("Check SSID, password, and 2.4GHz band settings.");
  }
}

void loop() {
  // Keep checking connection
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("⚠️ WiFi lost... trying to reconnect...");
    WiFi.begin(ssid, password);
    delay(10000);
  } else {
    Serial.println("✅ Still connected!");
    delay(10000);
  }
}
