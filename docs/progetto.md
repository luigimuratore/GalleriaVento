# VENTO — progetto preliminare

> Aggiornamento: il controllore disponibile è ESP8266MOD. Per il circuito e la distinta aggiornati consulta [circuito-esp8266.md](circuito-esp8266.md). Le indicazioni ESP32 sotto sono superate.

## Esperienza

Il telefono si collega al Wi-Fi dedicato dell'ESP32 e apre l'indirizzo del dispositivo, ad esempio `http://192.168.4.1`, non il proprio `localhost`. Nessuna app da installare; predisporre QR e telefono già connesso. La pagina inclusa è soltanto una demo locale. Per l'evento servono server HTTP, interfaccia, autenticazione, API e test integrati.

Sequenza: 1 ventole, 2 filetti fluidi, 3 attuazione. Spegnimento in ordine inverso; l'arresto d'emergenza prevale sempre. Attuazione: sopra / neutro / sotto, con valvole normalmente chiuse e interblocco. Un singolo pulsante di arresto fisico deve essere accessibile all'operatore. LED scenografici indipendenti, accesi già all'arrivo dell'ospite.

## Schema funzionale — non schema esecutivo

```text
RETE 230 V~ → sezionamento + protezioni dimensionate → alimentatore certificato 12/24 V DC
           └→ contattore idoneo al carico motore → regolatore idoneo ai motori → 4 ventole
PE ─────────────────────────────────────────────→ parti metalliche / motori dove previsto

12/24 V DC → protezioni per ramo → convertitore 5 V → scheda ESP32 (regolazione 3,3 V)
          ├→ driver DC protetto → pompa aria tracciante
          ├→ alimentatore/driver dedicato e protetto → generatore tracciante
          ├→ driver protetti + soppressione transitori → valvole NC sopra / sotto
          ├→ contattore o driver idoneo → compressore, se necessario
          └→ driver MOSFET DC → LED

ARRESTO FISICO + catena di consenso indipendente → disabilitazione potenze pericolose
Sensori / feedback → ESP32 → logica sequenziale e segnalazioni
Compressore → filtro / regolatore / manometro / limitazione pressione → valvole NC → ala
```

L'arresto di emergenza va progettato in base all'analisi dei rischi: la funzione non può dipendere da Wi-Fi, software o singolo relè economico. L'arresto d'emergenza interrompe le energie pericolose immediatamente, anche durante lo spurgo; per le energie pneumatiche prevedere scarico sicuro e prevenire movimenti inattesi. Protezioni, messa a terra, cavi, segregazione tensioni, involucro e test devono essere dimensionati da un elettricista qualificato.

### Ventole 230 V

Un MOSFET DC generico non è un regolatore per rete AC. Il regolatore SCR/triac è utilizzabile **solo se il costruttore della ventola lo consente** e va dimensionato per corrente di spunto e carico motore. Alcuni motori richiedono comando diverso, altri non sono regolabili; in alternativa usare ventole EC con ingresso 0–10 V/PWM compatibile, o funzionamento a velocità fissa. Separare consenso ON/OFF e regolazione. Senza marca, modello, targa e tipo di motore non si può definire lo schema né scegliere un regolatore.

### Tracciante e riscaldatore

Preferire un generatore di nebbia professionale con fluido specificato dal produttore, valutato per l'ambiente e con aspirazione/ventilazione adeguata. Aerosol da sigaretta elettronica non equivale a vapore acqueo innocuo; evitare nicotina e aromi e verificare scheda di sicurezza e condizioni di esposizione. L'atomizzatore da svapo è ancora da caratterizzare: resistenza, tensione, corrente, liquido, portata e temperatura. Non alimentare una coil direttamente a 12 V: la corrente può essere molto elevata. Occorrono driver adatto alla coil e alla potenza, limitazione di corrente, fusibile, protezione termica indipendente, controllo di flusso e verifica del liquido; nessun valore è ricavabile dai dati disponibili.

Il timer software di 40 s è soltanto un limite preliminare, non rileva un dry hit, non sostituisce sensori e non garantisce sicurezza. Lo spurgo di 3 s è un valore iniziale da validare, non elimina necessariamente la condensa. Attivare il riscaldatore solo dopo un tempo di pre-flusso e con consenso di flusso verificato; se manca il flusso o interviene una protezione, spegnere immediatamente il riscaldatore. In assenza di sensori, mantenere il riscaldatore disabilitato. Lo spurgo ordinario mantiene la pompa per 3 s dopo OFF o timeout, salvo emergenza o guasto che richiedano arresto immediato.

### Pompa, LED e valvole

