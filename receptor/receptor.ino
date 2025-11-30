#include <WiFi.h>
#include <esp_now.h>

#define IN1_PIN 25
#define IN2_PIN 26
#define IN3_PIN 27
#define IN4_PIN 13

bool motor1State = false;
bool motor2State = false;
bool stateL = false;
bool stateR = false;

uint8_t senderMAC[] = {0xCC, 0x7B, 0x5C, 0xF0, 0xFD, 0xF0};

char receivedCommand;
bool commandProcessed = true;

void stopAllMotors() {
  digitalWrite(IN1_PIN, LOW);
  digitalWrite(IN2_PIN, LOW);
  digitalWrite(IN3_PIN, LOW);
  digitalWrite(IN4_PIN, LOW);

  motor1State = false;
  motor2State = false;
  stateL = false;
  stateR = false;

  Serial.println("Todos los motores apagados");
}

void onDataRecv(const uint8_t *mac_addr, const uint8_t *data, int len) {
  receivedCommand = (char)data[0];
  Serial.print("Comando recibido: ");
  Serial.println(receivedCommand);
  commandProcessed = false;
}

void setup() {
  Serial.begin(115200);
  pinMode(IN1_PIN, OUTPUT);
  pinMode(IN2_PIN, OUTPUT);
  pinMode(IN3_PIN, OUTPUT);
  pinMode(IN4_PIN, OUTPUT);
  stopAllMotors(); // Asegurarse de iniciar con los motores apagados
  WiFi.mode(WIFI_STA);

  if (esp_now_init() != ESP_OK) {
    Serial.println("Error iniciando ESP-NOW");
    return;
  }

  esp_now_register_recv_cb(onDataRecv);

  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, senderMAC, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Error añadiendo peer");
    return;
  }

  Serial.println("ESP32 Carro listo para recibir comandos");
}

void loop() {
  if (commandProcessed) return;

  switch (receivedCommand) {
    case 'F':
      if (motor1State && motor2State) { // Si los motores ya están avanzando
        stopAllMotors();
        Serial.println("Motores apagados (deteniendo avance)");
      } else {
        stopAllMotors(); // Apaga todo antes de avanzar
        digitalWrite(IN1_PIN, HIGH);
        digitalWrite(IN3_PIN, HIGH);
        motor1State = true;
        motor2State = true;
        Serial.println("Motores avanzando");
      }
      break;

    case 'B':
      if (motor1State && motor2State) { // Si los motores ya están retrocediendo
        stopAllMotors();
        Serial.println("Motores apagados (deteniendo retroceso)");
      } else {
        stopAllMotors(); // Apaga todo antes de retroceder
        digitalWrite(IN2_PIN, HIGH);
        digitalWrite(IN4_PIN, HIGH);
        motor1State = true;
        motor2State = true;
        Serial.println("Motores retrocediendo");
      }
      break;

    case 'L':
      if (stateL) { // Si ya está girando a la izquierda
        stopAllMotors();
        Serial.println("Motores apagados (deteniendo giro a la izquierda)");
      } else {
        stopAllMotors(); // Apaga todo antes de girar
        digitalWrite(IN1_PIN, HIGH);
        digitalWrite(IN4_PIN, HIGH);
        stateL = true;
        motor1State = true;
        motor2State = true;
        Serial.println("Girando a la izquierda");
      }
      break;

    case 'R':
      if (stateR) { // Si ya está girando a la derecha
        stopAllMotors();
        Serial.println("Motores apagados (deteniendo giro a la derecha)");
      } else {
        stopAllMotors(); // Apaga todo antes de girar
        digitalWrite(IN2_PIN, HIGH);
        digitalWrite(IN3_PIN, HIGH);
        stateR = true;
        motor1State = true;
        motor2State = true;
        Serial.println("Girando a la derecha");
      }
      break;

    default:
      Serial.println("Comando no reconocido");
      break;
  }

  commandProcessed = true;
  delay(100);
}

