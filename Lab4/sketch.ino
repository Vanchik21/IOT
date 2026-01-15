#include <WiFi.h>

#define PIN_ACCESS_LED 12 
#define PIN_DENY_LED   14 

struct RFIDTag {
  String uid;
  String role;
  String surname; 
  String name;    
};

RFIDTag chief = {"DE AD BE EF", "Головний Інженер", "Petrenko", "Ivan"}; 
RFIDTag eng   = {"CA FE BA BE", "Інженер", "Sydorenko", "Oleg"};         
RFIDTag tech  = {"12 34 56 78", "Технік", "Kovalenko", "Anna"};          

void setup() {
  Serial.begin(115200);
  pinMode(PIN_ACCESS_LED, OUTPUT);
  pinMode(PIN_DENY_LED, OUTPUT);

  Serial.println("--- СИСТЕМА КОНТРОЛЮ ДОСТУПУ RFID (ЕМУЛЯЦІЯ) ---");
  Serial.println("Система готова.");
  Serial.println("Натисніть '1', '2' або '3' у Serial Monitor для імітації прикладання картки:");
  Serial.println("1 - Головний інженер (Синій)");
  Serial.println("2 - Інженер (Жовтий)");
  Serial.println("3 - Технік (Картка)");
  Serial.println("---------------------------------------------------");
}

void loop() {
  if (Serial.available()) {
    char key = Serial.read();
    
    if (key == '\n' || key == '\r' || key == ' ') return;

    RFIDTag currentCard;
    bool cardPresent = false;
    if (key == '1') { currentCard = chief; cardPresent = true; }
    else if (key == '2') { currentCard = eng; cardPresent = true; }
    else if (key == '3') { currentCard = tech; cardPresent = true; }
    else { Serial.println("Невідома картка"); return; }

    if (cardPresent) {
      processCard(currentCard);
    }
  }
}

void processCard(RFIDTag card) {
  Serial.println("\n[RFID] Картку виявлено!");
  Serial.print("[RFID] UID мітки: ");
  Serial.println(card.uid);
  Serial.println("\n--- ЗАПИС ДАНИХ У СЕКТОРИ ---");
  Serial.print("Запис прізвища '" + card.surname + "' у Блок 1... ");
  delay(200); 
  Serial.println("ОК (Запис вдало завершений)");
  Serial.print("Запис імені '" + card.name + "' у Блок 4... ");
  delay(200);
  Serial.println("ОК (Запис вдало завершений)");
  Serial.println("Заберіть мітку (віртуально)...");
  delay(500);
  Serial.println("\n--- ПЕРЕВІРКА ДОСТУПУ ---");
  Serial.println("Зчитування даних з картки...");
  Serial.print("Прочитано Ім'я (Блок 4): "); Serial.println(card.name);
  Serial.print("Прочитано Прізвище (Блок 1): "); Serial.println(card.surname);

  if (card.role == "Головний Інженер") {
    grantAccess(card);
  } else {
    denyAccess(card);
  }
}

void grantAccess(RFIDTag card) {
  Serial.println("\nSTATUS: ДОСТУП ДОЗВОЛЕНО!");
  digitalWrite(PIN_ACCESS_LED, HIGH);
  sendTelegramMessage("АВТОРИЗОВАНИЙ ДОСТУП: " + card.surname + " " + card.name);
  delay(3000); 
  digitalWrite(PIN_ACCESS_LED, LOW);
}

void denyAccess(RFIDTag card) {
  Serial.println("\nSTATUS: ДОСТУП ЗАБОРОНЕНО!");
  digitalWrite(PIN_DENY_LED, HIGH);
  sendTelegramMessage("НЕАВТОРИЗОВАНИЙ ДОСТУП (УВАГА!): " + card.surname + " " + card.name);
  delay(1000);
  digitalWrite(PIN_DENY_LED, LOW);
}

void sendTelegramMessage(String msg) {
  Serial.print("[Telegram Bot] Надсилаю повідомлення... ");
  delay(500); 
  Serial.println("ВІДПРАВЛЕНО!");
  Serial.print("[Telegram Bot] Текст: \"");
  Serial.print(msg);
  Serial.println("\"");
}