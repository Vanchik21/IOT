#include <esp_now.h>
#include <WiFi.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define ROLE_PIN 4       

#define TRIG_PIN 5        
#define ECHO_PIN 18       
#define SCREEN_WIDTH 128  
#define SCREEN_HEIGHT 64  

uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

typedef struct struct_message {
  float distance;
} struct_message;

struct_message myData;
bool isReceiver = false;

void OnDataRecv(const esp_now_recv_info_t * info, const uint8_t *incomingData, int len) {
  memcpy(&myData, incomingData, sizeof(myData));
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.println("ESP-NOW: RECEIVED");
  display.setTextSize(2);
  display.setCursor(0, 25);
  display.print(myData.distance, 1);
  display.print(" cm");
  display.display();
}

void setupReceiver() {
  Serial.println("РЕЖИМ: ПРИЙМАЧ З ЕКРАНОМ");
  
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    Serial.println("Помилка SSD1306 allocation failed"); 
    for(;;);
  }
  
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.println("Waiting for Data...");
  display.display();
  esp_now_register_recv_cb(OnDataRecv);
}

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
}

void setupSender() {
  Serial.println("РЕЖИМ: ПЕРЕДАВАЧ (ДАТЧИК)");
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  esp_now_register_send_cb(OnDataSent);
  esp_now_peer_info_t peerInfo;
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
  }
}

void loopSender() {
  digitalWrite(TRIG_PIN, LOW); delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH); delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  long duration = pulseIn(ECHO_PIN, HIGH);
  float distanceCm = duration * 0.034 / 2;
  if (distanceCm > 400 || distanceCm < 2) distanceCm = 0;
  myData.distance = distanceCm;
  esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));
  Serial.print("Відстань: "); Serial.println(distanceCm);
  delay(500); 
}

void setup() {
  Serial.begin(115200);
  pinMode(ROLE_PIN, INPUT_PULLUP); 
  
  WiFi.mode(WIFI_STA);
  
  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW Init Failed");
    return;
  }

  if (digitalRead(ROLE_PIN) == LOW) {
    isReceiver = true;
    setupReceiver();
  } else {
    isReceiver = false;
    setupSender();
  }
}

void loop() {
  if (!isReceiver) {
    loopSender();
  }
  delay(10); 
}