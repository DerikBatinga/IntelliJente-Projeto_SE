#define BLYNK_TEMPLATE_ID "TMPL2vCK3JGi6"
#define BLYNK_TEMPLATE_NAME "testLed"
#define BLYNK_AUTH_TOKEN "OS4CMvnheFSqPny83QX7UUtTKAZldyrG"

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <SPI.h>
#include <MFRC522.h>

#define RFID_SS   10
#define RFID_RST  9

MFRC522 rfid(RFID_SS, RFID_RST);

// Wi-Fi
char ssid[] = "RotiadorQuarto";
char pass[] = "169928263";

// LEDs da casa
#define LED_SALA       48
#define LED_QUARTO     47
#define LED_QUARTO2    38
#define LED_BANHEIRO   39
#define LED_COZINHA    40
#define LED_VARANDA    41

// Fotoresistor e LEDs dos postes
#define FOTORESISTOR  6
#define BUZZER 5
#define LED_POSTE1    17
#define LED_POSTE2    18

// Leitura alta no escuro e baixa com luz
const int LIMIAR_ESCURO = 700;
const int LIMIAR_CLARO  = 500;

const unsigned long INTERVALO_LEITURA = 1000;
unsigned long ultimaLeitura = 0;
bool posteAceso = false;

void setup() {
  Serial.begin(115200);
  analogReadResolution(12);

  SPI.begin(12, 13, 11, RFID_SS); // SCK, MISO, MOSI, SS
  rfid.PCD_Init();
  Serial.println("Aproxime um cartão RFID...");

  pinMode(BUZZER, OUTPUT);
  digitalWrite(BUZZER, LOW);

  pinMode(LED_SALA, OUTPUT);
  pinMode(LED_QUARTO, OUTPUT);
  pinMode(LED_QUARTO2, OUTPUT);
  pinMode(LED_BANHEIRO, OUTPUT);
  pinMode(LED_COZINHA, OUTPUT);
  pinMode(LED_VARANDA, OUTPUT);

  pinMode(FOTORESISTOR, INPUT);
  pinMode(LED_POSTE1, OUTPUT);
  pinMode(LED_POSTE2, OUTPUT);

  digitalWrite(LED_SALA, LOW);
  digitalWrite(LED_QUARTO, LOW);
  digitalWrite(LED_QUARTO2, LOW);
  digitalWrite(LED_BANHEIRO, LOW);
  digitalWrite(LED_COZINHA, LOW);
  digitalWrite(LED_VARANDA, LOW);

  digitalWrite(LED_POSTE1, LOW);
  digitalWrite(LED_POSTE2, LOW);

  Serial.println("Iniciando sistema inteligente...");
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
}

BLYNK_CONNECTED() {
  Blynk.syncVirtual(V4, V5, V6, V7, V8, V9);
  Serial.println("Blynk conectado!");
}

BLYNK_WRITE(V4) {
  digitalWrite(LED_SALA, param.asInt());
}

BLYNK_WRITE(V5) {
  digitalWrite(LED_QUARTO, param.asInt());
}

BLYNK_WRITE(V6) {
  digitalWrite(LED_QUARTO2, param.asInt());
}

BLYNK_WRITE(V7) {
  digitalWrite(LED_BANHEIRO, param.asInt());
}

BLYNK_WRITE(V8) {
  digitalWrite(LED_COZINHA, param.asInt());
}

BLYNK_WRITE(V9) {
  digitalWrite(LED_VARANDA, param.asInt());
}

void lerRFID() {
  if (!rfid.PICC_IsNewCardPresent()) {
    return;
  }

  if (!rfid.PICC_ReadCardSerial()) {
    return;
  }

  digitalWrite(BUZZER, HIGH);
  delay(100);
  digitalWrite(BUZZER, LOW);


  Serial.print("UID do cartão: ");

  for (byte i = 0; i < rfid.uid.size; i++) {
    if (rfid.uid.uidByte[i] < 0x10) {
      Serial.print("0");
    }
    Serial.print(rfid.uid.uidByte[i], HEX);
    Serial.print(" ");
  }

  Serial.println();

  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
}

void loop() {
  Blynk.run();
  lerRFID();

  // mantenha abaixo o restante do loop do fotoresistor


  unsigned long agora = millis();

  if (agora - ultimaLeitura >= INTERVALO_LEITURA) {
    ultimaLeitura = agora;

    int luz = analogRead(FOTORESISTOR);

    Serial.print("Leitura do fotoresistor: ");
    Serial.println(luz);

if (!posteAceso && luz < LIMIAR_ESCURO) {
  posteAceso = true;
  digitalWrite(LED_POSTE1, HIGH);
  digitalWrite(LED_POSTE2, HIGH);
  Serial.println("Escuro: postes ligados.");
} else if (posteAceso && luz > LIMIAR_CLARO) {
  posteAceso = false;
  digitalWrite(LED_POSTE1, LOW);
  digitalWrite(LED_POSTE2, LOW);
  Serial.println("Claro: postes desligados.");
}
  }
}


