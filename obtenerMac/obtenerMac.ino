#include <WiFi.h>

void setup() {
  Serial.begin(115200);

  // Espera un momento para que abra el monitor serie
  delay(1000);

  // Obtener la MAC Address
  String mac = WiFi.macAddress();

  Serial.println("=================================");
  Serial.println("     MAC ADDRESS DE LA ESP32");
  Serial.println("=================================");
  Serial.print("MAC: ");
  Serial.println(mac);
}

void loop() {
}
