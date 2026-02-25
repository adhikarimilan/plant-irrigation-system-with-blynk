// Blynk configuration
#define BLYNK_TEMPLATE_ID "TMPL6gb9RSV_e"
#define BLYNK_TEMPLATE_NAME "Smart Irrigation System"
#define BLYNK_AUTH_TOKEN "6b8Mj1_suX-"

#include <LiquidCrystal_I2C.h>
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>

// LCD display initialization
LiquidCrystal_I2C lcd(0x27, 16, 2);

// WiFi credentials and Blynk authentication
char auth[] = "6b8Mj1_s9uma26xg1Nl5NJIFvdCTvuX-"; // Authentication token
char ssid[] = "VodaFone12";                         // WiFi SSID
char pass[] = "r@ndom00";                         // WiFi password

// Blynk timer
BlynkTimer timer;

// Pin definitions
#define SENSOR_PIN A0               // Soil moisture sensor pin
#define WATER_PUMP_RELAY_PIN D5     // Water pump relay pin
#define RED_LED_PIN D6

// Global variables
bool waterPumpRelayState = false;  // Stores the relay state
unsigned long lastReconnectAttempt = 0;  // To manage Blynk reconnection attempts
unsigned long wifiReconnectTimer = 0;    // To manage Wi-Fi reconnection attempts

void setup() {
  // Initialize serial communication
  Serial.begin(9600);

  // Configure pins
  pinMode(WATER_PUMP_RELAY_PIN, OUTPUT);
  digitalWrite(WATER_PUMP_RELAY_PIN, HIGH); // Ensure the pump is initially off when connected to NO pin
  pinMode(RED_LED_PIN, OUTPUT);
  digitalWrite(RED_LED_PIN,LOW);

  // Initialize LCD
  lcd.init();
  lcd.backlight();

  
  // Connect to Blynk
  //Blynk.begin(auth, ssid, pass, "blynk.cloud", 80);
  Blynk.config(auth, "blynk.cloud", 80);
  //Blynk.connectWiFi(ssid, pass);
  WiFi.begin(ssid,pass);
  //WiFi.setAutoReconnect(true);
  //WiFi.persistent(true);

  // Display loading message on LCD
  lcd.setCursor(0, 0);
  lcd.print("Please Wait ");
  for (int i = 0; i < 16; i++) {
    lcd.setCursor(i, 1);
    lcd.print(".");
    delay(500);
  }
  lcd.clear();

  //initial output of lcd 
  lcd.setCursor(0, 0);
  lcd.print("Moist Val: ");
  lcd.print("00 %");
  lcd.setCursor(0,1);
  lcd.print("Motor is OFF");

  // Set a timer to read soil moisture values periodically
  timer.setInterval(1000L, readSoilMoisture);
  
  if(WiFi.status() == WL_CONNECTED)
  Blynk.connect(1000);
}

// Handle button input from the Blynk app
BLYNK_WRITE(V1) {
  waterPumpRelayState = param.asInt() || 0;

  if (waterPumpRelayState) {
    digitalWrite(WATER_PUMP_RELAY_PIN, LOW); // Turn the water pump ON when connected to NO pin
    updateLCDMotorState(true); // Update motor state on LCD
  } else {
    digitalWrite(WATER_PUMP_RELAY_PIN, HIGH); // Turn the water pump OFF when connected to NO pin
    updateLCDMotorState(false); // Update motor state on LCD
  }
}

// Function to update the LCD display
void updateLCDMotorState( bool motorState) {
  static bool lastMotorState = false;

  // Update LCD only if values have changed
  if (motorState != lastMotorState) {
    lcd.setCursor(0, 1);
    lcd.print(motorState ? F("Motor is ON ") : F("Motor is OFF"));

    lastMotorState = motorState;
  }
}

void updateLCDMoisture(int moisturePercentage){
  static int lastMoisture=-1;

  if (moisturePercentage != lastMoisture) {
    lcd.setCursor(0, 0);
    lcd.print(F("Moist Val: "));
    if (moisturePercentage < 10) lcd.print("0");
    lcd.print(moisturePercentage);
    lcd.print(" %");

    lastMoisture = moisturePercentage;
  }
}

// Read soil moisture sensor values
void readSoilMoisture() {
  int sensorValue = analogRead(SENSOR_PIN);
  int moisturePercentage = map(sensorValue, 0, 1024, 0, 100);
  moisturePercentage = 100 - moisturePercentage; // Convert to percentage

  
    // Send moisture value to Blynk app if connected
  if (WiFi.status() == WL_CONNECTED && Blynk.connected()) {
    Blynk.virtualWrite(V0, moisturePercentage);
    updateLCDMoisture(moisturePercentage);
  }

  // Handle motor auto-control based on moisture level only when not connected to blynk
  else
  if (moisturePercentage > 10 && moisturePercentage < 30) {
    digitalWrite(WATER_PUMP_RELAY_PIN, LOW); // Turn the pump ON
    updateLCDMoisture(moisturePercentage);
    updateLCDMotorState(true);
  } else {
    digitalWrite(WATER_PUMP_RELAY_PIN, HIGH); // Turn the pump OFF
    updateLCDMoisture(moisturePercentage);
    updateLCDMotorState(false);
  }
  
}

// Function to handle Wi-Fi reconnection
void reconnectWiFi() {
  if (WiFi.status() != WL_CONNECTED) {
    if (millis() - wifiReconnectTimer > 10000) {  // Attempt reconnect every 10 seconds
      wifiReconnectTimer = millis();
      WiFi.begin(ssid, pass);
      Serial.println("Attempting to reconnect Wi-Fi...");
    }
  }
}

void BlinkRedLed(int delay_time=300){
  digitalWrite(RED_LED_PIN, HIGH);
  delay(delay_time);
  digitalWrite(RED_LED_PIN, LOW);
}

void loop() {  
    timer.run();  // Run the Blynk timer

  reconnectWiFi();  // Check and reconnect Wi-Fi if necessary

  // Handle Blynk reconnection attempts
  unsigned long now = millis();
  if (WiFi.status() == WL_CONNECTED && !Blynk.connected()) {
    if (now - lastReconnectAttempt > 5000) {  // Attempt reconnect every 5 seconds
      lastReconnectAttempt = now;
      Blynk.connect(1000);  // Non-blocking connection attempt
      Serial.println("Attempting to reconnect to Blynk...");
    }
  }

  // Run Blynk only if connected
  if (WiFi.status() == WL_CONNECTED && Blynk.connected()) {
    Blynk.run();
  }

  // Debugging output
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Wi-Fi disconnected!");
  }

  if (!Blynk.connected()) {
    Serial.println("Blynk disconnected!");
    BlinkRedLed(400);
  }

  delay(100);  // Small delay to yield control
}