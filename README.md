# Global-Solution-2
# ComfortSense – Monitor de Conforto Térmico e Ambiental

**Global Solution 2025 – O Futuro do Trabalho**  
Curso: Engenharia de Software – FIAP  
Turma: **ESPY**
Link para o Wokwi: https://wokwi.com/projects/447532200399107073

## Integrantes

- Nome: **Davi Falcão** – RM: **561818**  
- Nome: **Mateus Saavedra** – RM: **563266**  
- Nome: **Danilo Fernandes** – RM: **561657**  

---

## 1. Visão Geral do Projeto

O **ComfortSense** é um dispositivo IoT baseado em **ESP32** que monitora, em tempo real, as condições ambientais de um local de trabalho – **temperatura, umidade e luminosidade** – e calcula um **nível de conforto** para o ambiente.

O sistema utiliza:

- **Sensores** (DHT22 e LDR/potenciômetro) para coletar dados ambientais;  
- **LEDs** para indicar visualmente o nível de conforto (ideal, atenção ou desconforto);  
- **MQTT** para enviar os dados a um broker na nuvem (FIAP VM ou broker público), permitindo integração com dashboards e plataformas como FIWARE.

O objetivo é demonstrar, na prática, como a **Internet das Coisas (IoT)** pode ser aplicada à **saúde e bem-estar no trabalho**, tema central do **Futuro do Trabalho**.

---

## 2. Problema Abordado

Com o aumento do trabalho remoto e dos ambientes híbridos, muitos profissionais passam horas em locais com:

- Temperatura desconfortável (muito quente ou muito fria);  
- Umidade fora da faixa ideal;  
- Iluminação inadequada (ambiente escuro demais ou excessivamente claro).

Essas condições:

- Geram **fadiga**, desconforto e estresse;  
- Reduzem a **produtividade** e a concentração;  
- Podem afetar a **saúde física e mental** no longo prazo.

> **Problema:** falta de uma forma **simples, acessível e em tempo real** de monitorar o conforto ambiental no espaço de trabalho, especialmente em home offices e ambientes híbridos.

---

## 3. Solução Proposta – ComfortSense

O **ComfortSense** propõe um monitor de conforto ambiental que:

1. **Lê continuamente**:
   - Temperatura (°C);
   - Umidade (%);
   - Luminosidade (escala de 0 a 100).

2. **Classifica o ambiente** em três níveis:

   - `ideal` – condições dentro da faixa saudável;  
   - `atencao` – um dos parâmetros saiu um pouco da faixa;  
   - `desconforto` – dois ou mais parâmetros estão fora do ideal.

3. **Indica o estado por LEDs**:

   - LED **verde** → conforto ideal;  
   - LED **amarelo** → estado de atenção;  
   - LED **vermelho** → desconforto.

4. **Publica os dados via MQTT** para um broker (FIAP VM ou broker público):

   - Permitindo que sistemas externos armazenem histórico;  
   - Gerem gráficos e dashboards;  
   - Criem alertas ou automações (por exemplo, ajustar ar-condicionado ou iluminação).

---

## 4. Arquitetura da Solução

### 4.1. Componentes de Hardware (Simulados no Wokwi)

- 1x **ESP32 DevKit**;  
- 1x **DHT22** – sensor de temperatura e umidade;  
- 1x **LDR ou potenciômetro** (simulando a luminosidade) ligado a um pino ADC;  
- 3x **LEDs**:
  - LED vermelho (desconforto);  
  - LED amarelo (atenção);  
  - LED verde (ideal);  
- Resistores para LEDs e LDR/potenciômetro (no circuito Wokwi).

### 4.2. Mapeamento de Pinos

| Componente      | Pino ESP32    |
|-----------------|--------------:|
| DHT22 (data)    | GPIO 15 (`DHTPIN`) |
| LDR / Pot       | GPIO 34 (ADC) |
| LED vermelho    | GPIO 26 (`LED_R_PIN`) |
| LED verde       | GPIO 25 (`LED_G_PIN`) |
| LED amarelo     | GPIO 33 (`LED_Y_PIN`) |

> Obs.: se no diagrama o DHT estiver no GPIO 4, basta ajustar no código: `#define DHTPIN 4`.

### 4.3. Lógica de Conforto

Faixas usadas para classificar o ambiente:

- **Temperatura ideal:** 20°C a 26°C;  
- **Umidade ideal:** 40% a 60%;  
- **Luminosidade ideal:** 30% a 80% (na escala de 0 a 100 derivada do ADC).

O código soma um **score**:

- +1 se a temperatura está na faixa ideal;  
- +1 se a umidade está na faixa ideal;  
- +1 se a luminosidade está na faixa ideal.

Classificação:

- `score == 3` → `comfort = "ideal"`;  
- `score == 2` → `comfort = "atencao"`;  
- `score <= 1` → `comfort = "desconforto"`.

Esse **status** é usado tanto para acender o LED correto quanto para publicar um resumo no tópico principal do MQTT.

---

## 5. Comunicação IoT (MQTT e HTTP)

### 5.1. Brokers Utilizados

O projeto foi testado com dois cenários:

1. **Broker público (para demonstração no Wokwi):**

   ```cpp
   const char* default_BROKER_MQTT = "broker.hivemq.com";
   const int   default_BROKER_PORT = 1883;
