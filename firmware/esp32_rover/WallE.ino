/*
  WallE - ESP32 WROOM32 + driver L298N ZX040
  Richiede il pacchetto "esp32 by Espressif Systems" versione 3.x.
  PWM recente: ledcAttach(pin, frequenza, bit), ledcWrite(pin, duty).

  Collegamenti: ENA=13, IN1=12, IN2=14, IN3=4, IN4=2, ENB=25.
  Si assume motore A = ruota sinistra, motore B = ruota destra.
  Togliere i ponticelli ENA/ENB del driver per usare il PWM.
  Collegare GND del driver e GND ESP32 in comune.
  Eseguire il primo test con le ruote sollevate da terra.
  I GPIO12 e GPIO2 sono pin di strapping: il circuito esterno non deve
  imporre livelli incompatibili con l'avvio della scheda durante il reset.
*/
#include <Arduino.h>
#include <esp_arduino_version.h>

#if ESP_ARDUINO_VERSION_MAJOR < 3
#error "Questo sketch richiede Arduino-ESP32 3.x o successivo."
#endif

constexpr uint8_t ENA = 13;
constexpr uint8_t IN1 = 12;
constexpr uint8_t IN2 = 14;
constexpr uint8_t IN3 = 4;
constexpr uint8_t IN4 = 2;
constexpr uint8_t ENB = 25;

constexpr uint32_t FREQUENZA_PWM = 1000; // 1 kHz
constexpr uint8_t BIT_PWM = 8;           // Velocita' da 0 a 255
constexpr int PWM_MAX = 255;
constexpr int PWM_TEST = 90;            // Circa 35%; aumentare poco se non parte
constexpr unsigned long DURATA_TEST_MS = 1000;
constexpr unsigned long PAUSA_MS = 2000;

// Se una ruota gira al contrario durante "avanti", cambiare solo il
// relativo valore in true. Non occorre modificare le funzioni di movimento.
constexpr bool INVERTI_SINISTRO = false;
constexpr bool INVERTI_DESTRO = false;

bool pwmSinistroPronto = false;
bool pwmDestroPronto = false;

// Stop in rilascio: ENA/ENB a zero, nessuna frenatura attiva.
void stop() {
  if (pwmSinistroPronto) ledcWrite(ENA, 0);
  else digitalWrite(ENA, LOW);
  if (pwmDestroPronto) ledcWrite(ENB, 0);
  else digitalWrite(ENB, LOW);
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
}

// Comando firmato: positivo = avanti, negativo = indietro, zero = fermo.
// Il PWM viene disabilitato prima di cambiare il verso del singolo motore.
void impostaMotore(uint8_t enable, uint8_t pin1, uint8_t pin2,
                   int velocita, bool inverti) {
  velocita = constrain(velocita, -PWM_MAX, PWM_MAX);
  if (inverti) velocita = -velocita;
  if (!ledcWrite(enable, 0)) {
    stop();
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
    stop();
    Serial.println("Errore PWM: test interrotto.");
    while (true) delay(1000);
  }
}

// Controllo indipendente delle due ruote, ciascuna da -255 a +255.
// Prima di invertire un motore in movimento, chiamare stop() e lasciare
// una pausa sufficiente all'arresto meccanico (come nella sequenza sotto).
void impostaMotori(int sinistro, int destro) {
  impostaMotore(ENA, IN1, IN2, sinistro, INVERTI_SINISTRO);
  impostaMotore(ENB, IN3, IN4, destro, INVERTI_DESTRO);
}

// Le funzioni pubbliche accettano due intensita' indipendenti: 0..255.
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

void terminaPasso() {
  delay(DURATA_TEST_MS);
  stop();
  Serial.println("Stop: pausa di 2 secondi.");
  delay(PAUSA_MS);
}

void setup() {
  // Disabilita per primi gli ingressi enable.
  pinMode(ENA, OUTPUT);
  pinMode(ENB, OUTPUT);
  digitalWrite(ENA, LOW);
  digitalWrite(ENB, LOW);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  stop();
  Serial.begin(115200);

  // ledcAttach assegna automaticamente un canale distinto a ciascun pin.
  pwmSinistroPronto = ledcAttach(ENA, FREQUENZA_PWM, BIT_PWM);
  pwmDestroPronto = ledcAttach(ENB, FREQUENZA_PWM, BIT_PWM);
  stop();
  if (!pwmSinistroPronto || !pwmDestroPronto) {
    Serial.println("Inizializzazione PWM fallita: motori fermi.");
    while (true) delay(1000);
  }
  Serial.println("Ruote sollevate: il test parte tra 5 secondi.");
  delay(5000);
}

void loop() {
  // Esegue la sequenza UNA SOLA VOLTA dopo accensione/reset.
  static bool testCompletato = false;
  if (testCompletato) {
    delay(100);
    return;
  }
  testCompletato = true;

  // 1-2: identificare le ruote e controllare il loro verso singolarmente.
  Serial.println("1: solo ruota SINISTRA avanti.");
  avanti(PWM_TEST, 0);
  terminaPasso();
  Serial.println("2: solo ruota DESTRA avanti.");
  avanti(0, PWM_TEST);
  terminaPasso();

  // 3-4: entrambe le ruote devono spingere nella stessa direzione.
  Serial.println("3: AVANTI.");
  avanti(PWM_TEST, PWM_TEST);
  terminaPasso();
  Serial.println("4: INDIETRO.");
  indietro(PWM_TEST, PWM_TEST);
  terminaPasso();

  // 5-6: rotazioni sul posto, osservate dall'alto.
  Serial.println("5: SINISTRA (sinistra indietro, destra avanti).");
  giraSinistra(PWM_TEST, PWM_TEST);
  terminaPasso();
  Serial.println("6: DESTRA (sinistra avanti, destra indietro).");
  giraDestra(PWM_TEST, PWM_TEST);
  terminaPasso();

  stop();
  Serial.println("Test terminato. Motori fermi; premere RESET per ripetere.");
}
