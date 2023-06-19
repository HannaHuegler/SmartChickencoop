#include <SPI.h>
#include <MFRC522.h>

// PIN-Nummern: RESET + SDAs
#define RST_PIN         9
#define SS_PIN1         8
#define SS_PIN2         10

MFRC522 mfrc522_1(SS_PIN1, RST_PIN);
MFRC522 mfrc522_2(SS_PIN2, RST_PIN);

unsigned long timestamp1 = 0;
unsigned long timestamp2 = 0;
bool reader1Detected = false;
bool reader2Detected = false;

struct TagData {
  byte uid[4];
  String name;
};

TagData tagList[] = {
  { { 0x27, 0x19, 0x28, 0x1B }, "Hildegart" },    // RFID-Tag mit UID 27 19 28 1B und Name "Hildegart"
  { { 0xDA, 0xAB, 0xFB, 0x80 }, "Agathe Bauer" }, // RFID-Tag mit UID DA AB FB 80 und Name "Agathe Bauer"
  // Weitere RFID-Tags und Namen können hier hinzugefügt werden
};

// Gleichstrommotor 1
int GSM1 = 13;
int in1 = 12;
int in2 = 11;

void setup() {
  Serial.begin(9600);
  SPI.begin();

  mfrc522_1.PCD_Init(); // Initialisiert den ersten RFID-Reader
  mfrc522_2.PCD_Init(); // Initialisiert den zweiten RFID-Reader

  pinMode(GSM1, OUTPUT);
  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);

  Serial.println("Approximate your card to the readers..."); // Aufforderung, die Karte an die Reader zu halten
  Serial.println();
}

void loop() {
  // Sucht nach neuen Karten auf Reader 1
  if (mfrc522_1.PICC_IsNewCardPresent() && mfrc522_1.PICC_ReadCardSerial()) {
    Serial.print("Reader 1 - UID-Tag: ");
    printUIDWithNames(mfrc522_1.uid.uidByte); // Druckt die UID des erkannten Tags mit dem zugehörigen Namen
    timestamp1 = millis();
    reader1Detected = true; // Markiert, dass auf Reader 1 eine Karte erkannt wurde
    Serial.println("Authorized access"); // Zugriff autorisiert
    Serial.println();
    mfrc522_1.PICC_HaltA();
    mfrc522_1.PCD_StopCrypto1();
  }

  // Sucht nach neuen Karten auf Reader 2
  if (mfrc522_2.PICC_IsNewCardPresent() && mfrc522_2.PICC_ReadCardSerial()) {
    Serial.print("Reader 2 - UID-Tag: ");
    printUIDWithNames(mfrc522_2.uid.uidByte); // Druckt die UID des erkannten Tags mit dem zugehörigen Namen
    timestamp2 = millis();
    reader2Detected = true; // Markiert, dass auf Reader 2 eine Karte erkannt wurde
    Serial.println("Authorized access"); // Zugriff autorisiert
    Serial.println();
    mfrc522_2.PICC_HaltA();
    mfrc522_2.PCD_StopCrypto1();
  }

  // Überprüft, ob beide Reader erkannt wurden
  if (reader1Detected && reader2Detected) {
    activateMotor();

    if (timestamp1 < timestamp2) {
      Serial.println("Direction: Draußen"); // Richtung: Draußen
    } else {
      Serial.println("Direction: Drinnen"); // Richtung: Drinnen
    }

    // Setzt die Erkennungsflags und Zeitstempel zurück
    reader1Detected = false;
    reader2Detected = false;
    timestamp1 = 0;
    timestamp2 = 0;

    Serial.println();
  }
}

void printUIDWithNames(byte *uid) {
  for (byte i = 0; i < mfrc522_1.uid.size; i++) {
    Serial.print(uid[i] < 0x10 ? " 0" : " "); // Druckt die UID des Tags mit führender Null, falls nötig
    Serial.print(uid[i], HEX); // Druckt jede Hexadezimalziffer der UID
  }
  Serial.print(" - ");

  // Sucht den Tag-Namen basierend auf der UID
  for (int i = 0; i < sizeof(tagList) / sizeof(tagList[0]); i++) {
    bool match = true;
    for (byte j = 0; j < mfrc522_1.uid.size; j++) {
      if (tagList[i].uid[j] != uid[j]) {
        match = false;
        break;
      }
    }
    if (match) {
      Serial.println(tagList[i].name); // Druckt den Namen des gefundenen Tags
      return;
    }
  }

  Serial.println("Unknown"); // Wenn kein entsprechender Tag gefunden wurde, wird "Unknown" gedruckt
}

void activateMotor() {
  digitalWrite(in1, HIGH); // Motor 1 beginnt zu rotieren
  digitalWrite(in2, LOW);

  analogWrite(GSM1, 255); // Motor 1 soll mit der Geschwindigkeit "200" (max. 255) rotieren

  delay(55000);

  digitalWrite(in1, LOW); // Anschließend sollen die Motoren 2 Sekunden ruhen.
  digitalWrite(in2, LOW);
}