#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>

// Configurações - variáveis editáveis
const char* default_SSID = "Wokwi-GUEST"; // Nome da rede Wi-Fi
const char* default_PASSWORD = ""; // Senha da rede Wi-Fi
const char* default_BROKER_MQTT = "51.120.0.220";// IP do Broker MQTT
const int default_BROKER_PORT = 1883; // Porta do Broker MQTT
const char* default_TOPICO_SUBSCRIBE = "/TEF/lamp001/cmd"; // Tópico MQTT de escuta
const char* default_TOPICO_PUBLISH_1 = "/TEF/lamp001/attrs"; // Tópico MQTT de envio de informações para Broker
const char* default_TOPICO_PUBLISH_2 = "/TEF/lamp001/attrs/l"; // Tópico MQTT de envio de informações para Broker luminosity
const char* default_TOPICO_PUBLISH_3 = "/TEF/lamp001/attrs/h"; // humidity
const char* default_TOPICO_PUBLISH_4 = "/TEF/lamp001/attrs/t"; // temperature
const char* default_TOPICO_PUBLISH_5 = "/TEF/lamp001/attrs/d"; // Distancia
const char* default_ID_MQTT = "fiware_001"; // ID MQTT
const int default_D4 = 2; // Pino do LED onboard
// Declaração da variável para o prefixo do tópico
const char* topicPrefix = "lamp001";

// Variáveis para configurações editáveis
char* SSID = const_cast<char*>(default_SSID);
char* PASSWORD = const_cast<char*>(default_PASSWORD);
char* BROKER_MQTT = const_cast<char*>(default_BROKER_MQTT);
int BROKER_PORT = default_BROKER_PORT;
char* TOPICO_SUBSCRIBE = const_cast<char*>(default_TOPICO_SUBSCRIBE);
char* TOPICO_PUBLISH_1 = const_cast<char*>(default_TOPICO_PUBLISH_1);
char* TOPICO_PUBLISH_2 = const_cast<char*>(default_TOPICO_PUBLISH_2);
char* TOPICO_PUBLISH_3 = const_cast<char*>(default_TOPICO_PUBLISH_3);
char* TOPICO_PUBLISH_4 = const_cast<char*>(default_TOPICO_PUBLISH_4);
char* TOPICO_PUBLISH_5 = const_cast<char*>(default_TOPICO_PUBLISH_5);
char* ID_MQTT = const_cast<char*>(default_ID_MQTT);
int D4 = default_D4;

WiFiClient espClient;
PubSubClient MQTT(espClient);
char EstadoSaida = '0';

// Temperatura e Umidade com DHT22
#define DHTPIN 15 // Pino GPIO15 do ESP32 para o DHT22
#define DHTTYPE DHT22 // Tipo de sensor DHT (DHT22)
DHT dht(DHTPIN, DHTTYPE);

// Definindo portas LED
#define LED_R_PIN 26
#define LED_G_PIN 25
#define LED_Y_PIN 33

void initSerial() {
    Serial.begin(115200);
}

void initWiFi() {
    delay(10);
    Serial.println("------Conexao WI-FI------");
    Serial.print("Conectando-se na rede: ");
    Serial.println(SSID);
    Serial.println("Aguarde");
    reconectWiFi();
}

void initMQTT() {
    MQTT.setServer(BROKER_MQTT, BROKER_PORT);
    MQTT.setCallback(mqtt_callback);
}

void setup() {
    InitOutput();
    initSerial();
    dht.begin();
    initWiFi();
    initMQTT();
    delay(5000);

    pinMode(LED_R_PIN, OUTPUT);
    pinMode(LED_G_PIN, OUTPUT);
    pinMode(LED_Y_PIN, OUTPUT);

    digitalWrite(LED_R_PIN, LOW);
    digitalWrite(LED_G_PIN, LOW);
    digitalWrite(LED_Y_PIN, LOW);
}

void loop() {
    // 1) Garante que Wi-Fi e MQTT estão conectados
    VerificaConexoesWiFIEMQTT();
    MQTT.loop();

    // 2) Faz uma leitura completa do ambiente
    handleLuminosity();

    // 3) Espera 5 segundos até a próxima medição
    delay(5000);
}


void reconectWiFi() {
    if (WiFi.status() == WL_CONNECTED)
        return;
    WiFi.begin(SSID, PASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.print(".");
    }
    Serial.println();
    Serial.println("Conectado com sucesso na rede ");
    Serial.print(SSID);
    Serial.println("IP obtido: ");
    Serial.println(WiFi.localIP());

    // Garantir que o LED inicie desligado
    digitalWrite(D4, LOW);
}

