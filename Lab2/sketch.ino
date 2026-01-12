#include <WiFi.h>
#include <WebServer.h>
#include "DHT.h"

const char* ssid = "Wokwi-GUEST";
const char* password = "";

#define DHTPIN 5      
#define DHTTYPE DHT22 
#define LED_PIN 2     
#define PIR_PIN 18    
#define MQ2_PIN 19    

DHT dht(DHTPIN, DHTTYPE);
WebServer server(80);

unsigned long lastTime = 0;
unsigned long timerDelay = 2000;
bool pirState = false;
bool mq2State = false;

String getHTML(float t, float h) {
  String ptr = "<!DOCTYPE html> <html>\n";
  ptr +="<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
  ptr +="<meta http-equiv=\"refresh\" content=\"10\">\n"; 
  ptr +="<title>ESP32 Weather Station</title>\n";
  ptr +="<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}\n";
  ptr +="body{margin-top: 50px;} h1 {color: #991111;margin: 50px auto 30px;} p {font-size: 24px;color: #444444;}\n";
  ptr +="</style></head>\n";
  ptr +="<body>\n";
  ptr +="<h1>ESP32 Weather & Alarm Server</h1>\n";
  
  ptr +="<p>Temperature: ";
  ptr += String(t);
  ptr +=" <span>&#176</span>C</p>\n";
  
  ptr +="<p>Humidity: ";
  ptr += String(h);
  ptr +=" %</p>\n";

  ptr +="<p>Motion Status: ";
  ptr += (pirState ? "<span style='color:red'>DETECTED!</span>" : "Clear");
  ptr +="</p>\n";
  
  ptr +="<p>Smoke Alarm: ";
  ptr += (mq2State ? "<span style='color:red'>DANGER!</span>" : "Safe");
  ptr +="</p>\n";

  ptr +="<p><a href=\"/led/on\"><button>LED ON</button></a>&nbsp;<a href=\"/led/off\"><button>LED OFF</button></a></p>\n";
  
  ptr +="</body></html>";
  return ptr;
}

void handleRoot() {
  float t = dht.readTemperature();
  float h = dht.readHumidity();
  server.send(200, "text/html", getHTML(t, h));
}

void handleLedOn() {
  digitalWrite(LED_PIN, HIGH);
  server.sendHeader("Location", "/"); 
  server.send(303);
  Serial.println("[WEB] LED Turned ON");
}

void handleLedOff() {
  digitalWrite(LED_PIN, LOW);
  server.sendHeader("Location", "/");
  server.send(303);
  Serial.println("[WEB] LED Turned OFF");
}

void setup() {
  Serial.begin(115200);
  
  pinMode(LED_PIN, OUTPUT);
  pinMode(PIR_PIN, INPUT);
  pinMode(MQ2_PIN, INPUT_PULLUP); 

  dht.begin();

  Serial.print("Connecting to WiFi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("WiFi connected. IP address: ");
  Serial.println(WiFi.localIP());

  server.on("/", handleRoot);
  server.on("/led/on", handleLedOn);
  server.on("/led/off", handleLedOff);
  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  server.handleClient(); 

  bool currentPir = digitalRead(PIR_PIN);
  if (currentPir && !pirState) {
    pirState = true;
    digitalWrite(LED_PIN, HIGH); 
    Serial.println("[TELEGRAM SIM] Motion detected! -> Sending msg to Bot..."); 
  } else if (!currentPir && pirState) {
    pirState = false;
    digitalWrite(LED_PIN, LOW);
    Serial.println("[TELEGRAM SIM] Motion ended."); 
  }

  bool currentMq2 = !digitalRead(MQ2_PIN); 
  if (currentMq2 && !mq2State) {
    mq2State = true;
    digitalWrite(LED_PIN, HIGH); 
    Serial.println("[TELEGRAM SIM] SMOKE DETECTED! DANGER! -> Sending alert..."); 
  } else if (!currentMq2 && mq2State) {
    mq2State = false;
    digitalWrite(LED_PIN, LOW);
    Serial.println("[TELEGRAM SIM] Smoke cleared."); 
  }

  if ((millis() - lastTime) > 10000) {
    float t = dht.readTemperature();
    float h = dht.readHumidity();
    
    Serial.println("--- Bot Check ---");
    Serial.print("Temp: "); Serial.print(t);
    Serial.print(" C, Hum: "); Serial.print(h); Serial.println(" %");
    Serial.println("Status: Waiting for commands (/start, /led on, /readings)...");
    
    lastTime = millis();
  }
}