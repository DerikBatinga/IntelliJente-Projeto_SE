// ============================================================================
//  PROJETO: IntelliJente: Automação & Segurança Residencial ESP32-S3
// ============================================================================

// ======================== BLYNK ========================
#define BLYNK_TEMPLATE_ID "TMPL2vCK3JGi6"
#define BLYNK_TEMPLATE_NAME "testLed"'
#define BLYNK_AUTH_TOKEN "OS4CMvnheFSqPny83QX7UUtTKAZldyrG"

// ======================== BIBLIOTECAS ========================
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <SPI.h>
#include <MFRC522.h>

// ======================== PINOS DOS LEDS ========================
// RECONEXOES FISICAS NECESSARIAS: quarto2 37->15, varanda 36->16,
// buzzer 39->42, poste 19->47. GPIO36/37 sao usados pela PSRAM.
#define LED_VERM 13
#define LED_VERD 14
#define LED_SALA 41
#define LED_QUARTO 40
#define LED_BANHEIRO 39
#define LED_COZINHA 38
#define LED_QUARTO2 15
#define LED_VARANDA 16
#define LED_POSTE 47
// ======================== SENSORES E ATUADORES ========================
#define SENSOR_MOV 7
#define BUZZER 42
#define SENSOR_RES 4

// ======================== RFID (SPI - PINOS CONFIGURADOS) ========================
#define RFID_SS 11
#define RFID_SCK 10
#define RFID_MOSI 9
#define RFID_MISO 46
#define RFID_RST 8

// ======================== WI-FI ========================
char ssid[] = "";
char pass[] = "";

// ======================== CONSTANTES E CREDENCIAIS DO ALARME ========================

const char UID_AUTORIZADO[] = "";.
const char SENHA_ALARME[] = "1234";
const int NIVEL_MOVIMENTO = HIGH;
const int LIGADO = HIGH;
const int DESLIGADO = LOW;
const int THRESHOLD = 10;
const unsigned long TEMPO_ALARME = 15000;  // Duração de cada disparo: 15000 milissegundos, equivalentes a 15 segundos.
const unsigned long TEMPO_REPOUSO = 1000;  // Exige 1000 milissegundos contínuos de repouso antes de aceitar movimento.
const unsigned long INTERVALO_REDE = 10000;  // Intervalo de 10000 milissegundos entre tentativas de conexão.

