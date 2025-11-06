#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Set the LCD address to 0x27 for a 16 chars and 2 line display
LiquidCrystal_I2C lcd(0x27, 16, 2);  // Use 0x3F if 0x27 doesn’t work

void setup() {
  // Initialize I2C for ESP8266 (D2 = SDA, D1 = SCL)
  Wire.begin(D2, D1);

  // Initialize LCD
  lcd.init();      
  lcd.backlight(); 

  // Display test message
  lcd.setCursor(0, 0);
  lcd.print("Hello Rohit!");
  lcd.setCursor(0, 1);
  lcd.print("LCD Working OK");
}

void loop() {
  // Nothing to loop — LCD will show static text
}
