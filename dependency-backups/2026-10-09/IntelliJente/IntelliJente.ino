
// ===================== CREDENCIAIS Blynk ========================

#define BLYNK_TEMPLATE_ID   "TMPL2vCK3JGi6"                  // ID do template criado no blynk.cloud (identifica o modelo do dispositivo)
#define BLYNK_TEMPLATE_NAME "testLed"                        // nome do template; tem que ser EXATAMENTE igual ao cadastrado no site
#define BLYNK_AUTH_TOKEN    "SEU_TOKEN_AQUI"                 // token �nico do device: a senha que liga este codigo a este dispositivo na nuvem

// ===================== CREDENCIAIS DO RFID ========================

#define SS_PIN  40
#define RST_PIN 17

MFRC522 mfrc522(SS_PIN, RST_PIN);

// ===================== HARDWARE / VARIAVEIS ========================

#define LED_VERM     48               // GPIO do LED RGB embutido (WS2812); se nao acender, troque para 38
#define LED_VERD     47

#define LED_SALA     46
#define LED_QUARTO   45
#define LED_BANHEIRO 44
#define LED_COZINHA  43
#define LED_QUARTO2  42

#define LED_POSTE  41

#define BUZZER       10

// ================== BIBLIOTECAS (codigos prontos) ==================

#include <WiFi.h>             // funcoes de Wi-Fi do ESP32 (conectar, status da rede...)
#include <WiFiClient.h>       // cliente TCP de rede, necessario para o Blynk abrir conexao com o servidor
#include <BlynkSimpleEsp32.h> // biblioteca Blynk p/ ESP32: traz Blynk.begin(), Blynk.run(), BLYNK_WRITE() e virtualWrite()
#include <SPI.h>              // biblioteca responsavel pela comunicacao SPI
#include <MFRC522.h>          // biblioteca responsavel pelo leitor RFID

// ============== DADOS DA REDE WI-FI (so 2.4 GHz!) ==================

char ssid[] = "SEU_WIFI";     // nome da rede Wi-Fi onde a placa vai conectar
char pass[] = "SUA_SENHA";    // senha dessa rede

// ===================== HARDWARE / VARIAVEIS ========================

const int sensor_pin = A0;
int sensor;
const int threshold = 500;
bool estadoLED = false;       // estado atual: false = desligado | true = ligado
bool alarmeEst = false;       // estado atual do alarme

// ========================= RFID AUTORIZADO =========================

String UID_AUTORIZADO = "UID_ DO_RFID";

// ===================== FUNCAO PARA APLICAR ESTADO LED ========================
// Parametro espelhaNoApp: se true, envia o estado pro V0 na nuvem,
// fazendo o botao do app/navegador se atualizar sozinho.

void aplicarEstado(bool espelhaNoApp) {

  if (estadoLED) {
    // WS2812 n�o aceita digitalWrite!
    // Acende VERDE (R=0, G=80, B=0), brilho 80/255
    neopixelWrite(LED_VERM, 0, 80, 0);

  } else {
    // Todos os canais em zero = LED apagado
    neopixelWrite(LED_VERM, 0, 0, 0);
  }

  if (espelhaNoApp) {
    // Escreve o valor (0 ou 1) no pino virtual V0 la na nuvem
    Blynk.virtualWrite(V0, estadoLED);
  }


  Serial.println(
    estadoLED
      ? "ALARME ACIONADO"
      : "ALARME DESLIGADO"
  );
}

// ===================== CALLBACK AUTOMATICA DO BLYNK ========================
// Roda SOZINHA toda vez que alguem mudar o V0
// (bot�o apertado no app ou no dashboard web do navegador)

BLYNK_WRITE(V0) {

  estadoLED = param.asInt();   // param = valor que chegou da nuvem; asInt() converte pra numero (0 ou 1)

  aplicarEstado(false);        // aplica no LED; false = nao reenvia pro app (quem chamou ja esta atualizado)
}

