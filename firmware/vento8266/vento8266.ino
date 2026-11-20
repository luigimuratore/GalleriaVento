#include <Arduino.h>
#if defined(ESP32)
#include <WiFi.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <ArduinoOTA.h>
#include <Preferences.h>
#include "ota_config.h"
using VentoWebServer = WebServer;
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
using VentoWebServer = ESP8266WebServer;
#else
#error Scheda non supportata
#endif
#include "secrets.h"
#include "state.h"
#include "dashboard_embedded.h"

// Solo la build esplicita VENTO_ENABLE_FAN_RELAYS comanda relè fisici.
#if defined(VENTO_ENABLE_FAN_RELAYS) && !defined(ESP32)
#error La prova relè è prevista solo per ESP32
#endif
#if defined(VENTO_ENABLE_FAN_RELAYS)
constexpr uint8_t FAN_RELAY_1 = 22, FAN_RELAY_2 = 23, MOS_PUMP = 27, MOS_COIL = 18;
#if defined(VENTO_DRY_TEST_OUTPUTS) || defined(VENTO_ENABLE_VALVES)
constexpr uint8_t VALVE_UPPER = 32, VALVE_LOWER = 33;
bool debugUpper = false, debugLower = false;
#endif
bool debugRelay1 = false, debugRelay2 = false, debugPump = false, debugCoil = false;
Preferences pumpPreferences;
uint8_t pumpPercent = 100;
bool pumpPwmReady = false;
constexpr uint32_t PUMP_PWM_HZ = 1000, PUMP_PWM_MAX = 1023;
void setupFanRelays() {
  // Precarica LOW prima di configurare le uscite; non controlla la fase di reset hardware.
  digitalWrite(FAN_RELAY_1, LOW); digitalWrite(FAN_RELAY_2, LOW);
  digitalWrite(MOS_PUMP, LOW); digitalWrite(MOS_COIL, LOW);
  pinMode(FAN_RELAY_1, OUTPUT); pinMode(FAN_RELAY_2, OUTPUT);
  pinMode(MOS_PUMP, OUTPUT); pinMode(MOS_COIL, OUTPUT);
#if defined(VENTO_DRY_TEST_OUTPUTS) || defined(VENTO_ENABLE_VALVES)
  digitalWrite(VALVE_UPPER, LOW); digitalWrite(VALVE_LOWER, LOW);
  pinMode(VALVE_UPPER, OUTPUT); pinMode(VALVE_LOWER, OUTPUT);
#endif
  if (pumpPreferences.begin("vento-pump", false)) {
    uint8_t saved = pumpPreferences.getUChar("percent", 100);
    pumpPercent = saved <= 100 ? saved : 100;
  }
  pumpPwmReady = ledcAttach(MOS_PUMP, PUMP_PWM_HZ, 10);
  if (pumpPwmReady) ledcWrite(MOS_PUMP, 0);
  else digitalWrite(MOS_PUMP, LOW);
}
void updateFanRelays(bool on, bool pumpOn = false, bool coilOn = false) {
  digitalWrite(FAN_RELAY_1, (on || debugRelay1) ? HIGH : LOW);
  digitalWrite(FAN_RELAY_2, (on || debugRelay2) ? HIGH : LOW);
  if (pumpPwmReady) ledcWrite(MOS_PUMP, (debugPump || pumpOn) ? (PUMP_PWM_MAX * pumpPercent + 50) / 100 : 0);
  digitalWrite(MOS_COIL, (debugCoil || coilOn) ? HIGH : LOW);
}
#else
void setupFanRelays() {}
void updateFanRelays(bool, bool = false, bool = false) {}
#endif
VentoWebServer server(80);
VentoState state;
void updateOutputs() {
#if defined(VENTO_ENABLE_FAN_RELAYS) && defined(VENTO_DRY_TEST_OUTPUTS)
  // Banco senza carichi: solo sequenza elettrica, NON autorizza l'uso con coil.
  updateFanRelays(state.fans, state.smoke || state.purging, state.coilRequested(millis()) && pumpPwmReady && pumpPercent > 0);
#else
  updateFanRelays(state.fans, state.smoke || state.purging);
#endif
#if defined(VENTO_ENABLE_FAN_RELAYS) && (defined(VENTO_DRY_TEST_OUTPUTS) || defined(VENTO_ENABLE_VALVES))
  digitalWrite(VALVE_UPPER, (debugUpper || (state.actuation && (state.direction == VentoState::UPPER || state.direction == VentoState::BOTH))) ? HIGH : LOW);
  digitalWrite(VALVE_LOWER, (debugLower || (state.actuation && (state.direction == VentoState::LOWER || state.direction == VentoState::BOTH))) ? HIGH : LOW);
#endif
}
const size_t networkCount = sizeof(NETWORKS) / sizeof(NETWORKS[0]);
size_t wifiIndex = 0;
unsigned long lastAttempt = 0;
bool wasConnected = false;
bool mdnsStarted = false;
#if defined(ESP32)
bool otaUpdating = false;
bool otaConfigured = false;
#endif
const char MDNS_HOST[] = "bench";
constexpr unsigned long WIFI_WAIT_MS = 15000;

