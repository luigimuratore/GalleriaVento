#include <Arduino.h>
#if defined(ESP32)
#include <WiFi.h>
#include <WebServer.h>
#include <ESPmDNS.h>
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

// SIMULAZIONE: nessun GPIO e nessun carico reale viene comandato.
VentoWebServer server(80);
VentoState state;
const size_t networkCount = sizeof(NETWORKS) / sizeof(NETWORKS[0]);
size_t wifiIndex = 0;
unsigned long lastAttempt = 0;
bool wasConnected = false;
bool mdnsStarted = false;
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
  const char* direction = state.direction == VentoState::UPPER ? "upper" : state.direction == VentoState::LOWER ? "lower" : "neutral";
  String body = "{\"fans\":" + String(state.fans ? "true" : "false") + ",\"smoke\":" + (state.smoke ? "true" : "false") + ",\"act\":" + (state.actuation ? "true" : "false") + ",\"purging\":" + (state.purging ? "true" : "false") + ",\"direction\":\"" + direction + "\",\"simulation\":true}";
  server.sendHeader("Cache-Control", "no-store");
  server.send(status, "application/json", body);
}
void setup() {
  Serial.begin(115200);
  WiFi.persistent(false); WiFi.mode(WIFI_AP_STA); WiFi.setAutoReconnect(false);
  if (WiFi.softAP(AP_SSID, AP_PASSWORD)) { Serial.print("Rete di emergenza: "); Serial.print(AP_SSID); Serial.print(" http://"); Serial.println(WiFi.softAPIP()); }
  else Serial.println("Errore avvio access point");
  startAttempt();
  server.on("/", HTTP_GET, [](){ server.sendHeader("Cache-Control", "no-store"); server.send_P(200, "text/html; charset=utf-8", DASHBOARD_HTML); });
  server.on("/api/status", HTTP_GET, [](){ sendStatus(); });
  server.on("/api/command", HTTP_POST, [](){ if (!server.hasArg("command")) { sendStatus(400); return; } bool ok = state.command(server.arg("command").c_str(), millis()); sendStatus(ok ? 200 : 409); });
  server.onNotFound([](){ server.send(404, "text/plain", "Non trovato"); });
  server.begin();
  Serial.println("VENTO: modalita simulazione, nessuna uscita hardware attiva.");
}
void loop() {
  server.handleClient(); state.tick(millis()); updateWifi();
#if defined(ESP8266)
  if (mdnsStarted) MDNS.update();
#endif
  yield();
}