// ===================== FUNCAO ALARME ========================
void alarme(bool verificacao) {

  if (verificacao) {
    // Ativa o buzzer
    tone(BUZZER, 300, 100);

  } else {
    // Desativa o buzzer
    noTone(BUZZER);
  }
}


// ===================== FUNCAO PARA LER RFID ========================
void RFID() {

  String RFID_UD = "";

  Serial.println("UID TAG:");

  // Percorre todos os bytes do UID da tag
  for (byte i = 0; i < mfrc522.uid.size; i++) {

    RFID_UD.concat(
      mfrc522.uid.uidByte[i] < 0x10
        ? " 0"
        : " "
    );

    RFID_UD.concat(
      String(mfrc522.uid.uidByte[i], HEX)
    );
  }

  // Converte o UID para letras mai�sculas
  RFID_UD.toUpperCase();


  Serial.println(RFID_UD);

// ===================== VERIFICA SE A TAG E AUTORIZADA ========================
  if (RFID_UD.equals(UID_AUTORIZADO)) {

    Serial.println("Tag autorizada!");
    Serial.println("Alarme desligado...");

    neopixelWrite(LED_VERD, 0, 80, 0);

    // Desliga o alarme
    alarmeEst = false;

    alarme(false);

    // Atualiza o estado do LED
    estadoLED = false;

    // Atualiza o Blynk
    aplicarEstado(true);
  }

  else {

    Serial.println("Tag n�o autorizada!");
    Serial.println("Alarme acionado...");

    // Acende o LED vermelho
    neopixelWrite(LED_VERM, 80, 0, 0);

    // Liga o alarme
    alarmeEst = true;

    alarme(true);

    // Atualiza o estado do LED
    estadoLED = true;

    // Atualiza o Blynk
    aplicarEstado(true);
  }

  // Finaliza a comunicacao com a tag
  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();
}

// ===================== SETUP() RODA UMA VEZ QUANDO A PLACA LIGA OU REINICIA ========================
void setup() {

  Serial.begin(115200);                        // inicia o monitor serial a 115200 bits/s (tem que bater com a velocidade configurada no IDE)
  Serial.println("\nConectando ao WiFi...");   // imprime mensagem (\n pula uma linha antes)

// ===================== INICIACAO DO BLYNK ========================
  Blynk.begin(
    BLYNK_AUTH_TOKEN,
    ssid,
    pass
  );                                           // conecta Wi-Fi + servidor Blynk; fica preso aqui ate conseguir conectar

  SPI.begin();

  mfrc522.PCD_Init();

  Serial.println("RFID iniciado!");
  Serial.println("Aproxime uma tag...");

 // GPIO do LED RGB embutido (WS2812); se nao acender, troque para 38
  pinMode(LED_VERM, OUTPUT);
  pinMode(LED_VERD, OUTPUT);
  pinMode(LED_SALA , OUTPUT);
  pinMode(LED_QUARTO, OUTPUT);
  pinMode(LED_BANHEIRO, OUTPUT);
  pinMode(LED_COZINHA, OUTPUT);
  pinMode(LED_QUARTO2, OUTPUT);
  pinMode(LED_POSTE, OUTPUT);

}

// ===================== LOOP() RODA EM REPETICAO INFINITA APOS O SETUP() ========================
void loop() {

  Blynk.run();  // mantem a conexao viva e processa comandos da nuvem (Ele quem dispara o BLYNK_WRITE quando algo muda no V0)


// ===================== SENSOR DE ILUMINOSIDADE (PHOTORESISTOR) ========================
sensor = analogRead(sensor_pin);
Serial.println(sensor);
if(sensor<threshold){
  digitalWrite(LED_POSTE,HIGH);
}
else{
  digitalWrite(LED_POSTE,LOW);
}
// ===================== VERIFICA SE UMA NOVA TAG FOI DETECTADA ========================
  if (!mfrc522.PICC_IsNewCardPresent()) {
    return;
  }

  if (!mfrc522.PICC_ReadCardSerial()) {
    return;
  }
  RFID();
}