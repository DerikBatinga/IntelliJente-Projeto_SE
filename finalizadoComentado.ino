// #define cria uma macro do pré-processador. As referências a BLYNK_TEMPLATE_ID passam a usar o texto entre aspas, que identifica o template no Blynk.
#define BLYNK_TEMPLATE_ID "TMPL2vCK3JGi6"
// BLYNK_TEMPLATE_NAME representa o nome do template. As aspas duplas delimitam a sequência de caracteres usada como valor da macro.
#define BLYNK_TEMPLATE_NAME "testLed"
// BLYNK_AUTH_TOKEN representa a credencial que identifica e autentica este dispositivo no Blynk. Ela é passada a Blynk.begin no setup.
#define BLYNK_AUTH_TOKEN "OS4CMvnheFSqPny83QX7UUtTKAZldyrG"

// #include incorpora as declarações do cabeçalho WiFi.h, que fornece a interface Wi-Fi do ESP32. Os sinais < e > indicam a busca pelo cabeçalho nos caminhos de inclusão.
#include <WiFi.h>
// Inclui WiFiClient.h, que declara WiFiClient, a classe cliente TCP usada para trocar dados pela rede Wi-Fi.
#include <WiFiClient.h>
// Inclui a integração do Blynk para ESP32, que fornece o objeto Blynk e as macros usadas para reagir a conexões e mensagens dos pinos virtuais.
#include <BlynkSimpleEsp32.h>
// Inclui a interface do barramento SPI. O sketch usa o objeto SPI para configurar a comunicação com o leitor RFID.
#include <SPI.h>
// Inclui a biblioteca do chip MFRC522. Ela fornece a classe do leitor e os métodos que inicializam o módulo, detectam cartões e leem o UID.
#include <MFRC522.h>

// Define RFID_SS como GPIO 10. SS é o sinal de seleção do leitor no barramento SPI.
#define RFID_SS   10
// Define RFID_RST como GPIO 9. O leitor usa esse sinal para reset.
#define RFID_RST  9

// Cria o objeto global rfid, do tipo MFRC522. Os parênteses passam ao construtor os pinos de seleção e reset, nessa ordem. O ponto e vírgula encerra a declaração.
MFRC522 rfid(RFID_SS, RFID_RST);

// O comentário original marca o grupo das credenciais Wi-Fi. As linhas iniciadas com // são ignoradas pelo compilador.
// Wi-Fi
// char ssid[] declara um vetor de caracteres. O texto à direita do = inicializa o nome da rede. Os colchetes vazios permitem inferir o tamanho, incluindo o caractere nulo final.
char ssid[] = "RotiadorQuarto";
// char pass[] declara outro vetor de caracteres, inicializado com a senha Wi-Fi. Blynk.begin recebe esse vetor junto com ssid.
char pass[] = "169928263";

// O comentário original agrupa as definições dos GPIOs dos cômodos e da varanda. Cada número identifica um pino físico do ESP32 pela numeração GPIO.
// LEDs da casa
// Define LED_SALA como GPIO 48, usado como saída digital para a luz da sala.
#define LED_SALA       48
// Define LED_QUARTO como GPIO 47, usado como saída digital para a luz do primeiro quarto.
#define LED_QUARTO     47
// Define LED_QUARTO2 como GPIO 38, usado como saída digital para a luz do segundo quarto.
#define LED_QUARTO2    38
// Define LED_BANHEIRO como GPIO 39, usado como saída digital para a luz do banheiro.
#define LED_BANHEIRO   39
// Define LED_COZINHA como GPIO 40, usado como saída digital para a luz da cozinha.
#define LED_COZINHA    40
// Define LED_VARANDA como GPIO 41, usado como saída digital para a luz da varanda.
#define LED_VARANDA    41

// O comentário original marca o grupo com a entrada do fotoresistor e as saídas dos postes. A definição do buzzer também aparece nesse grupo.
// Fotoresistor e LEDs dos postes
// Define FOTORESISTOR como GPIO 6. O programa lê nesse pino o sinal analógico do circuito com o fotoresistor.
#define FOTORESISTOR  6
// Define BUZZER como GPIO 5. O programa aplica um pulso nesse pino após ler com sucesso um cartão RFID.
#define BUZZER 5
// Define LED_POSTE1 como GPIO 17, a saída do primeiro poste.
#define LED_POSTE1    17
// Define LED_POSTE2 como GPIO 18, a saída do segundo poste.
#define LED_POSTE2    18

