/*
 * ----------------------------------------------------------------------
 *  Arquivo   : poliquest_s3_1.1_32.ino
 *  Projeto   : PoliQuest
 * ----------------------------------------------------------------------
 *  Descricao : código reponsável por cominucar o FPGA com o servidor
 *              web por meio do ESP32. Inpirado no vídeo do canal
 *              Bobsien "ESP8266 - envio e recebimento de dados via MQTT"
 * ----------------------------------------------------------------------
 *  Revisoes  :
 *      Data        Versao  Autor             Descricao
 *      28/03/2024  1.0     Vitor Thompson    versao inicial
 *      31/03/2024  1.1     Vitor Thompson    adição dos botões
 * ----------------------------------------------------------------------
 */

// biblicoteca para a conexão com wifi
#include <WiFi.h>

// biblioteca para utilizar o MQTT Broker, talvez seja necessário instalá-la antes
#include <PubSubClient.h>

// definição das portas para placa ESP32 WROOM
// saídas:
#define D23 23 // payload[1] reset   
#define D22 22 // payload[2] iniciarJogo
#define D21 21 // payload[3] jogada[0] 
#define D19 19 // payload[4] jogada[1]  
#define D18 18 // payload[5] jogada[2]
#define D5  5  // payload[6] jogada[3]
#define TX2 17 // payload[7] confimaJog 
#define RX2 16 // payload[8] resetJog    
#define D4  4  // payload[9] sairJogo  

// entradas:
#define D15 15 // payload[10] ganhou
#define D2  2  // payload[11] perdeu  
#define D25 25 // payload[12] botaoEsq
#define D26 26 // payload[13] botaoDir
#define D27 27 // payload[14] botaoSelect

/*
 * {100000000000000} - reset      :  D23
 * {010000000000000} - iniciarJogo:  D22
 * {001000000000000} - jogada[0]  :  D21
 * {000100000000000} - jogada[1]  :  D19
 * {000010000000000} - jogada[2]  :  D18
 * {000001000000000} - jogada[3]  :  D5
 * {000000100000000} - confimaJog :  TX2 
 * {000000010000000} - resetJog   :  RX2 
 * {000000001000000} - sairJogo   :  D4 
 */

// parametros para conexão com o WiFi
const char * ssid     = "Apt 164";    // nome da rede
const char * password = "20220104fm"; // senha

// parametros para conexão com o MQTT Broker
const char * mqtt_broker   = "test.mosquitto.org"; // Host do broker
const char * topic         = "poliQuestPortasESP"; 
const char * mqtt_username = "";
const char * mqtt_password = "";
const int    mqtt_port     = 1883;

// variáveis para contole do tópico
bool mqttStatus = false;

// Objetos
WiFiClient espClient;
PubSubClient client(espClient);

// Prototipos das funções para utilizar o MQTT
bool connectMQTT ();                                               // sera utilizada para fazera conexão com o broker    
void callback (char * topic, byte * payload, unsigned int length); // sera chamada toda vez que houver alteração no tópico

 void setup () {
  // inicializa serial
  Serial.begin(9600);

  // definição das portas
  // saídas:
  pinMode(D23, OUTPUT);
  pinMode(D22, OUTPUT);
  pinMode(D21, OUTPUT);
  pinMode(D19, OUTPUT);
  pinMode(D18, OUTPUT);
  pinMode(TX2, OUTPUT);
  pinMode(RX2, OUTPUT);
  pinMode(D5, OUTPUT);
  pinMode(D4, OUTPUT);
  
  // entradas:
  pinMode(D27, INPUT);
  pinMode(D26, INPUT);
  pinMode(D25, INPUT);
  pinMode(D15, INPUT);
  pinMode(D2, INPUT);
 
  // inicializa a conexão com a rede WiFi
  WiFi.begin(ssid, password);

  // aguarda até realizar a conexão
  Serial.println("\nConectando");
  while (WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.println(".");
  }
  Serial.println("\nWiFi conectado!");

  // Envia IP através da UART
  Serial.println(WiFi.localIP());

  mqttStatus = connectMQTT();
 }

bool jaGanhou = false;
bool jaPerdeu = false;
bool jaClicou = false;