Verificare sulle schede tecniche che pompa e relativo driver ammettano PWM alla frequenza scelta: 16–20 kHz e kick-start 150 ms non sono universali. Se non specificato usare ON/OFF e testare. Il diodo di ricircolo va dimensionato per corrente e tensione della bobina/motore; 1N4007 non è automaticamente adeguato. Proteggere anche i solenoidi secondo le indicazioni del costruttore. LED: PWM indicativamente 1–5 kHz, verificando driver, riprese e assenza di flicker. Non dare per scontata la compatibilità 3,3 V dei moduli Gebildet o la polarità del jumper: misurare e consultare il modello esatto; optoisolamento e alimentazioni separate non sono garantiti dal nome commerciale.

Due valvole NC possono selezionare sopra/sotto se il circuito pneumatico lo permette; quattro dipendono dal modo in cui sono disposti ingressi, scarichi e attuatori. Prevedere interblocco elettrico e software se l'apertura simultanea è incompatibile con l'impianto. Regolatore, valvola di sovrapressione e pressione di esercizio dipendono dai componenti. I tempi di commutazione e l'eventuale scarico vanno stabiliti sul circuito effettivo.

## Firmware e collegamenti logici proposti

ESP32 dev board, alimentazione stabile, livelli logici 3,3 V; non confondere ESP32 con ESP8266: GPIO 26/27 non esistono su ESP8266. I pin effettivi si assegnano solo dopo scelta della scheda e verifica di boot, PWM e driver. Uscite previste: consenso ventole, pompa, consenso riscaldatore, compressore se necessario, valvole sopra/sotto, LED. Ingressi: catena emergenza/feedback, flusso, protezione termica, eventuale livello, feedback ventole e pressione. Gli ingressi di feedback non sostituiscono la catena hardware.

Tutti i comandi partono disabilitati anche durante reset, avvio e perdita alimentazione mediante circuiti esterni opportuni. Nessun riavvio automatico dopo reset/guasto. Un comando ON va rifiutato se non sono soddisfatti sequenza e consensi. In caso di perdita del controllo prevedere un timeout di sessione configurabile e una strategia definita in analisi dei rischi; il browser non deve essere la protezione. Stati e interblocchi devono essere autorevoli nel firmware, non solo nella pagina.

### Contratto API proposto, non ancora implementato

- `GET /api/status`: stato effettivo, fase, direzione, consensi, allarmi, tempo residuo.
- `POST /api/command`: `{ "command": "fans_on" | "smoke_on" | "actuation_on" | "upper" | "lower" | "neutral" | "actuation_off" | "smoke_off" | "fans_off" }`; restituire errore se l'ordine o i consensi non sono rispettati.
- Credenziali Wi-Fi uniche, controllo di accesso per i comandi, protezione da richieste non autorizzate e nessuna esposizione su Internet. Il solo Wi-Fi protetto non è una sicurezza funzionale.

## Componenti da scegliere

ESP32 dev board; alimentatore/i certificati dimensionati; protezioni, sezionatore, arresto fisico e contattore; quattro ventole e relativi regolatori compatibili; generatore tracciante completo o driver coil progettato; pompa e sensore flusso; sensori e protezioni termiche; compressore, regolatore, manometro, limitatore pressione e 2/4 valvole NC; driver protetti per motori/valvole/LED; striscia LED e diffusori; involucro, morsetti, cablaggio, canaline e PE. Quantità e valori dipendono dalle targhe.

## Verifica e messa in servizio

1. Collaudo macchina a stati e interfaccia senza carichi, anche con reset, timeout, doppio comando e richieste fuori sequenza.
2. Verifica a banco dei driver con carichi equivalenti e accensione sicura durante boot/reset.
3. Verifica elettrica e protezioni da personale qualificato; test arresto fisico e assenza di riavvio inatteso.
4. Collaudo pneumatico a pressione controllata; verificare neutro, interblocchi e tenuta.
5. Solo dopo valutazione del tracciante e protezioni indipendenti, testare portata, temperatura, spurgo e visibilità con ventilazione adeguata.
6. Prova completa con telefono, perdita Wi-Fi, assenza flusso, timeout e piano di intervento dell'operatore.

## Informazioni necessarie

Modello e dati di targa delle quattro ventole; modello di ESP e scheda disponibile; modello della pompa e del compressore, tensione, corrente e pressione; specifiche e schema delle elettrovalvole; coil (resistenza, potenza prevista e alimentatore), composizione del liquido e ambiente/ventilazione; tipo di LED, lunghezza e potenza; data e luogo dell'evento; eventuale esempio del precedente sistema «forno».

## Riferimenti

- Espressif, [ESP8266EX datasheet](https://documentation.espressif.com/0a-esp8266ex_datasheet_en.html): differenze GPIO e limiti PWM.
- ebm-papst, [FAQ sul controllo dei motori AC](https://www.ebmpapst.com/us/en/support/faq.html): il metodo dipende dal motore.
- CDC, [aerosol delle sigarette elettroniche](https://www.cdc.gov/tobacco/e-cigarettes/about.html): non è semplice vapore acqueo innocuo.
