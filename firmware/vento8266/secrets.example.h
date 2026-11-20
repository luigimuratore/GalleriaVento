#pragma once
struct WifiCredential { const char* ssid; const char* password; };
const WifiCredential NETWORKS[] = {
  {"RETE_1", "PASSWORD_1"}, {"RETE_2", "PASSWORD_2"}, {"RETE_3", "PASSWORD_3"}
};
const char AP_SSID[] = "Active Flow Control Bench";
const char AP_PASSWORD[] = "INSERISCI_PASSWORD_UNICA";