// O comentário original descreve uma leitura que cresce no escuro. No loop, as comparações associam valores abaixo de 700 a ligar e valores acima de 500 a desligar, conforme o estado dos postes.
// Leitura alta no escuro e baixa com luz
// const impede alteração após a inicialização e int é um tipo inteiro com sinal. O valor 700 é o limite exclusivo superior para ligar os postes quando estão apagados.
const int LIMIAR_ESCURO = 700;
// Cria a constante inteira LIMIAR_CLARO com valor 500. Uma leitura estritamente maior pode desligar os postes quando estão acesos.
const int LIMIAR_CLARO  = 500;

// Cria uma constante inteira sem sinal, do tipo unsigned long, com valor 1000. Como millis() mede milissegundos, esse intervalo equivale a um segundo.
const unsigned long INTERVALO_LEITURA = 1000;
// Cria a variável global ultimaLeitura, do tipo unsigned long. O zero inicial serve de referência para contar o tempo até a primeira leitura do fotoresistor.
unsigned long ultimaLeitura = 0;
// Cria o booleano posteAceso, inicialmente false, para guardar o estado lógico dos dois postes. As saídas físicas são configuradas no setup.
bool posteAceso = false;

// Define setup, chamada uma vez pelo Arduino na inicialização. void indica ausência de valor de retorno, () indica ausência de parâmetros e { abre o corpo da função.
void setup() {
  // O ponto acessa o método begin de Serial. O argumento 115200 configura a taxa serial em bauds quando essa porta usa UART. A chamada prepara as mensagens de diagnóstico.
  Serial.begin(115200);
  // Configura analogRead para devolver leituras de 12 bits. A faixa de valores brutos é de 0 a 4095, correspondente a 2 elevado a 12 possibilidades.
  analogReadResolution(12);

  // Inicializa o SPI com SCK no GPIO 12 para o relógio, MISO no 13 para dados do leitor ao ESP32, MOSI no 11 para dados do ESP32 ao leitor e SS no GPIO 10 para seleção.
  // As vírgulas separam os quatro argumentos. O comentário após // registra a ordem deles.
  SPI.begin(12, 13, 11, RFID_SS); // SCK, MISO, MOSI, SS
  // Chama PCD_Init no objeto rfid para inicializar o circuito leitor MFRC522 após a configuração do SPI. PCD identifica o leitor na nomenclatura da biblioteca.
  rfid.PCD_Init();
  // println recebe o texto entre aspas, envia-o à porta Serial e encerra a linha. A mensagem orienta a aproximar um cartão durante o uso.
  Serial.println("Aproxime um cartão RFID...");

  // pinMode recebe o GPIO do buzzer e OUTPUT, que configura esse pino para emitir um nível digital.
  pinMode(BUZZER, OUTPUT);
  // digitalWrite recebe o GPIO do buzzer e LOW, que aplica nível lógico baixo para iniciar com o buzzer desativado.
  digitalWrite(BUZZER, LOW);

  // Configura o GPIO 48, associado a LED_SALA, como saída digital pelo segundo argumento OUTPUT.
  pinMode(LED_SALA, OUTPUT);
  // Configura o GPIO 47, associado a LED_QUARTO, como saída digital.
  pinMode(LED_QUARTO, OUTPUT);
  // Configura o GPIO 38, associado a LED_QUARTO2, como saída digital.
  pinMode(LED_QUARTO2, OUTPUT);
  // Configura o GPIO 39, associado a LED_BANHEIRO, como saída digital.
  pinMode(LED_BANHEIRO, OUTPUT);
  // Configura o GPIO 40, associado a LED_COZINHA, como saída digital.
  pinMode(LED_COZINHA, OUTPUT);
  // Configura o GPIO 41, associado a LED_VARANDA, como saída digital.
  pinMode(LED_VARANDA, OUTPUT);

  // pinMode configura o GPIO do fotoresistor como entrada com INPUT. A conversão analógica desse sinal acontece mais tarde em analogRead.
  pinMode(FOTORESISTOR, INPUT);
  // Configura o GPIO 17, associado a LED_POSTE1, como saída digital.
  pinMode(LED_POSTE1, OUTPUT);
  // Configura o GPIO 18, associado a LED_POSTE2, como saída digital.
  pinMode(LED_POSTE2, OUTPUT);

  // Aplica LOW ao GPIO da sala para iniciar com essa luz apagada.
  digitalWrite(LED_SALA, LOW);
  // Aplica LOW ao GPIO do primeiro quarto para iniciar com essa luz apagada.
  digitalWrite(LED_QUARTO, LOW);
  // Aplica LOW ao GPIO do segundo quarto para iniciar com essa luz apagada.
  digitalWrite(LED_QUARTO2, LOW);
  // Aplica LOW ao GPIO do banheiro para iniciar com essa luz apagada.
  digitalWrite(LED_BANHEIRO, LOW);
  // Aplica LOW ao GPIO da cozinha para iniciar com essa luz apagada.
  digitalWrite(LED_COZINHA, LOW);
  // Aplica LOW ao GPIO da varanda para iniciar com essa luz apagada.
  digitalWrite(LED_VARANDA, LOW);

  // Aplica LOW ao GPIO do primeiro poste para iniciar com seu LED apagado.
  digitalWrite(LED_POSTE1, LOW);
  // Aplica LOW ao GPIO do segundo poste para iniciar com seu LED apagado.
  digitalWrite(LED_POSTE2, LOW);

  // Envia uma mensagem de início do sistema pela Serial e acrescenta uma quebra de linha.
  Serial.println("Iniciando sistema inteligente...");
  // Chama o método begin de Blynk com o token do dispositivo, o nome da rede Wi-Fi e a senha, nessa ordem. A biblioteca usa esses dados para conectar o ESP32 à rede e ao Blynk.
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
// A chave fecha o corpo de setup e encerra a sequência de instruções de inicialização.
}

