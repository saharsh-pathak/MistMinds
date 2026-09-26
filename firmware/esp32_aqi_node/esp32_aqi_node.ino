#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <DHT.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"
#include "secrets.h"

// Hardware Pin Definitions
#define RELAY_PIN  23
#define SWITCH_PIN 27
#define POT_PIN    35
#define DHTPIN     4
#define DHTTYPE    DHT22

LiquidCrystal_I2C lcd(0x27, 16, 2);
DHT dht(DHTPIN, DHTTYPE);

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

// State Variables
bool manualMode = false;
bool lastButtonState = HIGH;

float liveAQI = 60;
float manualAQI = 60;
int AQI = 60;

float liveSum = 0;
int liveCount = 0;

unsigned long lastLiveStep = 0;
unsigned long lastAverageReset = 0;
unsigned long lastFirebasePush = 0;

void connectWiFi() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  Serial.println("\nWiFi Connected!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
}

void updateLiveAQI() {
  if (millis() - lastLiveStep > 3000) {
    int step = random(-2, 3);
    liveAQI += step;

    if (liveAQI < 20) liveAQI = 20;
    if (liveAQI > 95) liveAQI = 95;

    liveSum += liveAQI;
    liveCount++;

    lastLiveStep = millis();
  }

  if (millis() - lastAverageReset > 15000) {
    if (liveCount > 0) {
      AQI = (int)(liveSum / liveCount);
    }

    liveSum = 0;
    liveCount = 0;
    lastAverageReset = millis();
  }
}

void checkModeToggle() {
  bool reading = digitalRead(SWITCH_PIN);

  if (reading == LOW && lastButtonState == HIGH) {
    manualMode = !manualMode;

    if (!manualMode) {
      liveAQI = random(40, 70);
      AQI = liveAQI;
      liveSum = 0;
      liveCount = 0;
      lastAverageReset = millis();
    } else {
      int pot = analogRead(POT_PIN);
      manualAQI = map(pot, 0, 4095, 20, 300);
      AQI = manualAQI;
    }

    lcd.clear();
  }

  lastButtonState = reading;
}

void setup() {
  Serial.begin(115200);
  Wire.begin(21, 22);

  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);

  pinMode(SWITCH_PIN, INPUT_PULLUP);

  lcd.init();
  lcd.backlight();

  dht.begin();

  connectWiFi();

  // Firebase Initialization
  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;
  config.token_status_callback = tokenStatusCallback;

  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  fbdo.setResponseSize(2048);
  config.timeout.serverResponse = 10000;

  Serial.println("Initializing Firebase connection...");
  delay(3000);

  lcd.setCursor(0, 0);
  lcd.print("AQI SYSTEM");
  delay(2000);
  lcd.clear();
}

void loop() {
  checkModeToggle();

  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();

  if (isnan(humidity)) humidity = 0;
  if (isnan(temperature)) temperature = 0;

  if (manualMode) {
    int pot = analogRead(POT_PIN);
    int target = map(pot, 0, 4095, 20, 300);

    manualAQI = manualAQI * 0.85 + target * 0.15;
    AQI = (int)manualAQI;
  } else {
    updateLiveAQI();
  }

  // Relay Activation Logic
  bool sprinklerActive = false;
  if (AQI > 150) {
    digitalWrite(RELAY_PIN, HIGH);
    sprinklerActive = true;
  } else {
    digitalWrite(RELAY_PIN, LOW);
  }

  // LCD Update
  lcd.setCursor(0, 0);
  lcd.print("AQI:");
  lcd.print(AQI);
  lcd.print(" H:");
  lcd.print((int)humidity);
  lcd.print("%  ");

  lcd.setCursor(0, 1);
  lcd.print("T:");
  lcd.print((int)temperature);
  lcd.print("C ");
  lcd.print(manualMode ? "M" : "L");
  lcd.print("        ");

  // Firebase Real-time Push
  if (Firebase.ready() && millis() - lastFirebasePush > 2000) {
    FirebaseJson json;
    json.set("aqi", AQI);
    json.set("humidity", humidity);
    json.set("temperature", temperature);
    json.set("mode", manualMode ? "MANUAL" : "LIVE");
    json.set("sprinklerActive", sprinklerActive);
    json.set("timestamp/.sv", "timestamp");

    if (Firebase.RTDB.updateNode(&fbdo, "nodes/Node1", &json)) {
      Serial.println("Firebase Update OK");
    } else {
      Serial.print("Firebase Update Error: ");
      Serial.println(fbdo.errorReason());
    }

    if (Firebase.RTDB.pushJSON(&fbdo, "history/Node1", &json)) {
      Serial.println("Firebase History Push OK");
    } else {
      Serial.print("Firebase Push Error: ");
      Serial.println(fbdo.errorReason());
    }

    lastFirebasePush = millis();
  }
}