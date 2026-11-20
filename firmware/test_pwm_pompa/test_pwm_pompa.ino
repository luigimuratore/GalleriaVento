// Test separato della pompa via XY-MOS su GPIO 27, ESP32 Arduino core 3.x.
// Pompa 12 V con alimentazione separata e diodo di ricircolo adeguato.
// Nessuna coil o carico a 230 V deve essere collegato durante la prova.
#include <Arduino.h>
#include <stdlib.h>
#include <errno.h>
constexpr uint8_t PUMP_PIN = 27;
constexpr uint32_t PWM_HZ = 1000;
constexpr uint8_t PWM_BITS = 10;
constexpr uint32_t PWM_MAX = (1UL << PWM_BITS) - 1;
char input[24];
size_t length = 0;
bool overflowed = false;

void setPump(unsigned percent) {
  uint32_t duty = (PWM_MAX * percent + 50) / 100;
  ledcWrite(PUMP_PIN, duty);
  Serial.printf("Pompa: %u%% (PWM %lu Hz)\n", percent, (unsigned long)PWM_HZ);
}
void handleLine() {
  input[length] = '\0';
  if (!strcmp(input, "off") || !strcmp(input, "OFF")) { setPump(0); return; }
  char *end = nullptr;
  errno = 0;
  long value = strtol(input, &end, 10);
  if (errno || end == input || *end != '\0' || value < 0 || value > 100) {
    Serial.println("Inserisci un numero intero 0-100 oppure off."); return;
  }
  setPump((unsigned)value);
}
void setup() {
  digitalWrite(PUMP_PIN, LOW);
  pinMode(PUMP_PIN, OUTPUT);
  Serial.begin(115200);
  if (!ledcAttach(PUMP_PIN, PWM_HZ, PWM_BITS)) {
    Serial.println("Errore configurazione PWM: pompa spenta.");
    while (true) delay(1000);
  }
  setPump(0);
  Serial.println("Invia 0-100 e premi Invio; off spegne la pompa.");
}
void loop() {
  while (Serial.available()) {
    char c = Serial.read();
    if (c == '\r' || c == '\n') {
      if (overflowed) Serial.println("Comando troppo lungo.");
      else if (length) handleLine();
      length = 0; overflowed = false;
    } else if (!overflowed) {
      if (length < sizeof(input) - 1) input[length++] = c;
      else overflowed = true;
    }
  }
}