// BLYNK_CONNECTED() é uma macro que declara a função chamada pela biblioteca ao estabelecer a conexão com o Blynk. A chave abre o corpo dessa função.
BLYNK_CONNECTED() {
  // syncVirtual solicita ao servidor os valores salvos dos seis pinos virtuais V4 a V9. Quando chegam, cada valor é entregue ao BLYNK_WRITE correspondente.
  // V4, V5, V6, V7, V8 e V9 identificam canais de dados do Blynk. Cada um é associado a um GPIO físico pela função que recebe seu valor.
  Blynk.syncVirtual(V4, V5, V6, V7, V8, V9);
  // Imprime a confirmação de conexão com o Blynk e termina a linha na Serial.
  Serial.println("Blynk conectado!");
// Fecha o bloco executado ao conectar ao Blynk.
}

// BLYNK_WRITE(V4) declara a função que roda quando chega um valor para o pino virtual V4. Neste sketch, V4 está associado à sala. A chave abre o bloco de atendimento.
BLYNK_WRITE(V4) {
  // param é o valor recebido do Blynk e asInt() o converte em inteiro. O resultado vira o segundo argumento de digitalWrite para aplicar 0 (LOW) ou 1 (HIGH) no GPIO da sala.
  digitalWrite(LED_SALA, param.asInt());
// Fecha o bloco que atende V4.
}

// BLYNK_WRITE(V5) abre a função que atende valores enviados ao pino virtual V5, associado ao primeiro quarto.
BLYNK_WRITE(V5) {
  // Converte o valor recebido em inteiro com param.asInt() e o usa como nível digital da luz do primeiro quarto. Os valores esperados são 0 para apagar e 1 para acender.
  digitalWrite(LED_QUARTO, param.asInt());
// Fecha o bloco que atende V5.
}

// BLYNK_WRITE(V6) abre a função que atende o pino virtual V6, associado ao segundo quarto.
BLYNK_WRITE(V6) {
  // Converte o valor recebido em inteiro com param.asInt() e o aplica ao GPIO do segundo quarto. O nível esperado é LOW para 0 ou HIGH para 1.
  digitalWrite(LED_QUARTO2, param.asInt());
// Fecha o bloco que atende V6.
}

// BLYNK_WRITE(V7) abre a função que atende o pino virtual V7, associado ao banheiro.
BLYNK_WRITE(V7) {
  // Converte o valor recebido em inteiro com param.asInt() e escreve esse nível no GPIO do banheiro. O valor 0 apaga a luz e 1 a acende.
  digitalWrite(LED_BANHEIRO, param.asInt());
// Fecha o bloco que atende V7.
}