// ======================== MAPEAMENTO BLYNK E VARIÁVEIS GLOBAIS ========================
// Blynk: V0=PIR, V1=senha (String), V2=status do alarme (0/1).
// V3=sala, V4=quarto, V5=quarto2, V6=banheiro, V7=cozinha, V8=varanda.
const int PINOS_LUZES[] = {
  LED_SALA, LED_QUARTO, LED_QUARTO2, LED_BANHEIRO, LED_COZINHA, LED_VARANDA
const int VIRTUAIS_LUZES[] = {V3, V4, V5, V6, V7, V8};
bool estadosLuzes[6] = {};  // Guarda os seis estados das luzes, inicialmente todos falsos (apagados).
bool pirHabilitado = false;
bool aguardandoRepouso = true;
bool contandoRepouso = false;  // Indica se a contagem de repouso já começou.
bool movimentoAnterior = false;  // Guarda a leitura anterior para detectar a mudança de repouso para movimento.
bool alarmeAtivo = false;
bool posteLigado = false;
unsigned long inicioRepouso = 0;  // Instante em milissegundos em que começou o repouso do PIR.
unsigned long inicioAlarme = 0;  // Instante em milissegundos em que o alarme foi disparado.
unsigned long ultimaRede = 0;  // Instante da última tentativa de conexão com a rede ou com o Blynk.
unsigned long ultimoPrint = 0;  // Instante da última impressão periódica no Monitor Serial.
unsigned long ultimaLeituraLdr = 0;  // Instante da última leitura do fotoresistor.
int luminosidade = 0;
MFRC522 rfid(RFID_SS, RFID_RST);

// ======================== PROTÓTIPOS DAS FUNÇÕES ========================
void prepararRepouso();  // Declara a função que reinicia a espera de repouso do PIR.
void silenciarAlarme();  // Declara a função que desliga o buzzer e encerra o alarme.
void dispararAlarme(const char* evento);  // Declara a função que dispara o alarme e recebe o nome do evento Blynk.
void definirLuz(byte indice, int valor);  // Declara a função que liga ou desliga uma luz pelo índice na lista.
void processarSenha(const String& senha);  // Declara a função que verifica a senha recebida, sem copiar a String.
void manterConexao();  // Declara a função que mantém ou tenta recuperar as conexões.
void processarPIR(unsigned long agora, int nivelPIR);  // Declara a função que recebe o horário e a leitura do PIR para processá-los.
void processarRFID();  // Declara a função que lê o UID e verifica a autorização da tag.
void atualizarPoste(unsigned long agora);  // Declara a função que controla o poste com a leitura do LDR.
void atualizarLeds(unsigned long agora);  // Declara a função que atualiza os LEDs de status.
void imprimirEstado(unsigned long agora, int nivelPIR);  // Declara a função que imprime o estado do sistema periodicamente.

// ======================== SETUP - INICIALIZAÇÃO ========================
void setup() {
  Serial0.begin(115200);
  pinMode(SENSOR_MOV, INPUT);
  pinMode(SENSOR_RES, INPUT);
  analogReadResolution(12);

  const int saidas[] = {
    BUZZER, LED_VERM, LED_VERD, LED_POSTE, LED_SALA, LED_QUARTO,
    LED_QUARTO2, LED_BANHEIRO, LED_COZINHA, LED_VARANDA
  };  // Encerra a lista de saídas.
  for (int pino : saidas) {
    digitalWrite(pino, DESLIGADO);
    pinMode(pino, OUTPUT);
  }

  SPI.begin(RFID_SCK, RFID_MISO, RFID_MOSI, RFID_SS);
  rfid.PCD_Init();

  byte versao = rfid.PCD_ReadRegister(MFRC522::VersionReg);  // Lê o registro de versão do chip para diagnosticar a comunicação SPI.
  Serial0.print("Versao RFID: 0x");
  Serial0.println(versao, HEX);  // Imprime a versão em hexadecimal.
  if (versao == 0x00 || versao == 0xFF) {
    Serial0.println("RFID sem comunicacao: confira alimentacao e fios.");
  }
  if (UID_AUTORIZADO[0] == '\0') {
    Serial0.println("RFID em modo de leitura: configure UID_AUTORIZADO.");
  }

  Blynk.config(BLYNK_AUTH_TOKEN);
  WiFi.mode(WIFI_STA);  // Configura o ESP32 como cliente de uma rede Wi-Fi.
  WiFi.begin(ssid, pass);
  ultimaRede = millis() - INTERVALO_REDE;  // Permite uma tentativa imediata de conexão na primeira passagem pelo loop.
  Serial0.println("Sistema iniciado; conectando ao Wi-Fi em segundo plano.");
}  // Encerra a inicialização do sistema.

// ======================== LOOP PRINCIPAL ========================
void loop() {
  manterConexao();
  unsigned long agora = millis();
  int nivelPIR = digitalRead(SENSOR_MOV);

  processarPIR(agora, nivelPIR);
  processarRFID();
  atualizarPoste(agora);
  atualizarLeds(agora);
  imprimirEstado(agora, nivelPIR);
}

// ======================== CALLBACKS BLYNK ========================
BLYNK_CONNECTED() {  // Executa automaticamente quando a conexão com o Blynk é estabelecida.
  Blynk.virtualWrite(V0, pirHabilitado ? 1 : 0);
  Blynk.virtualWrite(V2, alarmeAtivo ? 1 : 0);
  for (byte i = 0; i < 6; i++) {
    Blynk.virtualWrite(VIRTUAIS_LUZES[i], estadosLuzes[i] ? 1 : 0);
  }
  Serial0.println("Blynk conectado.");
}

BLYNK_WRITE(V0) {
  bool novoEstado = param.asInt() == 1;
  if (novoEstado == pirHabilitado) return;
  pirHabilitado = novoEstado;
  prepararRepouso();  // Exige novo repouso antes da próxima detecção, sem cortar o alarme atual.
}

BLYNK_WRITE(V1) { processarSenha(String(param.asStr())); }  // Recebe a senha em V1 como texto e encaminha para validação.
BLYNK_WRITE(V3) { definirLuz(0, param.asInt()); }  // Recebe V3 e controla a sala, índice 0 da lista de luzes.
BLYNK_WRITE(V4) { definirLuz(1, param.asInt()); }  // Recebe V4 e controla o primeiro quarto
BLYNK_WRITE(V5) { definirLuz(2, param.asInt()); }  // Recebe V5 e controla o segundo quarto
BLYNK_WRITE(V6) { definirLuz(3, param.asInt()); }  // Recebe V6 e controla o banheiro
BLYNK_WRITE(V7) { definirLuz(4, param.asInt()); }  // Recebe V7 e controla a cozinha
BLYNK_WRITE(V8) { definirLuz(5, param.asInt()); }  // Recebe V8 e controla a varanda

// ======================== CONTROLE DE ILUMINAÇÃO DA CASA ========================
void definirLuz(byte indice, int valor) {
  if (indice >= 6) return;
  estadosLuzes[indice] = valor != 0;  // Guarda o novo estado: zero apaga e qualquer outro valor acende.
  digitalWrite(PINOS_LUZES[indice], estadosLuzes[indice] ? LIGADO : DESLIGADO);  // Aplica o estado salvo ao GPIO da luz escolhida
}

// ======================== CONTROLE DO ESTADO DO ALARME ========================
void prepararRepouso() {  // Reinicia as condições necessárias para aceitar um novo movimento.
  aguardandoRepouso = true;  // Ativa a exigência de repouso antes de detectar movimento.
  contandoRepouso = false;  // Descarta qualquer contagem de repouso anterior.
  movimentoAnterior = false;  // Reinicia o histórico usado para reconhecer uma nova transição.
}

void silenciarAlarme() {
  alarmeAtivo = false;
  digitalWrite(BUZZER, DESLIGADO);
  prepararRepouso();  // Exige repouso para não redisparar imediatamente com o PIR ainda alto.
  if (Blynk.connected()) Blynk.virtualWrite(V2, 0);
}

void dispararAlarme(const char* evento) {
  if (alarmeAtivo) return;
  alarmeAtivo = true;
  inicioAlarme = millis();
  digitalWrite(BUZZER, LIGADO);
  Serial0.println("Alarme acionado.");
  if (Blynk.connected()) {
    Blynk.virtualWrite(V2, 1);
    Blynk.logEvent(evento);  // Registra no Blynk o evento recebido pela função.
  }
}

// ======================== VALIDAÇÃO DE SENHA ========================
void processarSenha(const String &senha) {
  if (senha == SENHA_ALARME) {
    bool estavaAtivo = alarmeAtivo;
    silenciarAlarme();
    Serial0.println("Senha correta; alarme silenciado.");
    if (estavaAtivo && Blynk.connected()) Blynk.logEvent("senha_correta");
  } else {
    Serial0.println("Senha incorreta.");
    if (Blynk.connected()) Blynk.logEvent("senha_incorreta");
  }
}

// ======================== CONEXÃO WI-FI E BLYNK ========================
void manterConexao() {
  if (WiFi.status() == WL_CONNECTED && Blynk.connected()) {
    Blynk.run();
    return;
  }

  unsigned long agora = millis();
  if (agora - ultimaRede < INTERVALO_REDE) return;
  ultimaRede = agora;
  if (WiFi.status() != WL_CONNECTED) {
    WiFi.reconnect();
  } else {
    Blynk.connect(100);  // Solicita conexão Blynk com timeout de 100 ms; DNS/TCP podem demorar mais.
  }
}

// ======================== PROCESSAMENTO DO ALARME PIR ========================
void processarPIR(unsigned long agora, int nivelPIR) {
  if (alarmeAtivo && agora - inicioAlarme >= TEMPO_ALARME) {
    silenciarAlarme();
    Serial0.println("Ciclo de alarme encerrado.");
  }

  if (!pirHabilitado) return;
  bool movimento = nivelPIR == NIVEL_MOVIMENTO;
  if (aguardandoRepouso) {
    if (movimento) {
      contandoRepouso = false;
    } else if (!contandoRepouso) {
      contandoRepouso = true;
      inicioRepouso = agora;
    } else if (agora - inicioRepouso >= TEMPO_REPOUSO) {
      aguardandoRepouso = false;
      movimentoAnterior = false;
    }
    return;
  }

  if (movimento && !movimentoAnterior) dispararAlarme("movimento_detectado");
  movimentoAnterior = movimento;
}

// ======================== LEITURA E AUTORIZAÇÃO RFID ========================
void processarRFID() {
  if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial()) return;

  String uid;
  for (byte i = 0; i < rfid.uid.size; i++) {
    if (i > 0) uid += ' ';
    if (rfid.uid.uidByte[i] < 0x10) uid += '0';
    uid += String(rfid.uid.uidByte[i], HEX);
  }
  uid.toUpperCase();
  Serial0.print("UID TAG: [");
  Serial0.print(uid);
  Serial0.println("]");
  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();

  if (UID_AUTORIZADO[0] == '\0') return;
  String autorizado(UID_AUTORIZADO);
  autorizado.trim();
  autorizado.toUpperCase();
  if (uid == autorizado) {
    bool estavaAtivo = alarmeAtivo;
    silenciarAlarme();
    Serial0.println("Tag autorizada; alarme silenciado.");
    if (estavaAtivo && Blynk.connected()) Blynk.logEvent("alarme_desligado");
  } else {
    Serial0.println("Tag nao autorizada.");
    dispararAlarme("alarme_acionado");
  }
}

