// Test a impulsi ESP32-WROOM. Tenere 230 V e carichi scollegati.
// Valido solo per moduli verificati attivi HIGH.
constexpr uint8_t RELAY1_PIN = 22, RELAY2_PIN = 23, MOS1_PIN = 27, MOS2_PIN = 18;
constexpr unsigned long MAX_RELAY_SECONDS = 300;
const uint8_t pins[] = {RELAY1_PIN, RELAY2_PIN, MOS1_PIN, MOS2_PIN};
int activePin = -1;
unsigned long startedAt = 0, durationMs = 0;
char line[32];
size_t lineLength = 0;
bool overflow = false;

void allOff() {
  for (uint8_t pin : pins) digitalWrite(pin, LOW);
  activePin = -1;
}
void pulse(uint8_t pin, const char *name, unsigned long seconds) {
  allOff();
  digitalWrite(pin, HIGH);
  activePin = pin;
  startedAt = millis();
  durationMs = seconds * 1000UL;
  Serial.printf("%s ON per %lu secondi\n", name, seconds);
}
void execute(char *command) {
  char which = 0, extra = 0;
  unsigned long seconds = 0;
  if (sscanf(command, " %c", &which) != 1) return;
  if (which == 'x' || which == 'X') { allOff(); Serial.println("Tutto OFF"); return; }
  if (which == '1' || which == '2') { pulse(which == '1' ? MOS1_PIN : MOS2_PIN, which == '1' ? "MOS1" : "MOS2", 1); return; }
  if ((which == 'r' || which == 'R' || which == 's' || which == 'S') &&
      sscanf(command, " %c %lu %c", &which, &seconds, &extra) == 2 &&
      seconds >= 1 && seconds <= MAX_RELAY_SECONDS) {
    bool first = which == 'r' || which == 'R';
    pulse(first ? RELAY1_PIN : RELAY2_PIN, first ? "RELAY1" : "RELAY2", seconds);
    return;
  }
  Serial.println("Comando non valido. Usa r 10 oppure s 10 (1-300 secondi), 1, 2 o x.");
}
void setup() {
  for (uint8_t pin : pins) { digitalWrite(pin, LOW); pinMode(pin, OUTPUT); }
  allOff();
  Serial.begin(115200);
  Serial.println("Invia r 10 o s 10 e premi Invio: rele GPIO22 / GPIO23 per 10 secondi.");
  Serial.println("Durata 1-300 secondi. 1/2: MOS per 1 secondo. x: tutto OFF.");
}
void loop() {
  if (activePin >= 0 && millis() - startedAt >= durationMs) { allOff(); Serial.println("Tempo scaduto: tutto OFF"); }
  while (Serial.available()) {
    char c = Serial.read();
    if (c == '\r' || c == '\n') {
      if (overflow) Serial.println("Comando troppo lungo.");
      else if (lineLength) { line[lineLength] = '\0'; execute(line); }
      lineLength = 0; overflow = false;
    } else if (!overflow) {
      if (lineLength < sizeof(line) - 1) line[lineLength++] = c;
      else overflow = true;
    }
  }
}
