
#include <WiFi.h> 
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

const char *ssid     = "Wokwi-GUEST";
const char *password = "";

const long utcOffsetInSeconds = 7200; // GMT+2 (Київ)
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64 
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
const int ledPin = 2; 

void setup() {
  Serial.begin(115200);
  
  pinMode(ledPin, OUTPUT);

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  display.clearDisplay();
  display.setTextColor(WHITE);
  
  Serial.println("Start WiFi Scan...");
  int n = WiFi.scanNetworks();
  if (n == 0) {
      Serial.println("no networks found");
  } else {
      Serial.print(n);
      Serial.println(" networks found");
  }

  WiFi.begin(ssid, password);
  while ( WiFi.status() != WL_CONNECTED ) {
    delay ( 500 );
    Serial.print ( "." );
  }
  Serial.println("\nWiFi connected!");

  timeClient.begin();
}

void loop() {
  timeClient.update();
  
  String formattedTime = timeClient.getFormattedTime();
  int currentMinute = timeClient.getMinutes();

  if (currentMinute % 2 == 0) {
     digitalWrite(ledPin, HIGH); // Увімкнути
  } else {
     digitalWrite(ledPin, LOW);  // Вимкнути
  }

  display.clearDisplay();
  
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("ESP32 Lab Work"); 
  
  display.setTextSize(2);
  display.setCursor(15, 20);
  display.println(formattedTime); 

  display.setTextSize(1);
  display.setCursor(0, 45);
  if (digitalRead(ledPin)) {
    display.println("Output: ON");
  } else {
    display.println("Output: OFF");
  }
  
  display.display();
  delay(1000);
}