// ======================== CONTROLE AUTOMÁTICO DO POSTE ========================
void atualizarPoste(unsigned long agora) {
  if (agora - ultimaLeituraLdr < 200) return;
  ultimaLeituraLdr = agora;
  luminosidade = analogRead(SENSOR_RES);
  // Assume leitura menor no escuro, conforme o codigo original.
  if (luminosidade < LIMIAR_ESCURO) posteLigado = true;
  else if (luminosidade > LIMIAR_CLARO) posteLigado = false;
  digitalWrite(LED_POSTE, posteLigado ? LIGADO : DESLIGADO);
}  // Encerra o controle automático do poste.

// ======================== STATUS DOS LEDS ========================
void atualizarLeds(unsigned long agora) {
  digitalWrite(LED_VERM, alarmeAtivo ? LIGADO : DESLIGADO);
  bool verde = pirHabilitado && (!aguardandoRepouso || (agora / 500) % 2);
  digitalWrite(LED_VERD, verde ? LIGADO : DESLIGADO);
}  // Encerra a atualização dos LEDs de status.

// ======================== DEBUG - MONITOR SERIAL ========================
void imprimirEstado(unsigned long agora, int nivelPIR) {
  if (agora - ultimoPrint < 1000) return;
  ultimoPrint = agora;
  Serial0.print("PIR: "); Serial0.print(nivelPIR);
  Serial0.print(" | Habilitado: "); Serial0.print(pirHabilitado);
  Serial0.print(" | Aguarda repouso: "); Serial0.print(aguardandoRepouso);
  Serial0.print(" | Alarme: "); Serial0.print(alarmeAtivo);
  Serial0.print(" | LDR: "); Serial0.print(luminosidade);
  Serial0.print(" | Poste: "); Serial0.print(posteLigado);
  Serial0.print(" | Blynk: "); Serial0.println(Blynk.connected());
}