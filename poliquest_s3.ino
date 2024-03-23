/*
 * ----------------------------------------------------------------------
 *  Arquivo   : poliquest_s3.ino
 *  Projeto   : PoliQuest
 * ----------------------------------------------------------------------
 *  Descricao : código reponsável por cominucar o FPGA com o servidor
 *              web por meio do ESP8266. Inpirado no vídeo do canal
 *              Bobsien "ESP8266 - envio e recebimento de dados via MQTT"
 * ----------------------------------------------------------------------
 *  Revisoes  :
 *      Data        Versao  Autor             Descricao
 *      22/03/2024  1.0     Vitor Thompson    versao inicial
 * ----------------------------------------------------------------------
 */

// biblicoteca para a conexão com wifi
# include "ESP8266WiFi.h"

// biblioteca para utilizar o MQTT Broker, talvez seja necessário instalá-la antes
# include <PubSubClient.h>

// parametros para conexão com o WiFi
const char * ssid     = "";    // nome da rede
const char * password = ""; // senha

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
    client.publish(topic, "{conectado ao broker, 123}");
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
  Serial.print("Message arrived in topic: ");
  Serial.println(topic);
  Serial.print("Message: ");
  for (int i = 0; i < length; i++) { 
    Serial.print((char) payload[i]);
  } 
  Serial.println();
  Serial.println("-----------------");
}









