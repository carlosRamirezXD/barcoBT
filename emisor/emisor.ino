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
  <title>Control Carrito Gamer</title>
  <style>
    * {
      margin: 0;
      padding: 0;
      box-sizing: border-box;
    }
    
    body {
      display: flex;
      justify-content: center;
      align-items: center;
      min-height: 100vh;
      font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
      background: linear-gradient(135deg, #0f0c29, #302b63, #24243e);
      overflow: hidden;
      padding: 10px;
    }
    
    .container {
      display: grid;
      grid-template-columns: repeat(3, 1fr);
      grid-template-rows: repeat(3, 1fr);
      gap: 15px;
      width: 100%;
      height: 100%;
      max-width: min(90vmin, 500px);
      max-height: min(90vmin, 500px);
      padding: 20px;
      background: rgba(0, 0, 0, 0.4);
      border-radius: 20px;
      border: 2px solid rgba(0, 255, 255, 0.3);
      box-shadow: 0 0 40px rgba(0, 255, 255, 0.2);
    }
    
    button {
      font-size: clamp(14px, 3vmin, 20px);
      font-weight: bold;
      background: linear-gradient(145deg, #1e3c72, #2a5298);
      color: #00ffff;
      border: 2px solid #00ffff;
      border-radius: 12px;
      cursor: pointer;
      transition: all 0.3s ease;
      text-transform: uppercase;
      letter-spacing: 1px;
      box-shadow: 0 4px 15px rgba(0, 255, 255, 0.3);
      position: relative;
      overflow: hidden;
    }
    
    button::before {
      content: '';
      position: absolute;
      top: 50%;
      left: 50%;
      width: 0;
      height: 0;
      border-radius: 50%;
      background: rgba(0, 255, 255, 0.3);
      transform: translate(-50%, -50%);
      transition: width 0.6s, height 0.6s;
    }
    
    button:active::before {
      width: 300px;
      height: 300px;
    }
    
    button:hover {
      background: linear-gradient(145deg, #2a5298, #1e3c72);
      box-shadow: 0 6px 25px rgba(0, 255, 255, 0.5);
      transform: translateY(-2px);
      border-color: #00ffff;
    }
    
    button:active {
      transform: translateY(0);
      box-shadow: 0 2px 10px rgba(0, 255, 255, 0.4);
    }
    
    .btn-left {
      grid-column: 1;
      grid-row: 2;
    }
    
    .btn-right {
      grid-column: 3;
      grid-row: 2;
    }
    
    .btn-forward {
      grid-column: 2;
      grid-row: 1;
    }
    
    .btn-back {
      grid-column: 2;
      grid-row: 3;
    }
    
    .center-indicator {
      grid-column: 2;
      grid-row: 2;
      display: flex;
      justify-content: center;
      align-items: center;
      background: radial-gradient(circle, rgba(0, 255, 255, 0.1), transparent);
      border-radius: 50%;
      border: 2px solid rgba(0, 255, 255, 0.2);
    }
    
    .center-dot {
      width: 20px;
      height: 20px;
      background: #00ffff;
      border-radius: 50%;
      box-shadow: 0 0 20px #00ffff;
      animation: pulse 2s infinite;
    }
    
    @keyframes pulse {
      0%, 100% {
        opacity: 1;
        transform: scale(1);
      }
      50% {
        opacity: 0.6;
        transform: scale(0.8);
      }
    }
    
    @media (max-width: 600px) {
      .container {
        gap: 10px;
        padding: 15px;
      }
    }
  </style>
</head>
<body>
  <div class="container">
    <button class="btn-left" onclick="sendData('L')">◄ LATERAL IZQ</button>
    <button class="btn-right" onclick="sendData('R')">LATERAL DER ►</button>
    <button class="btn-forward" onclick="sendData('F')">▲ ADELANTE</button>
    <button class="btn-back" onclick="sendData('B')">▼ ATRÁS</button>
    <div class="center-indicator">
      <div class="center-dot"></div>
    </div>
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
    
    document.addEventListener('keydown', (e) => {
      switch(e.key) {
        case 'ArrowLeft':
        case 'a':
        case 'A':
          sendData('L');
          break;
        case 'ArrowRight':
        case 'd':
        case 'D':
          sendData('R');
          break;
        case 'ArrowUp':
        case 'w':
        case 'W':
          sendData('F');
          break;
        case 'ArrowDown':
        case 's':
        case 'S':
          sendData('B');
          break;
      }
    });
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