// BLYNK_WRITE(V8) abre a função que atende o pino virtual V8, associado à cozinha.
BLYNK_WRITE(V8) {
  // Converte o valor recebido em inteiro com param.asInt() e o aplica ao GPIO da cozinha. O valor 0 apaga a luz e 1 a acende.
  digitalWrite(LED_COZINHA, param.asInt());
// Fecha o bloco que atende V8.
}

// BLYNK_WRITE(V9) abre a função que atende o pino virtual V9, associado à varanda.
BLYNK_WRITE(V9) {
  // Converte o valor recebido em inteiro com param.asInt() e o aplica ao GPIO da varanda. O valor 0 apaga a luz e 1 a acende.
  digitalWrite(LED_VARANDA, param.asInt());
// Fecha o bloco que atende V9.
}

// Define lerRFID, uma função sem argumentos e sem valor de retorno. Seu corpo tenta detectar um cartão, ler o UID e sinalizar uma leitura bem-sucedida.
void lerRFID() {
  // if testa uma condição. PICC_IsNewCardPresent() consulta o leitor sobre um novo cartão e ! inverte o resultado. A chave abre o bloco executado se a função retornar falso.
  if (!rfid.PICC_IsNewCardPresent()) {
    // return encerra esta chamada de lerRFID. O programa volta ao ponto seguinte à chamada no loop, pulando o pulso do buzzer e a impressão do UID nesta passagem.
    return;
  // Fecha o bloco executado quando a sondagem retorna falso. Com uma detecção bem-sucedida, a execução chega ao teste de leitura seguinte.
  }

  // PICC_ReadCardSerial() tenta selecionar o cartão e carregar seus dados de identificação em rfid.uid. ! inverte o resultado e o if entra no bloco se a leitura falhar.
  if (!rfid.PICC_ReadCardSerial()) {
    // Sai de lerRFID sem valor de retorno quando a leitura do identificador falha.
    return;
  // Fecha o bloco de falha na leitura. As próximas instruções da função são alcançadas quando a presença e a leitura têm sucesso.
  }

  // Aplica HIGH, nível lógico alto, ao GPIO do buzzer para começar o pulso sonoro.
  digitalWrite(BUZZER, HIGH);
  // delay(100) espera 100 milissegundos antes da próxima instrução desta função. O buzzer permanece com a saída em HIGH durante essa espera.
  delay(100);
  // Volta a saída do buzzer para LOW para encerrar o pulso.
  digitalWrite(BUZZER, LOW);


  // Serial.print escreve o texto que antecede o UID. A chamada deixa a linha aberta para que os bytes sejam impressos em seguida.
  Serial.print("UID do cartão: ");

  // O for tem três partes separadas por ponto e vírgula. byte i = 0 cria o índice de 8 bits sem sinal e começa na posição zero. i < rfid.uid.size mantém o laço enquanto houver bytes.
  // Os pontos em rfid.uid.size acessam o tamanho do UID do leitor. i++ acrescenta uma unidade ao índice ao fim de cada passagem e { abre o corpo do laço.
  for (byte i = 0; i < rfid.uid.size; i++) {
    // rfid.uid.uidByte[i] acessa o byte na posição i do vetor do UID. O operador < compara esse byte a 0x10, que vale 16 em decimal. Valores de 0 a 15 têm um único dígito em hexadecimal.
    if (rfid.uid.uidByte[i] < 0x10) {
      // Imprime o caractere zero à esquerda para que um byte de apenas um dígito hexadecimal fique com dois dígitos. print mantém a mesma linha.
      Serial.print("0");
    // Fecha o if que acrescenta o zero. A impressão do byte vem a seguir para qualquer valor.
    }
    // Imprime o byte na posição i. HEX é o segundo argumento e seleciona a base 16 para formatar o número.
    Serial.print(rfid.uid.uidByte[i], HEX);
    // Imprime um espaço depois do byte atual. Isso separa os bytes e também deixa um espaço após o último byte.
    Serial.print(" ");
  // Fecha o corpo do for. O índice avança, a condição é testada de novo e o laço termina quando i alcança rfid.uid.size.
  }

  // Chama println com os parênteses vazios para encerrar a linha do UID depois que todos os bytes foram impressos.
  Serial.println();

  // PICC_HaltA() envia ao cartão selecionado o comando de parada do protocolo. PICC identifica o cartão na nomenclatura da biblioteca.
  rfid.PICC_HaltA();
  // PCD_StopCrypto1() desativa o modo Crypto1 no leitor, encerrando esse estado de comunicação antes da próxima leitura.
  rfid.PCD_StopCrypto1();
// Fecha o corpo de lerRFID. O controle volta ao ponto em que a função foi chamada.
}