void startAttempt() {
  WiFi.disconnect();
  WiFi.begin(NETWORKS[wifiIndex].ssid, NETWORKS[wifiIndex].password);
  lastAttempt = millis();
  Serial.printf("Tentativo rete %u/%u\n", unsigned(wifiIndex + 1), unsigned(networkCount));
}
void updateWifi() {
  bool connected = WiFi.status() == WL_CONNECTED;
  if (connected && !wasConnected) {
    Serial.print("Indirizzo rete: http://"); Serial.println(WiFi.localIP());
#if defined(ESP32)
    // La connessione può invalidare il socket UDP aperto prima del Wi-Fi.
    if (otaConfigured && !otaUpdating) { ArduinoOTA.end(); ArduinoOTA.begin(); Serial.println("Servizio OTA reinizializzato sulla rete connessa."); }
#endif
    if (mdnsStarted) MDNS.end();
    mdnsStarted = MDNS.begin(MDNS_HOST);
    if (mdnsStarted) { MDNS.addService("http", "tcp", 80); Serial.println("Nome locale: http://bench.local/"); }
    else Serial.println("Nome mDNS non disponibile; usa l'indirizzo IP.");
  }
  if (!connected && wasConnected) { lastAttempt = millis() - WIFI_WAIT_MS; }
  wasConnected = connected;
  if (!connected && millis() - lastAttempt >= WIFI_WAIT_MS) { wifiIndex = (wifiIndex + 1) % networkCount; startAttempt(); }
}
void sendStatus(int status = 200) {
  state.tick(millis());
  const char* direction = state.direction == VentoState::UPPER ? "upper" : state.direction == VentoState::LOWER ? "lower" : state.direction == VentoState::BOTH ? "both" : "neutral";
  String body = "{\"fans\":" + String(state.fans ? "true" : "false") + ",\"smoke\":" + (state.smoke ? "true" : "false") + ",\"act\":" + (state.actuation ? "true" : "false") + ",\"purging\":" + (state.purging ? "true" : "false") + ",\"direction\":\"" + direction + "\"" + ",\"simulation\":true}";
#if defined(VENTO_ENABLE_FAN_RELAYS)
  body.remove(body.length() - 1); body += String(",\"fansPhysical\":true,\"debugRelay1\":") + (debugRelay1 ? "true" : "false") + ",\"debugRelay2\":" + (debugRelay2 ? "true" : "false") + ",\"debugPump\":" + (debugPump ? "true" : "false") + ",\"debugCoil\":" + (debugCoil ? "true" : "false") + ",\"pumpPwmReady\":" + (pumpPwmReady ? "true" : "false") + ",\"pumpPercent\":" + String(pumpPercent) + ",\"preSeconds\":" + String(state.preheatMs / 1000) + ",\"purgeSeconds\":" + String(state.purgeMs / 1000) + "}";
#else
  body.remove(body.length() - 1); body += ",\"fansPhysical\":false}";
#endif
#if defined(VENTO_ENABLE_VALVES) || defined(VENTO_DRY_TEST_OUTPUTS)
  body.remove(body.length() - 1); body += String(",\"valvesPhysical\":true,\"debugUpper\":") + (debugUpper ? "true" : "false") + ",\"debugLower\":" + (debugLower ? "true" : "false") + "}";
#endif
#if defined(VENTO_DRY_TEST_OUTPUTS)
  body.remove(body.length() - 1); body += ",\"dryTestOutputs\":true}";
#endif
  server.sendHeader("Cache-Control", "no-store");
  server.send(status, "application/json", body);
}
void setup() {
  setupFanRelays();
#if defined(VENTO_ENABLE_FAN_RELAYS)
  const uint32_t legacy = pumpPreferences.getUInt("timings", (5UL << 16) | 15UL);
  const uint64_t fallback = (uint64_t(legacy >> 16) << 32) | (legacy & 0xffff);
  const uint64_t saved = pumpPreferences.getULong64("timings64", fallback);
  const uint32_t preSeconds = uint32_t(saved >> 32), purgeSeconds = uint32_t(saved);
  if (preSeconds <= 4294967UL && purgeSeconds <= 4294967UL) { state.preheatMs = preSeconds * 1000UL; state.purgeMs = purgeSeconds * 1000UL; }
#endif
  Serial.begin(115200);
  WiFi.persistent(false); WiFi.mode(WIFI_AP_STA); WiFi.setAutoReconnect(false);
  if (WiFi.softAP(AP_SSID, strlen(AP_PASSWORD) ? AP_PASSWORD : nullptr)) { Serial.print("Rete di emergenza: "); Serial.print(AP_SSID); Serial.print(" http://"); Serial.println(WiFi.softAPIP()); }
  else Serial.println("Errore avvio access point");
  mdnsStarted = MDNS.begin(MDNS_HOST);
  if (mdnsStarted) MDNS.addService("http", "tcp", 80);
#if defined(ESP32)
  if (strlen(OTA_PASSWORD) >= 16 && strcmp(OTA_PASSWORD, "SOSTITUISCI_CON_PASSWORD_PRIVATA") != 0) {
    ArduinoOTA.setHostname(MDNS_HOST);
    ArduinoOTA.setPort(3232);
    ArduinoOTA.setPassword(OTA_PASSWORD);
    ArduinoOTA.setMdnsEnabled(false); // mDNS della dashboard è gestito separatamente.
    ArduinoOTA.onStart([](){ state.reset();
#if defined(VENTO_ENABLE_FAN_RELAYS)
      debugRelay1 = debugRelay2 = debugPump = debugCoil = false;
#if defined(VENTO_ENABLE_VALVES) || defined(VENTO_DRY_TEST_OUTPUTS)
      debugUpper = debugLower = false;
#endif
#endif
      updateOutputs(); otaUpdating = true; Serial.println("OTA iniziato: simulazione azzerata."); });
    ArduinoOTA.onError([](ota_error_t){ state.reset(); updateFanRelays(false); otaUpdating = false; Serial.println("OTA non completato: simulazione azzerata."); });
    otaConfigured = true;
    ArduinoOTA.begin();
    Serial.println("OTA configurato sulla porta 3232.");
  } else Serial.println("OTA disabilitato: configurare una password privata valida.");
#endif
  startAttempt();
  server.on("/", HTTP_GET, [](){ server.sendHeader("Cache-Control", "no-store"); server.send_P(200, "text/html; charset=utf-8", DASHBOARD_HTML); });
  server.on("/api/status", HTTP_GET, [](){ sendStatus(); });
  server.on("/api/command", HTTP_POST, [](){
#if defined(ESP32)
    if (otaUpdating) { server.send(503, "text/plain", "Aggiornamento in corso"); return; }
#endif
#if defined(VENTO_ENABLE_FAN_RELAYS)
    if (debugRelay1 || debugRelay2 || debugPump || debugCoil) { server.send(409, "text/plain", "Turn off debug relays first"); return; }
#endif
#if defined(VENTO_ENABLE_VALVES) || defined(VENTO_DRY_TEST_OUTPUTS)
    if (debugUpper || debugLower) { server.send(409, "text/plain", "Turn off debug valves first"); return; }
#endif
    if (!server.hasArg("command")) { sendStatus(400); return; } bool ok = state.command(server.arg("command").c_str(), millis()); updateOutputs(); sendStatus(ok ? 200 : 409); });
#if defined(VENTO_ENABLE_FAN_RELAYS)
  server.on("/api/settings/timings", HTTP_POST, [](){
    if (otaUpdating || state.smoke || state.purging || state.actuation || debugPump || debugCoil) { server.send(409, "text/plain", "Stop flow lines and debug outputs first"); return; }
    if (!server.hasArg("password") || server.arg("password") != "sib") { server.send(403, "text/plain", "Forbidden"); return; }
    String pre = server.arg("pre"), purge = server.arg("purge");
    if (!pre.length() || pre.length() > 7 || !purge.length() || purge.length() > 7) { server.send(400, "text/plain", "Invalid timing"); return; }
    for (size_t i=0; i<pre.length(); ++i) if (pre[i] < '0' || pre[i] > '9') { server.send(400, "text/plain", "Invalid timing"); return; }
    for (size_t i=0; i<purge.length(); ++i) if (purge[i] < '0' || purge[i] > '9') { server.send(400, "text/plain", "Invalid timing"); return; }
    unsigned long a = pre.toInt(), b = purge.toInt();
    if (a > 4294967UL || b > 4294967UL) { server.send(400, "text/plain", "Timing out of range"); return; }
    uint64_t value = (uint64_t(a) << 32) | uint64_t(b);
    if ((state.preheatMs != a*1000 || state.purgeMs != b*1000) && pumpPreferences.putULong64("timings64", value) != sizeof(uint64_t)) { server.send(500, "text/plain", "Save failed"); return; }
    state.preheatMs = a*1000; state.purgeMs = b*1000; sendStatus();
  });
  server.on("/api/settings/pump", HTTP_POST, [](){
    if (otaUpdating) { server.send(503, "text/plain", "Updating"); return; }
    if (!server.hasArg("password") || server.arg("password") != "sib") { server.send(403, "text/plain", "Forbidden"); return; }
    if (!pumpPwmReady || !server.hasArg("percent")) { server.send(503, "text/plain", "PWM unavailable"); return; }
    String text = server.arg("percent");
    if (!text.length() || text.length() > 3) { server.send(400, "text/plain", "Invalid percent"); return; }
    for (size_t i = 0; i < text.length(); ++i) if (text[i] < '0' || text[i] > '9') { server.send(400, "text/plain", "Invalid percent"); return; }
    int value = text.toInt();
    if (value > 100) { server.send(400, "text/plain", "Invalid percent"); return; }
    if (value != pumpPercent) {
      if (pumpPreferences.putUChar("percent", value) != 1) { server.send(500, "text/plain", "Save failed"); return; }
      pumpPercent = value;
    }
    updateOutputs(); sendStatus();
  });
  server.on("/api/debug/mos", HTTP_POST, [](){
    if (otaUpdating || state.fans || state.smoke || state.actuation || state.purging) { server.send(409, "text/plain", "Stop other functions first"); return; }
    if (!server.hasArg("password") || server.arg("password") != "sib") { server.send(403, "text/plain", "Forbidden"); return; }
    const String mos = server.arg("mos"), value = server.arg("value");
    if ((mos != "pump" && mos != "coil") || (value != "on" && value != "off")) { server.send(400, "text/plain", "Invalid command"); return; }
#if defined(VENTO_ENABLE_VALVES) && !defined(VENTO_DRY_TEST_OUTPUTS)
    if (mos == "coil" && value == "on") { server.send(403, "text/plain", "Coil test disabled"); return; }
#endif
    if (mos == "pump") debugPump = value == "on"; else debugCoil = value == "on";
    updateFanRelays(false); sendStatus();
  });
#if defined(VENTO_ENABLE_VALVES) || defined(VENTO_DRY_TEST_OUTPUTS)
  server.on("/api/debug/valve", HTTP_POST, [](){
    if (otaUpdating || state.fans || state.smoke || state.actuation || state.purging) { server.send(409, "text/plain", "Stop other functions first"); return; }
    if (!server.hasArg("password") || server.arg("password") != "sib") { server.send(403, "text/plain", "Forbidden"); return; }
    String valve = server.arg("valve"), value = server.arg("value");
    if ((valve != "upper" && valve != "lower") || (value != "on" && value != "off")) { server.send(400, "text/plain", "Invalid command"); return; }
    if (valve == "upper") debugUpper = value == "on"; else debugLower = value == "on";
    updateOutputs(); sendStatus();
  });
#endif
  server.on("/api/debug/relay", HTTP_POST, [](){
    if (otaUpdating || state.fans || state.smoke || state.actuation || state.purging) { server.send(409, "text/plain", "Stop other functions first"); return; }
    if (!server.hasArg("password") || server.arg("password") != "sib") { server.send(403, "text/plain", "Forbidden"); return; }
    const String relay = server.arg("relay"), value = server.arg("value");
    if ((relay != "1" && relay != "2") || (value != "on" && value != "off")) { server.send(400, "text/plain", "Invalid command"); return; }
    if (relay == "1") debugRelay1 = value == "on"; else debugRelay2 = value == "on";
    updateFanRelays(false); sendStatus();
  });
#endif
  server.onNotFound([](){ server.send(404, "text/plain", "Non trovato"); });
  server.begin();
#if defined(VENTO_ENABLE_FAN_RELAYS)
  Serial.println("VENTO: relè ventole GPIO 22/23 e pompa GPIO 27 attivi; coil e valvole simulate.");
#else
  Serial.println("VENTO: simulazione, nessuna uscita hardware attiva.");
#endif
}
void loop() {
#if defined(ESP32)
  ArduinoOTA.handle();
#endif
  server.handleClient(); state.tick(millis()); updateOutputs(); updateWifi();
#if defined(ESP8266)
  if (mdnsStarted) MDNS.update();
#endif
  yield();
}
