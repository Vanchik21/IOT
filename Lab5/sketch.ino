#define BLYNK_TEMPLATE_ID "TMPLxxxxxx"
#define BLYNK_TEMPLATE_NAME "Gas Monitor"
#define BLYNK_AUTH_TOKEN "YourAuthTokenHere"

#include <WiFi.h>
#include <math.h>

#ifdef USE_REAL_BLYNK
  #include <BlynkSimpleEsp32.h>
  char auth[] = BLYNK_AUTH_TOKEN;
  char ssid[] = "Wokwi-GUEST"; 
  char pass[] = "";
#endif

#define ADC_PIN 34       
float VCC = 3.3;         
float RL = 2.0;          
float RO = 7.2;          
float m = -0.32;         
float b = 1.44;          

unsigned long previousMillis = 0;
const long interval = 1000; 

void sendSensorData() {
  int adcValue = analogRead(ADC_PIN);
  
  float Vout = (adcValue * VCC) / 4095.0;
  if (Vout == 0) Vout = 0.01; 

  float RS = (VCC / Vout - 1) * RL;
  float ratio = RS / RO;
  float ppm = pow(10, ((log10(ratio) - b) / m));

  Serial.print("Vout: "); Serial.print(Vout); Serial.print("V | ");
  Serial.print("Rs: "); Serial.print(RS); Serial.print("k | ");
  Serial.print("PPM CO: "); Serial.println(ppm);

  #ifdef USE_REAL_BLYNK
    Blynk.virtualWrite(V4, ppm);
  #else
    Serial.print("[BLYNK SIM] Sending to V4: ");
    Serial.println(ppm);
  #endif
}

void setup() {
  Serial.begin(115200);
  analogReadResolution(12);

  #ifdef USE_REAL_BLYNK
    Blynk.begin(auth, ssid, pass);
  #else
    Serial.println("--- BLYNK SIMULATION MODE ---");
  #endif
}

void loop() {
  #ifdef USE_REAL_BLYNK
    Blynk.run();
  #endif
  
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    sendSensorData();
  }
}