# Galleria Vento

Galleria del vento con ESP8266 D1 mini e dashboard web locale. Il progetto prevede quattro ventole, filetti fluidi, due valvole per l'ala e una striscia LED.

**Stato attuale: simulazione.** L'ESP ospita la dashboard e simula i comandi. Non pilota pin, relè, motori, valvole o coil. Non collegare carichi alla scheda per questa prova.

## Progetti e documenti

- [Firmware ESP8266](firmware/vento8266/vento8266.ino): Wi-Fi, server web e comandi simulati.
- [Dashboard predefinita](dashboard/index.html): tema PaletteAziendale, apribile nel browser anche senza ESP.
- [PaletteAziendale](dashboard/PaletteAziendale.html): copia nominata del tema attuale.
- [Dashboard v1](dashboard/v1.html): versione precedente, conservata come anteprima indipendente.
- [Preparazione delle prove reali](docs/prove-reali.md): pin proposti, blocchi e verifiche prima dei collegamenti.
- [Millefori e collegamenti](schemi/millefori-posizione.svg).
- [Mappa dei pin](schemi/circuito-pin.svg) e [esempio carico DC](schemi/circuito-dc.svg): bozze, non schemi esecutivi.
- [Circuito spiegato](docs/circuito-chiaro.html), [componenti](docs/circuito-esp8266.md) e [lista per il sopralluogo](docs/sopralluogo.md).
- `firmware/GalleriaVento/`: vecchia bozza ESP32. Non è il firmware installato.

## Caricare e accedere

### Cosa serve

- Una D1 mini con ESP8266 e un cavo USB dati.
- [PlatformIO](https://platformio.org/install) e Python 3 sul computer.
- Un telefono con browser e Wi-Fi.

Durante il primo collaudo lascia scollegati relè e carichi. Apri un terminale nella cartella principale del progetto.

### 1. Configura le reti

La prima volta copia il file di esempio:

```sh
cp firmware/vento8266/secrets.example.h firmware/vento8266/secrets.h
```

Apri `firmware/vento8266/secrets.h` e inserisci le reti Wi-Fi da provare. Imposta anche una password privata per la rete `VENTO-SETUP`, lunga almeno 8 caratteri. Su questo computer il file è già configurato: non sovrascriverlo se vuoi mantenere le impostazioni attuali.

`secrets.h` è escluso da Git. Anche il firmware compilato e caricato sulla scheda contiene le credenziali: non condividere il file o il binario. Cambia le password se sospetti che siano state divulgate.

### 2. Incorpora la dashboard

Esegui questo comando dopo ogni modifica a `dashboard/index.html` o a `tools/embed_dashboard.py`:

```sh
python3 tools/embed_dashboard.py
```

La dashboard predefinita usata dal firmware è `dashboard/index.html`. Quando modifichi il tema PaletteAziendale, aggiorna anche `dashboard/PaletteAziendale.html` se vuoi mantenerne la copia nominata sincronizzata.

Il comando genera `firmware/vento8266/dashboard_embedded.h`. Il file è escluso da Git. La dashboard viene caricata insieme al firmware: non serve caricare un filesystem separato.

### 3. Compila e carica via USB

```sh
pio run -e d1_mini_simulazione
pio run -e d1_mini_simulazione -t upload
```

Se PlatformIO non trova la scheda, elenca le porte e specifica quella corretta:

```sh
pio device list
pio run -e d1_mini_simulazione -t upload --upload-port /dev/cu.usbserial-PORTA
```

Sostituisci `/dev/cu.usbserial-PORTA` con la porta trovata. Chiudi eventuali monitor seriali prima dell'upload. Il caricamento sostituisce il firmware presente sulla scheda. Gli aggiornamenti OTA via Wi-Fi non sono ancora disponibili.

### 4. Apri la dashboard

Apri il monitor seriale:

```sh
pio device monitor -b 115200
```

Se non vedi i messaggi iniziali, premi brevemente il tasto reset della scheda. L'ESP prova in sequenza le reti configurate. Quando si collega, la seriale mostra l'indirizzo da aprire nel browser. Il telefono deve essere sulla stessa rete. L'indirizzo può cambiare dopo un riavvio o una riconnessione.

L'ESP crea anche la rete diretta `VENTO-SETUP`. La password è nel file locale `secrets.h`. Collegati a questa rete e apri **http://192.168.4.1/**. Se il telefono segnala che la rete non ha Internet, resta connesso. La rete diretta può cambiare canale quando l'ESP si collega a un'altra rete.

La dashboard e i comandi funzionano senza Internet. I font esterni possono essere sostituiti automaticamente dai font del dispositivo. Non serve un account. Chi raggiunge la dashboard può inviare comandi simulati: usa una rete attendibile.

## Comandi simulati

1. **Ventole:** accende le quattro ventole simulate.
2. **Filetti fluidi:** avvia pompa e coil simulate. La coil ha un limite di 40 secondi.
3. **Aerodinamica:** abilita neutro, sopra e sotto. È attiva una sola direzione alla volta.
4. Per spegnere, segui l'ordine inverso. I filetti scompaiono subito. Lo spurgo della pompa resta simulato per 3 secondi; durante lo spurgo puoi comandare ARIA, mentre FUMO resta bloccato fino al termine.

Un reset della scheda riporta tutti gli stati simulati a spento. Il timer usa il tempo trascorso dall'accensione; NTP non è usato. L'animazione e gli stati della pagina non dimostrano il funzionamento di dispositivi reali.

## Verifica

Compila con `pio run`. Per verificare la macchina a stati senza una scheda:

```sh
c++ -std=c++11 -Wall -Wextra -pedantic tests/state_test.cpp -o /tmp/vento-state-test
/tmp/vento-state-test
```

Il test copre la sequenza, i comandi fuori ordine, le direzioni, il timer della coil, lo spurgo e il passaggio oltre il limite del contatore del tempo. Non sostituisce il collaudo hardware.

## Prima di collegare i carichi

Servono componenti identificati, un progetto elettrico e pneumatico, driver adatti e protezioni indipendenti dal firmware. Le ventole a 230 V e la coil richiedono particolare attenzione. Il timer di 40 secondi non rileva il funzionamento a secco. Lo schema a 12 V nei documenti è solo un esempio. La sezione a 230 V e le relative protezioni devono essere progettate e verificate da personale qualificato.
