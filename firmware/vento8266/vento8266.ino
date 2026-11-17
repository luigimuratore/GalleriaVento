#include <Arduino.h>
#if defined(ESP32)
#include <WiFi.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <ArduinoOTA.h>
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
constexpr uint8_t FAN_RELAY_1 = 22, FAN_RELAY_2 = 23;
void setupFanRelays() {
  // Precarica LOW prima di configurare le uscite; non controlla la fase di reset hardware.
  digitalWrite(FAN_RELAY_1, LOW); digitalWrite(FAN_RELAY_2, LOW);
  pinMode(FAN_RELAY_1, OUTPUT); pinMode(FAN_RELAY_2, OUTPUT);
}
void updateFanRelays(bool on) {
  digitalWrite(FAN_RELAY_1, on ? HIGH : LOW);
  digitalWrite(FAN_RELAY_2, on ? HIGH : LOW);
}
#else
void setupFanRelays() {}
void updateFanRelays(bool) {}
#endif
VentoWebServer server(80);
VentoState state;
const size_t networkCount = sizeof(NETWORKS) / sizeof(NETWORKS[0]);
size_t wifiIndex = 0;
unsigned long lastAttempt = 0;
bool wasConnected = false;
bool mdnsStarted = false;
#if defined(ESP32)
bool otaUpdating = false;
#endif
const char MDNS_HOST[] = "galleriaventosafran";
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
    mdnsStarted = MDNS.begin(MDNS_HOST);
    if (mdnsStarted) { MDNS.addService("http", "tcp", 80); Serial.println("Nome locale: http://galleriaventosafran.local/"); }
    else Serial.println("Nome mDNS non disponibile; usa l'indirizzo IP.");
  }
  if (!connected && wasConnected) { if (mdnsStarted) MDNS.end(); mdnsStarted = false; lastAttempt = millis() - WIFI_WAIT_MS; }
  wasConnected = connected;
  if (!connected && millis() - lastAttempt >= WIFI_WAIT_MS) { wifiIndex = (wifiIndex + 1) % networkCount; startAttempt(); }
}
void sendStatus(int status = 200) {
  state.tick(millis());
  const char* direction = state.direction == VentoState::UPPER ? "upper" : state.direction == VentoState::LOWER ? "lower" : state.direction == VentoState::BOTH ? "both" : "neutral";
  String body = "{\"fans\":" + String(state.fans ? "true" : "false") + ",\"smoke\":" + (state.smoke ? "true" : "false") + ",\"act\":" + (state.actuation ? "true" : "false") + ",\"purging\":" + (state.purging ? "true" : "false") + ",\"direction\":\"" + direction + "\",\"simulation\":true}";
#if defined(VENTO_ENABLE_FAN_RELAYS)
  body.remove(body.length() - 1); body += ",\"fansPhysical\":true}";
#else
  body.remove(body.length() - 1); body += ",\"fansPhysical\":false}";
#endif
  server.sendHeader("Cache-Control", "no-store");
  server.send(status, "application/json", body);
}
void setup() {
  setupFanRelays();
  Serial.begin(115200);
  WiFi.persistent(false); WiFi.mode(WIFI_AP_STA); WiFi.setAutoReconnect(false);
  if (WiFi.softAP(AP_SSID, AP_PASSWORD)) { Serial.print("Rete di emergenza: "); Serial.print(AP_SSID); Serial.print(" http://"); Serial.println(WiFi.softAPIP()); }
  else Serial.println("Errore avvio access point");
#if defined(ESP32)
  if (strlen(OTA_PASSWORD) >= 16 && strcmp(OTA_PASSWORD, "SOSTITUISCI_CON_PASSWORD_PRIVATA") != 0) {
    ArduinoOTA.setHostname(MDNS_HOST);
    ArduinoOTA.setPort(3232);
    ArduinoOTA.setPassword(OTA_PASSWORD);
    ArduinoOTA.setMdnsEnabled(false); // mDNS della dashboard è gestito separatamente.
    ArduinoOTA.onStart([](){ state.reset(); updateFanRelays(false); otaUpdating = true; Serial.println("OTA iniziato: simulazione azzerata."); });
    ArduinoOTA.onError([](ota_error_t){ state.reset(); updateFanRelays(false); otaUpdating = false; Serial.println("OTA non completato: simulazione azzerata."); });
    ArduinoOTA.begin();
    Serial.println("OTA protetto attivo sulla porta 3232.");
  } else Serial.println("OTA disabilitato: configurare una password privata valida.");
#endif
  startAttempt();
  server.on("/", HTTP_GET, [](){ server.sendHeader("Cache-Control", "no-store"); server.send_P(200, "text/html; charset=utf-8", DASHBOARD_HTML); });
  server.on("/api/status", HTTP_GET, [](){ sendStatus(); });
  server.on("/api/command", HTTP_POST, [](){
#if defined(ESP32)
    if (otaUpdating) { server.send(503, "text/plain", "Aggiornamento in corso"); return; }
#endif
    if (!server.hasArg("command")) { sendStatus(400); return; } bool ok = state.command(server.arg("command").c_str(), millis()); updateFanRelays(state.fans); sendStatus(ok ? 200 : 409); });
  server.onNotFound([](){ server.send(404, "text/plain", "Non trovato"); });
  server.begin();
#if defined(VENTO_ENABLE_FAN_RELAYS)
  Serial.println("VENTO: relè ventole fisici GPIO 22/23; pompa, coil e valvole simulati.");
#else
  Serial.println("VENTO: simulazione, nessuna uscita hardware attiva.");
#endif
}
void loop() {
#if defined(ESP32)
  ArduinoOTA.handle();
#endif
  server.handleClient(); state.tick(millis()); updateFanRelays(state.fans); updateWifi();
#if defined(ESP8266)
  if (mdnsStarted) MDNS.update();
#endif
  yield();
}
