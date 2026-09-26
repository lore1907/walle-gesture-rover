#include "WiFi.h"
#include <WiFiUdp.h>
#include <Arduino.h>
#include <esp_arduino_version.h>

#if ESP_ARDUINO_VERSION_MAJOR < 3
#error "Questo sketch richiede Arduino-ESP32 3.x o successivo."
#endif

//costanti corrispondenti con configurazione hardware dei pin
constexpr uint8_t ENA = 13;
constexpr uint8_t IN1 = 12;
constexpr uint8_t IN2 = 14;
constexpr uint8_t IN3 = 4;
constexpr uint8_t IN4 = 2;
constexpr uint8_t ENB = 25;

constexpr uint32_t FREQUENZA_PWM = 1000; // 1 kHz
constexpr uint8_t BIT_PWM = 8;           // Velocita' da 0 a 255
constexpr int PWM_MAX = 255;
constexpr int PWM_TEST = 150;            // Circa 35%; aumentare poco se non parte


// Se una ruota gira al contrario durante "avanti", cambiare solo il
// relativo valore in true. Non occorre modificare le funzioni di movimento.
constexpr bool INVERTI_SINISTRO = false;
constexpr bool INVERTI_DESTRO = false;

bool pwmSinistroPronto = false;
bool pwmDestroPronto = false;

//variabili di connessione wifi
const char* ssid = "iPhone";
const char* psw = "stabia10";

//porta udp in ascolto
unsigned int localPort = 8000;

WiFiUDP udp;
char packetBuffer[255];//buffer dei messaggi in arrivo;


void setup() {
  Serial.begin(115200);
  delay(1000);

  // ==========================
  // INIZIALIZZAZIONE MOTORI
  // ==========================

  pinMode(ENA, OUTPUT);
  pinMode(ENB, OUTPUT);

  digitalWrite(ENA, LOW);
  digitalWrite(ENB, LOW);

  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  pwmSinistroPronto = ledcAttach(ENA, FREQUENZA_PWM, BIT_PWM);
  pwmDestroPronto = ledcAttach(ENB, FREQUENZA_PWM, BIT_PWM);

  if (!pwmSinistroPronto || !pwmDestroPronto) {
    Serial.println("Errore inizializzazione PWM.");
    while (true) {
      delay(1000);
    }
  }

  stopMotori();

  // ==========================
  // WIFI
  // ==========================

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(500);

  Serial.println("--- TEST AVVIATO ---");

  int retiTrovate = WiFi.scanNetworks();

  for (int i = 0; i < retiTrovate; i++) {
    Serial.print("Rete: [");
    Serial.print(WiFi.SSID(i));
    Serial.print("] Segnale: ");
    Serial.println(WiFi.RSSI(i));
  }

  WiFi.begin(ssid, psw);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("Connessione riuscita");

  Serial.print("Indirizzo IP: ");
  Serial.println(WiFi.localIP());

  udp.begin(localPort);

  Serial.print("Server UDP in ascolto sulla porta ");
  Serial.println(localPort);
}
// Comando firmato: positivo = avanti, negativo = indietro, zero = fermo.
// Il PWM viene disabilitato prima di cambiare il verso del singolo motore.
void impostaMotore(uint8_t enable, uint8_t pin1, uint8_t pin2,
                   int velocita, bool inverti) {
  velocita = constrain(velocita, -PWM_MAX, PWM_MAX);
  if (inverti) velocita = -velocita;
  if (!ledcWrite(enable, 0)) {
    stopMotori();
    Serial.println("Errore PWM: test interrotto.");
    while (true) delay(1000);
  }
  digitalWrite(pin1, LOW);
  digitalWrite(pin2, LOW);
  delayMicroseconds(100);
  digitalWrite(pin1, velocita > 0 ? HIGH : LOW);
  digitalWrite(pin2, velocita < 0 ? HIGH : LOW);
  const uint32_t duty = velocita < 0 ? -velocita : velocita;
  if (!ledcWrite(enable, duty)) {
    stopMotori();
    Serial.println("Errore PWM: test interrotto.");
    while (true) delay(1000);
  }
}

void impostaMotori(int sinistro, int destro) {
  impostaMotore(ENA, IN1, IN2, sinistro, INVERTI_SINISTRO);
  impostaMotore(ENB, IN3, IN4, destro, INVERTI_DESTRO);
}


void avanti(int sinistro, int destro) {
  impostaMotori(constrain(sinistro, 0, PWM_MAX), constrain(destro, 0, PWM_MAX));
}

void indietro(int sinistro, int destro) {
  impostaMotori(-constrain(sinistro, 0, PWM_MAX), -constrain(destro, 0, PWM_MAX));
}

// Rotazioni sul posto: le due ruote girano in direzioni opposte.
void giraSinistra(int sinistro, int destro) {
  impostaMotori(-constrain(sinistro, 0, PWM_MAX), constrain(destro, 0, PWM_MAX));
}

void giraDestra(int sinistro, int destro) {
  impostaMotori(constrain(sinistro, 0, PWM_MAX), -constrain(destro, 0, PWM_MAX));
}


void stopMotori() {
  if (pwmSinistroPronto) ledcWrite(ENA, 0);
  else digitalWrite(ENA, LOW);
  if (pwmDestroPronto) ledcWrite(ENB, 0);
  else digitalWrite(ENB, LOW);
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
}

void loop() {
  int packetSize = udp.parsePacket();
  if(packetSize){
    Serial.print("Ricevuti ");
    Serial.print(packetSize);
    Serial.print(" bytes da ");
    IPAddress remoteIp = udp.remoteIP();
    Serial.print(remoteIp);
    Serial.print(", porta ");
    Serial.println(udp.remotePort());

    int len = udp.read(packetBuffer,255);
    if (len > 0) {
      packetBuffer[len] = 0; // Termina la stringa
    }
      
    Serial.print("Messaggio: ");
    Serial.println(packetBuffer);
  
    if(strcmp(packetBuffer,"AVANTI") == 0){
      avanti(PWM_TEST, PWM_TEST);
    }
    else if(strcmp(packetBuffer,"INDIETRO") == 0){
      indietro(PWM_TEST, PWM_TEST);   
    }
    else if(strcmp(packetBuffer,"SINISTRA") == 0){
      giraSinistra(PWM_TEST, PWM_TEST);
    }
    else if(strcmp(packetBuffer,"DESTRA") == 0){
      giraDestra(PWM_TEST, PWM_TEST);
    }
    else if (strcmp(packetBuffer, "STOP") == 0) {
      stopMotori();
    }
  }
}

 
