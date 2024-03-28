/*
 * ----------------------------------------------------------------------
 *  Arquivo   : poliquest_s3_1.1.ino
 *  Projeto   : PoliQuest
 * ----------------------------------------------------------------------
 *  Descricao : código reponsável por cominucar o FPGA com o servidor
 *              web por meio do ESP8266. Inpirado no vídeo do canal
 *              Bobsien "ESP8266 - envio e recebimento de dados via MQTT"
 * ----------------------------------------------------------------------
 *  Revisoes  :
 *      Data        Versao  Autor             Descricao
 *      22/03/2024  1.0     Vitor Thompson    versao inicial
 *      23/03/2024  1.2     Vitor Thompson    ativação das portas do ESP
 *      25/03/2024  1.3     Vitor Thompson    modificações nas portas
 * ----------------------------------------------------------------------
 */

// biblicoteca para a conexão com wifi
#include "ESP8266WiFi.h"

// biblioteca para utilizar o MQTT Broker, talvez seja necessário instalá-la antes
#include <PubSubClient.h>

// definição das portas para placa ESP8266 12-E NodeMCU
#define D0 16  // payload[1]  resetJog    HIGH no BOOT
#define D1 5   // payload[2]  jogada[0]
#define D2 4   // payload[3]  perdeu      BOOT falha se estiver em LOW
#define D3 0   // payload[4]  jogada[1] 
#define D4 2   // payload[5]  confimaJog  HIGH no BOOT, BOOT falha se estiver em LOW
#define D5 14  // payload[6]  jogada[2]
#define D6 12  // payload[7]  jogada[3]
#define D7 13  // payload[8]  ganhou
#define D8 15  // payload[9]  iniciarJogo BOOT falha se estiver em HIGH
#define RX 3   // payload[10] reset       HIGH no BOOT
#define TX 1   // payload[11] sairJogo    HIGH no BOOT, BOOT falha se estiver em LOW

// parametros para conexão com o WiFi
const char * ssid     = "AndroidAP7F43";    // nome da rede
const char * password = "tvuh4449"; // senha

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
  pinMode(D0, OUTPUT);
  pinMode(D1, OUTPUT);
  pinMode(D3, OUTPUT);
  pinMode(D4, OUTPUT);
  pinMode(D5, OUTPUT);
  pinMode(D6, OUTPUT);
  pinMode(D8, OUTPUT);
  pinMode(RX, OUTPUT);
  pinMode(D2, INPUT);
  pinMode(D7, INPUT);

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

 void loop () {
  if (mqttStatus) {
    client.loop();

    if (digitalRead(D7)) {
      // client.publish(topic, "{00000001000}");
      Serial.println("Ganhou jogo");
    }
    
    if (digitalRead(D2)) {
      // client.publish(topic, "{00000000100}");
      Serial.println("Perdeu jogo");
    }
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
    String client_id = "PoliQuest-";
    client_id += String(WiFi.macAddress()); 

    if (client.connect(client_id.c_str(), mqtt_username, mqtt_password)) {
      Serial.println("Êxito na conexão:"); 
      Serial.printf("Cliente %s conectado ao Broker \n", client_id.c_str());
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

  digitalWrite(D0, payload[1] - '0');
  digitalWrite(D1, payload[2] - '0');
  digitalWrite(D3, payload[4] - '0');
  digitalWrite(D4, payload[5] - '0');
  digitalWrite(D5, payload[6] - '0');
  digitalWrite(D6, payload[7] - '0');
  digitalWrite(D8, payload[9] - '0');
  digitalWrite(RX, payload[10] - '0');

  Serial.println();
  Serial.println();
  Serial.print("D8 - iniciarJogo: ");
  Serial.println(digitalRead(D8));
  Serial.print("D1 - jogada[0]  : ");
  Serial.println(digitalRead(D1));
  Serial.print("D3 - jogada[1]  : ");
  Serial.println(digitalRead(D3));
  Serial.print("D5 - jogada[2]  : ");
  Serial.println(digitalRead(D5));
  Serial.print("D6 - jogada[3]  : ");
  Serial.println(digitalRead(D6));
  Serial.print("D0 - resetJog   : ");
  Serial.println(digitalRead(D0));
  Serial.print("D4 - confimaJog : ");
  Serial.println(digitalRead(D4));
  Serial.print("D7 - ganhou     : ");
  Serial.println(digitalRead(D7));
  Serial.print("D2 - perdeu     : ");
  Serial.println(digitalRead(D2));
  Serial.print("RX - reset      : ");
  Serial.println(digitalRead(RX));
  Serial.println();

  // faz com que as saídas fiquem ativas por 50 milisegundos
  unsigned long int timer = millis();
  while ((millis() - timer) < 50) {
  }
  
  digitalWrite(D0, 0);
  digitalWrite(D1, 0);
  digitalWrite(D2, 0);
  digitalWrite(D3, 0);
  digitalWrite(D4, 0);
  digitalWrite(D5, 0);
  digitalWrite(D6, 0);
  digitalWrite(RX, 0);
  digitalWrite(TX, 0);

  Serial.println();
  Serial.println("-----------------");
}