void loop () {
  if (mqttStatus) {
    client.loop();

    if (digitalRead(D15)) {
      if (!jaGanhou) {
      client.publish(topic, "{00000000010000}");
      Serial.println("Ganhou jogo");
      jaGanhou = true;
      }
    }
    else
      jaGanhou = false;
    
    if (digitalRead(D2)) {
      if (!jaPerdeu) {
        client.publish(topic, "{00000000001000}");
        Serial.println("Perdeu jogo");
        jaPerdeu = true;
      }
    }
    else
      jaPerdeu = false;

      if (digitalRead(D25) || digitalRead(D26) || digitalRead(D27)) {
        if (!jaClicou) {
          if (digitalRead(D25)) {
            client.publish(topic, "{00000000000100}");
            Serial.println("botao esquerdo apertado");
          }
          else if (digitalRead(D26)) {
            client.publish(topic, "{00000000000010}");
            Serial.println("botao direito apertado");
          }
          else if (digitalRead(D27)) {
            client.publish(topic, "{00000000000001}");
            Serial.println("botao select apertado");
          }
        }
        jaClicou = true;
      }
      else
        jaClicou = false;
  }
}

/*
 * Função responsável por realizar a conexão entre o ESP8266
 * e o broker MQTT. Retorna true caso a conexão foi realizada
 * corretamente e false caso contrário.
 */
bool connectMQTT () {
  byte tentativa    = 0;
  int numTentativas = 5;
  client.setServer(mqtt_broker, mqtt_port);  
  client.setCallback(callback);             

  do {
    String client_id = "PoliQuest-ESP"; 

    if (client.connect(client_id.c_str(), mqtt_username, mqtt_password)) {
      Serial.println("Êxito na conexão:"); 
      Serial.print("Cliente ");
      Serial.print(client_id);
      Serial.println(" conectado ao Broker");
    }
    else {
      Serial.print("Falha ao conectar: ");
      Serial.print(client.state());
      Serial.println();
      Serial.print("Tentativa: ");
      Serial.println(tentativa);
      delay(2000);
    }
    tentativa ++;
  } while (!client.connected() && (tentativa < numTentativas));

  if (tentativa < numTentativas) {
    // conectado com sucesso 
    // publica e escreve no tópico
    client.publish(topic, "{Conectado ao broker}");
    client.subscribe(topic);
    return true;
  }
  else {
    Serial.println("Não conectado");
    return false;
  }
}

/*
 * Essa função será chamada pelo client.loop() toda vez que 
 * houver uma mudança no tópico que o esp está conectado.
 */
void callback (char * topic, byte * payload, unsigned int length) {
  Serial.print("Nova mensagem no topico: ");
  Serial.println(topic);
  Serial.print("Mensagem: ");

  for (int i = 0; i < length; i++) { 
    Serial.print((char) payload[i]);
  } 

  digitalWrite(D23, payload[1] - '0');
  digitalWrite(D22, payload[2] - '0');
  digitalWrite(D21, payload[3] - '0');
  digitalWrite(D19, payload[4] - '0');
  digitalWrite(D18, payload[5] - '0');
  digitalWrite(D5, payload[6] - '0');
  digitalWrite(TX2, payload[7] - '0');
  digitalWrite(RX2, payload[8] - '0');
  digitalWrite(D4, payload[9] - '0');

  Serial.println();
  Serial.println();
  Serial.print("D22 - iniciarJogo: ");
  Serial.println(digitalRead(D22));
  Serial.print("D21 - jogada[0]  : ");
  Serial.println(digitalRead(D21));
  Serial.print("D19 - jogada[1]  : ");
  Serial.println(digitalRead(D19));
  Serial.print("D18 - jogada[2]  : ");
  Serial.println(digitalRead(D18));
  Serial.print("D5  - jogada[3]  : ");
  Serial.println(digitalRead(D5));
  Serial.print("RX2 - resetJog   : ");
  Serial.println(digitalRead(RX2));
  Serial.print("TX2 - confimaJog : ");
  Serial.println(digitalRead(TX2));
  Serial.print("D23 - reset      : ");
  Serial.println(digitalRead(D23));
  Serial.print("D4  - sairJogo   : ");
  Serial.println(digitalRead(D4));
  Serial.println("-----------------------");

  // faz com que as saídas fiquem ativas por 50 milisegundos
  unsigned long int timer = millis();
  while ((millis() - timer) < 50) {
  }
  
  digitalWrite(D23, 0);
  digitalWrite(D22, 0);
  digitalWrite(D21, 0);
  digitalWrite(D19, 0);
  digitalWrite(D18, 0);
  digitalWrite(D5, 0);
  digitalWrite(TX2, 0);
  digitalWrite(RX2, 0);
  digitalWrite(D4, 0);
}