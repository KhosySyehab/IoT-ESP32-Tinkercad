#include <WiFi.h>
#include <HTTPClient.h>
#include <ThingSpeak.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// WIFI
const char* ssid = " ";
const char* password = " ";

// ThingSpeak
WiFiClient client;
unsigned long channelID = 3095423;
const char* writeAPIKey = "MQ869CAWMZ5XHW6S";

// OLED
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// HC-SR04
#define TRIG_PIN 5
#define ECHO_PIN 18

// Water
#define WATER_SENSOR_PIN 34
int waterValue = 0;
int bacaWaterSensor() {
  int value = analogRead(WATER_SENSOR_PIN);
  return value;
}

// LED dan Buzzer
#define LED_PIN 2
#define BUZZER_PIN 4

// Timer
unsigned long lastSensorUpdate = 0;
unsigned long lastThingSpeakUpdate = 0;
const unsigned long sensorInterval = 200;
const unsigned long thingspeakInterval = 15000;

// Fungsi Baca Jarak
long bacaJarak() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH);
  long distance = duration * 0.034 / 2;
  return distance;
}

void setup() {
  Serial.begin(115200);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  // OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("OLED tidak ditemukan"));
    for (;;);
  }
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);

  // WiFi
  WiFi.begin(ssid, password);
  Serial.print("Menghubungkan WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi Terkoneksi!");

  // ThingSpeak
  ThingSpeak.begin(client);
}

void loop() {
  unsigned long now = millis();
  // Update Sensor + OLED + LED + Buzzer setiap 200ms
  if (now - lastSensorUpdate >= sensorInterval) {
    long jarak = bacaJarak();
    long waterValue = bacaWaterSensor();
    Serial.print("Jarak: ");
    Serial.print(jarak);
    Serial.print(" cm | Water: ");
    Serial.println(waterValue);

    display.clearDisplay();
    display.setCursor(0, 20);

if (WiFi.status() != WL_CONNECTED) {
  Serial.println("WiFi terputus, mencoba reconnecting....");
  WiFi.reconnect();
}

    if (jarak <= 10) {
      display.setTextSize(3);
      display.println("AWAS!");
      display.setCursor(0, 45);
      display.setTextSize(3);
      display.println("Pak Hatma");
      digitalWrite(LED_PIN, HIGH);
      digitalWrite(BUZZER_PIN, LOW);
    } else {
      display.setTextSize(3);
      display.println("Halo");
      display.setCursor(0, 45);
      display.setTextSize(3);
      display.println("Pak Hatma");
      digitalWrite(LED_PIN, LOW);
      digitalWrite(BUZZER_PIN, HIGH);
    }

    display.display();

    lastSensorUpdate = now;
  }

  // Kirim ke ThingSpeak setiap 15 detik
if (now - lastThingSpeakUpdate >= thingspeakInterval) {
    long jarak = bacaJarak();
    int waterValue = bacaWaterSensor();
  
    ThingSpeak.setField(1, jarak);
    ThingSpeak.setField(2, waterValue);

    int statusCode = ThingSpeak.writeFields(channelID, writeAPIKey);

    if (statusCode == 200) {
      Serial.println("Data terkirim ke ThingSpeak");
      Serial.print("Jarak: ");
      Serial.print(jarak);
      Serial.print(" cm | Water: ");
      Serial.println(waterValue);
    } else {
      Serial.print("Gagal kirim. Kode: ");
      Serial.println(statusCode);
    }
    lastThingSpeakUpdate = now;
}

