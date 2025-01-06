// Blynk configuration
#define BLYNK_TEMPLATE_ID "TMPL6gb_e"
#define BLYNK_TEMPLATE_NAME "Smart Irrigation System"
#define BLYNK_AUTH_TOKEN "6b8Mj1_s9uma26x-"

#include <LiquidCrystal_I2C.h>
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>

// LCD display initialization
LiquidCrystal_I2C lcd(0x27, 16, 2);

// WiFi credentials and Blynk authentication
char auth[] = "lGP6cpN_SjbVKbHLqp"; // Authentication token
char ssid[] = "VodaFone12";                         // WiFi SSID
char pass[] = "random@898";                         // WiFi password

// Blynk timer
BlynkTimer timer;

// Pin definitions
#define SENSOR_PIN A0      // Soil moisture sensor pin
#define WATER_PUMP_PIN D3  // Water pump pin

// Global variables
bool relayState = false;  // Stores the relay state

void setup() {
  // Initialize serial communication
  Serial.begin(9600);

  // Configure pins
  pinMode(WATER_PUMP_PIN, OUTPUT);
  digitalWrite(WATER_PUMP_PIN, HIGH); // Ensure the pump is initially off

  // Initialize LCD
  lcd.init();
  lcd.backlight();

  // Display loading message on LCD
  lcd.setCursor(1, 0);
  lcd.print("Please Wait ");
  for (int i = 0; i < 16; i++) {
    lcd.setCursor(i, 1);
    lcd.print(".");
    delay(500);
  }
  lcd.clear();

  // Connect to Blynk
  Blynk.begin(auth, ssid, pass, "blynk.cloud", 80);

  // Set a timer to read soil moisture values periodically
  timer.setInterval(100L, readSoilMoisture);
}

// Handle button input from the Blynk app
BLYNK_WRITE(V1) {
  relayState = param.asInt();

  if (relayState) {
    digitalWrite(WATER_PUMP_PIN, LOW); // Turn the water pump ON
    lcd.setCursor(0, 1);
    lcd.print("Motor is ON ");
  } else {
    digitalWrite(WATER_PUMP_PIN, HIGH); // Turn the water pump OFF
    lcd.setCursor(0, 1);
    lcd.print("Motor is OFF");
  }
}

// Read soil moisture sensor values
void readSoilMoisture() {
  int sensorValue = analogRead(SENSOR_PIN);
  int moisturePercentage = map(sensorValue, 0, 1024, 0, 100);
  moisturePercentage = 100 - moisturePercentage; // Convert to percentage

  // Send moisture value to Blynk app and display on LCD
  Blynk.virtualWrite(V0, moisturePercentage);
  lcd.setCursor(0, 0);
  lcd.print("Moisture Val: ");
  lcd.print(moisturePercentage);
  lcd.print(" %");
}

void loop() {
  Blynk.run();  // Run the Blynk library
  timer.run();  // Run the Blynk timer
}
