#include "WiFi.h"

const char* ssid = "iPhone";
const char* psw = "stabia10";

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(500);
  
  Serial.println("--- TEST AVVIATO ---");
  
  // Avvia una singola scansione veloce
  int retiTrovate = WiFi.scanNetworks();
  for (int i = 0; i < retiTrovate; i++) {
    Serial.print("Rete: [");
    Serial.print(WiFi.SSID(i));
    Serial.print("] Segnale: ");
    Serial.println(WiFi.RSSI(i));
  }

  if (retiTrovate >= 0) {
    Serial.println("✅ FUNZIONA! Il chip Wi-Fi risponde correttamente.");
    Serial.print("Reti rilevate nei paraggi: ");
    Serial.println(retiTrovate);
  } else {
    Serial.println("❌ ERRORE: La scansione ha restituito un codice di fallimento.");
  }

  WiFi.begin(ssid, psw);

  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("Connessione riuscita");
  Serial.print("Indirizzo IP:");
  Serial.println(WiFi.localIP());

}

void loop() {
  // Non fa nulla, il test gira una volta sola all'avvio
}