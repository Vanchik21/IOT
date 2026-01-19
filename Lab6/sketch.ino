#include <WiFi.h>
#include <esp_now.h>
#include <DHT.h>
#include <HTTPClient.h>

String apiKey = "K8Z16P7T1C5MVKA0";

const char* ssid = "Wokwi-GUEST";
const char* password = "";

#define DHTPIN 15
#define DHTTYPE DHT22
#define MQ_PIN 34 

DHT dht(DHTPIN, DHTTYPE);

typedef struct struct_message {
  float temp;
  float hum;
  float ppm;
} struct_message;

struct_message myData;
bool isGateway = false;
bool dataReceived = false;

uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

float calculatePPM(int adc) {
  if (adc < 10) adc = 10; 
  float VCC = 3.3; float RL = 2.0; float RO = 7.2;
  float m = -0.32; float b = 1.44;
  float Vout = (adc * VCC) / 4095.0;
  float RS = (VCC / Vout - 1) * RL;
  float ratio = RS / RO;
  return pow(10, ((log10(ratio) - b) / m));
}

void senderLoop() {
  float t = dht.readTemperature();
  float h = dht.readHumidity();
  int adc = analogRead(MQ_PIN);
  float ppm = calculatePPM(adc);

  if (isnan(t) || isnan(h)) {
    Serial.println("Error DHT");
    delay(2000);
    return;
  }

  myData.temp = t;
  myData.hum = h;
  myData.ppm = ppm;

  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));

  Serial.print("Sending: ");
  Serial.print(t); Serial.print("C, ");
  Serial.print(ppm); Serial.println(" ppm");

  delay(20000); 
}

void OnDataRecv(const esp_now_recv_info_t * info, const uint8_t *incomingData, int len) {
  memcpy(&myData, incomingData, sizeof(myData));
  dataReceived = true;
  Serial.println("\nPacket Received");
}

void gatewayLoop() {
  if (dataReceived) {
    if (WiFi.status() != WL_CONNECTED) {
      WiFi.disconnect();
      WiFi.begin(ssid, password);
      while (WiFi.status() != WL_CONNECTED) {
        delay(500); 
      }
    }

    HTTPClient http;
    
    String serverPath = "http://api.thingspeak.com/update?api_key=" + apiKey + 
                        "&field1=" + String(myData.temp) + 
                        "&field2=" + String(myData.hum) + 
                        "&field3=" + String(myData.ppm);
    
    Serial.println("URL: " + serverPath); 

    http.begin(serverPath.c_str());
    
    int httpResponseCode = http.GET();
    
    if (httpResponseCode > 0) {
      Serial.print("Response: ");
      Serial.println(httpResponseCode);
    }
    else {
      Serial.print("Error: ");
      Serial.println(httpResponseCode);
    }
    
    http.end();
    dataReceived = false; 
  }
}

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_AP_STA);

  dht.begin();
  delay(200);
  float checkSensor = dht.readTemperature();

  if (isnan(checkSensor)) {
    isGateway = true;
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
      delay(500); 
    }
    
    esp_now_init();
    esp_now_register_recv_cb(OnDataRecv);
  } 
  else {
    isGateway = false;
    esp_now_init();
    esp_now_peer_info_t peerInfo;
    memcpy(peerInfo.peer_addr, broadcastAddress, 6);
    peerInfo.channel = 0;  
    peerInfo.encrypt = false;
    esp_now_add_peer(&peerInfo);
  }
}

void loop() {
  if (isGateway) {
    gatewayLoop();
  } else {
    senderLoop();
  }
  delay(100);
}