/* VENTO — BOZZA ESP32. SOLO SIMULAZIONE SU SERIALE.
   Non collega GPIO, non controlla carichi e non implementa rete/API.
   Le sicurezze hardware indipendenti sono indispensabili. */
#include <Arduino.h>

enum class Stage { Off, Fans, Preflow, Smoke, Actuation, Purge, Fault };
enum class Direction { Neutral, Upper, Lower };
Stage stage = Stage::Off;
Direction direction = Direction::Neutral;
uint32_t phaseStart = 0;
uint32_t heatStart = 0;
const uint32_t PREFLOW_MS = 2000; // ipotesi da validare sul sistema reale
const uint32_t MAX_HEAT_MS = 40000;
const uint32_t PURGE_MS = 3000;
bool safetyOK = false; // da ingresso monitorato; parte sempre non abilitato
bool flowOK = false;   // da sensore reale, mai assumere vero
bool fanOK = false;    // da feedback reale

const char* stageName() {
  switch (stage) {
    case Stage::Off: return "OFF"; case Stage::Fans: return "FANS";
    case Stage::Preflow: return "PREFLOW"; case Stage::Smoke: return "SMOKE";
    case Stage::Actuation: return "ACTUATION"; case Stage::Purge: return "PURGE";
    default: return "FAULT";
  }
}
void publishOutputs() {
  // Solo visualizzazione: non aggiungere pin reali senza progetto e test hardware.
  bool fans = stage == Stage::Fans || stage == Stage::Preflow || stage == Stage::Smoke || stage == Stage::Actuation || stage == Stage::Purge;
  bool pump = stage == Stage::Preflow || stage == Stage::Smoke || stage == Stage::Actuation || stage == Stage::Purge;
  bool heat = (stage == Stage::Smoke || stage == Stage::Actuation) && safetyOK && flowOK && fanOK;
  Serial.printf("state=%s fans=%d pump=%d heater=%d upper=%d lower=%d\n", stageName(), fans, pump, heat,
    stage == Stage::Actuation && direction == Direction::Upper,
    stage == Stage::Actuation && direction == Direction::Lower);
}
void change(Stage next) { stage = next; phaseStart = millis(); if (next == Stage::Smoke) heatStart = phaseStart; publishOutputs(); }
void fault() { direction = Direction::Neutral; change(Stage::Fault); }
void startPurge() { direction = Direction::Neutral; change(Stage::Purge); }

// Esempio di punto unico di validazione per una futura API autenticata.
// Restituisce false per comandi non ammessi. Un reboot riparte sempre da OFF.
bool command(const String &cmd) {
  if (cmd == "reset_fault" && stage == Stage::Fault && safetyOK) { change(Stage::Off); return true; }
  if (!safetyOK || stage == Stage::Fault) return false;
  if (cmd == "fans_on" && stage == Stage::Off) { change(Stage::Fans); return true; }
  if (cmd == "smoke_on" && stage == Stage::Fans && fanOK) { change(Stage::Preflow); return true; }
  if (cmd == "actuation_on" && stage == Stage::Smoke) { direction = Direction::Neutral; change(Stage::Actuation); return true; }
  if (stage == Stage::Actuation && (cmd == "upper" || cmd == "lower" || cmd == "neutral")) {
    direction = cmd == "upper" ? Direction::Upper : cmd == "lower" ? Direction::Lower : Direction::Neutral;
    publishOutputs(); return true;
  }
  if (cmd == "actuation_off" && stage == Stage::Actuation) { direction = Direction::Neutral; stage = Stage::Smoke; publishOutputs(); return true; }
  if (cmd == "smoke_off" && (stage == Stage::Smoke || stage == Stage::Preflow)) { startPurge(); return true; }
  if (cmd == "fans_off" && stage == Stage::Fans) { change(Stage::Off); return true; }
  return false;
}
void setup() { Serial.begin(115200); publishOutputs(); }
void loop() {
  // Integrazione futura: acquisire ingressi reali e prevedere timeout/diagnostica.
  if (stage != Stage::Off && stage != Stage::Fault && !safetyOK) fault();
  else if ((stage == Stage::Smoke || stage == Stage::Actuation) && (!flowOK || !fanOK)) fault();
  else if (stage == Stage::Preflow && millis() - phaseStart >= PREFLOW_MS) {
    if (flowOK && fanOK) change(Stage::Smoke); else fault();
  } else if ((stage == Stage::Smoke || stage == Stage::Actuation) && millis() - heatStart >= MAX_HEAT_MS) startPurge();
  else if (stage == Stage::Purge && millis() - phaseStart >= PURGE_MS) change(Stage::Fans);
  delay(10);
}
