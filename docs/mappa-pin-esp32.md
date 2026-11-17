# SAFRAN — Freenove ESP32-WROOM: mappa preliminare

[Apri lo schema aggiornato della millefori e dei morsetti](../schemi/millefori-esp32-morsetti.svg). I disegni precedenti degli altri circuiti potrebbero non riflettere questa configurazione.

Il disegno è una mappa logica: non mostra la posizione fisica dei pin. Questa mappa è una proposta per un ESP32 classico con modulo WROOM. Occorre confrontarla con la revisione esatta della scheda Freenove prima di collegare i fili. Le etichette sono numeri **GPIO**, non i vecchi D0–D8 della D1 mini.

| GPIO proposto | Funzione | Collegamento previsto |
|---|---|---|
| 22 | ARIA · ventole 1 e 2 | Comando scheda relè 1, attivo HIGH |
| 23 | ARIA · ventole 3 e 4 | Comando scheda relè 2, attivo HIGH |
| 27 | FUMO · pompa | TRIG/PWM del MOSFET montato sulla millefori, da verificare |
| 18 | FUMO · coil | Comando driver montato sulla millefori, con protezioni indipendenti |
| 32 | FLUSSI · sopra | Driver valvola normalmente chiusa |
| 33 | FLUSSI · sotto | Driver valvola normalmente chiusa |
| — | Compressore e LED | Gestione indipendente, nessun collegamento di controllo all’ESP32 |

I due segnali ventole sono comandati insieme dalla dashboard. La build sperimentale `freenove_esp32_ventole` comanda solo questi relè; tutte le altre funzioni restano simulate. Le velocità sono regolate manualmente. GPIO 34–39 sono solo ingressi sull'ESP32 classico; GPIO 0, 2, 5, 12 e 15 influenzano l'avvio. Pin flash non disponibili. Consultare il [pinout Freenove](https://github.com/Freenove/Freenove_ESP32_WROOM_Board) e la [documentazione Espressif](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/gpio.html).

## Stato del firmware

La build `freenove_esp32_simulazione` ospita la dashboard, prova le reti configurate e simula gli stati. **Non configura né pilota questi GPIO**. La coil non viene alimentata e non sono letti sensori reali. Il vecchio firmware simulato ESP8266 rimane disponibile nell'ambiente `d1_mini_simulazione`; il codice comune è `firmware/vento8266/vento8266.ino`, ora compilabile per entrambe le famiglie.

Per caricare sulla Freenove, con tutti i carichi e driver scollegati:

```sh
python3 tools/embed_dashboard.py
pio run -e freenove_esp32_simulazione
pio device list
pio run -e freenove_esp32_simulazione -t upload --upload-port PORTA
pio device monitor -b 115200
```

Sostituisci PORTA con la porta rilevata. Le reti e la password dell'access point rimangono nel file locale ignorato da Git `firmware/vento8266/secrets.h`. Se mancante, copiare `secrets.example.h` e configurarlo. La scheda stampa l'indirizzo IP sulla seriale. L'indirizzo IP può differire da quello della vecchia scheda.

Prima di un firmware con uscite reali occorre identificare driver e polarità, alimentatori, protezioni indipendenti, condizioni sicure ai reset, monitoraggio e verifica dell'avvio, arresto fisico, compatibilità del regolatore MY-9892 con i motori, flusso e protezione termica coil. L'ESP32 usa logica 3,3 V e non alimenta i carichi dai GPIO. La parte a 230 V va progettata e verificata da personale qualificato.

Il disegno a 16 poli è una bozza precedente, non definitiva: si sta valutando di saldare direttamente i fili di potenza e usare un solo segnale per ciascuna scheda relè. La nuova disposizione deve ancora essere verificata. Il driver della coil è provvisorio: compatibilità e correnti non sono ancora note. LED e compressore sono esclusi dal controllo ESP.

**Prova relè:** prima dell’upload scollegare i carichi, in particolare la rete 230 V. Verificare polarità HIGH, stato di ogni relè ad accensione/reset e collegamento comune. Il firmware non garantisce lo stato durante reset o interruzione dell’alimentazione: servono interfacce e protezioni hardware idonee.