// Define loop como uma função sem parâmetros e sem valor de retorno. O Arduino chama essa função repetidamente após setup. A chave abre o corpo da função.
void loop() {
  // Blynk.run() processa a comunicação com o servidor, incluindo mensagens que acionam as funções dos pinos virtuais. A chamada ocorre a cada passagem pelo loop.
  Blynk.run();
  // Chama lerRFID para verificar se há um cartão. Se a leitura for bem-sucedida, a função pulsa o buzzer e imprime o UID. Ao voltar, o loop segue para o controle dos postes.
  lerRFID();

  // O comentário original marca a posição da lógica do fotoresistor dentro de loop. O texto após // serve como anotação no código.
  // mantenha abaixo o restante do loop do fotoresistor


  // millis() devolve o tempo em milissegundos desde o início do programa. A expressão inicializa agora como unsigned long, reutilizado nos testes de tempo desta passagem.
  unsigned long agora = millis();

  // Subtrai o instante da última leitura do instante atual. >= significa maior ou igual. Se tiver passado pelo menos INTERVALO_LEITURA, que vale 1000 ms, o bloco executa.
  // A subtração entre inteiros sem sinal também permite medir esse intervalo quando o contador de millis() volta a zero, desde que o tempo decorrido caiba nesse tipo.
  if (agora - ultimaLeitura >= INTERVALO_LEITURA) {
    // O operador = guarda o horário atual como instante da última leitura. A próxima comparação mede o tempo a partir daqui.
    ultimaLeitura = agora;

    // analogRead converte em um número o sinal no GPIO do fotoresistor. O valor bruto retornado é guardado na variável inteira local luz.
    int luz = analogRead(FOTORESISTOR);

    // print envia ao Monitor Serial o texto que precede o número da leitura e mantém a linha aberta.
    Serial.print("Leitura do fotoresistor: ");
    // println imprime o valor inteiro de luz depois do texto anterior e então encerra a linha.
    Serial.println(luz);

// !posteAceso verifica se os postes estão apagados. && exige que essa condição e luz < LIMIAR_ESCURO sejam verdadeiras. Com o estado apagado, o bloco liga os postes se luz for menor que 700.
if (!posteAceso && luz < LIMIAR_ESCURO) {
  // true é o valor booleano verdadeiro. A atribuição marca os dois postes como acesos antes de alterar as respectivas saídas.
  posteAceso = true;
  // Aplica HIGH ao GPIO do primeiro poste para acender seu LED.
  digitalWrite(LED_POSTE1, HIGH);
  // Aplica HIGH ao GPIO do segundo poste para acender seu LED.
  digitalWrite(LED_POSTE2, HIGH);
  // Registra na Serial que a condição de ligar ocorreu e encerra a linha.
  Serial.println("Escuro: postes ligados.");
// A primeira chave fecha o bloco que liga os postes. else if testa esta segunda condição quando o primeiro if falha. && exige postes acesos e leitura estritamente maior que 500.
// As faixas se sobrepõem entre 501 e 699. Mantida uma leitura nesse intervalo, o estado alterna a cada leitura, pois uma condição manda ligar quando está apagado e a outra manda desligar quando está aceso.
} else if (posteAceso && luz > LIMIAR_CLARO) {
  // Atribui false à variável posteAceso para marcar os postes como apagados.
  posteAceso = false;
  // Aplica LOW ao GPIO do primeiro poste para apagar seu LED.
  digitalWrite(LED_POSTE1, LOW);
  // Aplica LOW ao GPIO do segundo poste para apagar seu LED.
  digitalWrite(LED_POSTE2, LOW);
  // Registra na Serial que a condição de desligar ocorreu e encerra a linha.
  Serial.println("Claro: postes desligados.");
// Fecha o bloco do else if. Se os dois testes forem falsos, nenhuma saída dos postes é alterada nesta leitura.
}
  // Fecha o if que controla o intervalo do fotoresistor. Enquanto o intervalo estiver abaixo de 1000 ms, a leitura, os registros na Serial e o controle dos postes dentro desse bloco são pulados.
  }
// Fecha o corpo de loop. Na próxima chamada, a execução começa de novo em Blynk.run().
}


