#include <WiFi.h>
#include <WebServer.h>
#include <esp_now.h>

const char* ssid = "Emisor";
const char* password = "12345678";

uint8_t receiverMAC[] = {0xA0, 0xA3, 0xB3, 0xAA, 0xBB, 0xF8};

WebServer server(80);

void onDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("Estado del envío: ");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Éxito" : "Fallo");
}

const char htmlPage[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Control Carrito</title>
  <style>
    body {
      display: flex;
      justify-content: center;
      align-items: center;
      height: 100vh;
      margin: 0;
      font-family: Arial, sans-serif;
      background-color: #f0f0f0;
    }
    .container {
      display: grid;
      grid-template-columns: 1fr 1fr;
      grid-gap: 20px;
      width: 100%;
      max-width: 600px;
    }
    button {
      width: 100px;
      height: 50px;
      margin: 10px;
      font-size: 16px;
      font-weight: bold;
      background-color: #007BFF;
      color: white;
      border: none;
      border-radius: 5px;
      cursor: pointer;
    }
    button:hover {
      background-color: #0056b3;
    }
  </style>
</head>
<body>
  <div class="container">
    <button onclick="sendData('L')">Izquierda</button>
    <button onclick="sendData('R')">Derecha</button>
    <button onclick="sendData('F')">Avanzar</button>
    <button onclick="sendData('B')">Reversa</button>
  </div>
  <script>
    function sendData(command) {
      fetch('/?data=' + command)
        .then(response => {
          if (response.ok) {
            console.log('Comando enviado:', command);
          } else {
            console.error('Error al enviar el comando');
          }
        })
        .catch(error => console.error('Error:', error));
    }
  </script>
</body>
</html>
)rawliteral";

void handleRoot() {
  if (server.hasArg("data")) {
    char command = server.arg("data")[0];
    Serial.println(command);

    esp_err_t result = esp_now_send(receiverMAC, (uint8_t*)&command, sizeof(command));
    if (result == ESP_OK) {
      Serial.println("Dato enviado por ESP-NOW: " + String(command));
    } else {
      Serial.println("Error enviando dato por ESP-NOW");
    }

    server.send(200, "text/plain", "OK");
  } else {
    server.send(200, "text/html", htmlPage);
  }
}

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP(ssid, password);
  Serial.println("Punto de acceso configurado");
  Serial.print("Nombre de red: ");
  Serial.println(ssid);
  Serial.print("Contraseña: ");
  Serial.println(password);
  Serial.print("Dirección IP del punto de acceso: ");
  Serial.println(WiFi.softAPIP());

  if (esp_now_init() != ESP_OK) {
    Serial.println("Error iniciando ESP-NOW");
    return;
  }
  esp_now_register_send_cb(onDataSent);
  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, receiverMAC, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Error añadiendo peer");
    return;
  }

  server.on("/", handleRoot);
  server.begin();
}

void loop() {
  server.handleClient();
}