void mqtt_callback(char* topic, byte* payload, unsigned int length) {
    String msg;
    for (int i = 0; i < length; i++) {
        char c = (char)payload[i];
        msg += c;
    }
    Serial.print("- Mensagem recebida: ");
    Serial.println(msg);

    // Forma o padrão de tópico para comparação
    String onTopic = String(topicPrefix) + "@on|";
    String offTopic = String(topicPrefix) + "@off|";

    // Compara com o tópico recebido
    if (msg.equals(onTopic)) {
        digitalWrite(D4, HIGH);
        EstadoSaida = '1';
    }

    if (msg.equals(offTopic)) {
        digitalWrite(D4, LOW);
        EstadoSaida = '0';
    }
}

void VerificaConexoesWiFIEMQTT() {
    if (!MQTT.connected())
        reconnectMQTT();
    reconectWiFi();
}

void EnviaEstadoOutputMQTT() {
    if (EstadoSaida == '1') {
        MQTT.publish(TOPICO_PUBLISH_1, "s|on");
        Serial.println("- Led Ligado");
    }

    if (EstadoSaida == '0') {
        MQTT.publish(TOPICO_PUBLISH_1, "s|off");
        Serial.println("- Led Desligado");
    }
    Serial.println("- Estado do LED onboard enviado ao broker!");
    delay(5000);
}

void InitOutput() {
    pinMode(D4, OUTPUT);
    digitalWrite(D4, HIGH);
    boolean toggle = false;

    for (int i = 0; i <= 10; i++) {
        toggle = !toggle;
        digitalWrite(D4, toggle);
        delay(200);
    }
}

void reconnectMQTT() {
    while (!MQTT.connected()) {
        Serial.print("* Tentando se conectar ao Broker MQTT: ");
        Serial.println(BROKER_MQTT);
        if (MQTT.connect(ID_MQTT)) {
            Serial.println("Conectado com sucesso ao broker MQTT!");
            MQTT.subscribe(TOPICO_SUBSCRIBE);
        } else {
            Serial.println("Falha ao reconectar no broker.");
            Serial.println("Haverá nova tentativa de conexão em 2s");
            delay(2000);
        }
    }
}

void handleLuminosity() {
    // 1) Leitura da luminosidade no pino 34 (LDR ou potenciometro)
    const int potLum = 34;
    int sensorValue = analogRead(potLum);
    int luminosity = map(sensorValue, 0, 4095, 0, 100); // 0 a 100 %

    // 2) Leitura dos sensores de temperatura e umidade (DHT22)
    float h = dht.readHumidity();
    float t = dht.readTemperature();

    if (isnan(h) || isnan(t)) {
      Serial.println("Falha ao ler o sensor DHT22!");
      return;
    }

    // 3) Publica os valores brutos nos tópicos que você já usa
    String msgLum = String(luminosity);
    MQTT.publish(TOPICO_PUBLISH_2, msgLum.c_str()); // luminosidade

    String msgHum = String(h);
    MQTT.publish(TOPICO_PUBLISH_3, msgHum.c_str()); // umidade

    String msgTemp = String(t);
    MQTT.publish(TOPICO_PUBLISH_4, msgTemp.c_str()); // temperatura

    // 4) Calcula o "score" de conforto
    // Regras:
    //  - Temperatura ideal: 20 a 26 C
    //  - Umidade ideal:     40 a 60 %
    //  - Luminosidade ideal:30 a 80 %
    int score = 0;
    if (t >= 20 && t <= 26) score++;
    if (h >= 40 && h <= 60) score++;
    if (luminosity >= 30 && luminosity <= 80) score++;

    String comfort;
    if (score == 3) {
      comfort = "ideal";
    } else if (score == 2) {
      comfort = "atencao";
    } else {
      comfort = "desconforto";
    }

    // 5) Atualiza os LEDs de status
    // LED_G_PIN -> verde   (ideal)
    // LED_Y_PIN -> amarelo (atencao)
    // LED_R_PIN -> vermelho(desconforto)

    if (comfort == "ideal") {
      digitalWrite(LED_G_PIN, HIGH);
      digitalWrite(LED_Y_PIN, LOW);
      digitalWrite(LED_R_PIN, LOW);
    } else if (comfort == "atencao") {
      digitalWrite(LED_G_PIN, LOW);
      digitalWrite(LED_Y_PIN, HIGH);
      digitalWrite(LED_R_PIN, LOW);
    } else { // desconforto
      digitalWrite(LED_G_PIN, LOW);
      digitalWrite(LED_Y_PIN, LOW);
      digitalWrite(LED_R_PIN, HIGH);
    }

    // 6) Publica um resumo do conforto no tópico principal
    // Formato FIWARE: "comfort|ideal" etc.
    String msgComfort = "comfort|" + comfort;
    MQTT.publish(TOPICO_PUBLISH_1, msgComfort.c_str());

    // 7) Log no Serial para visualizar
    Serial.print("Temp: ");
    Serial.print(t);
    Serial.print("  Umid: ");
    Serial.print(h);
    Serial.print("  Lum: ");
    Serial.print(luminosity);
    Serial.print("  Conforto: ");
    Serial.println(comfort);
}
