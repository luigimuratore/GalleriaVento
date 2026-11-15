# VENTO — preparazione delle prove con componenti reali

## Stato

L'ESP installato ospita solo la dashboard di simulazione. Nessun pin pilota carichi. La nuova bozza `firmware/vento8266/output_plan.h` traduce gli stati della dashboard in richieste di uscita, ma non è collegata ai GPIO né inclusa nel firmware installato. La richiesta di riscaldare la coil resta sempre disabilitata.

## Mappa provvisoria

| Pin D1 mini | GPIO | Funzione proposta | Da verificare |
|---|---:|---|---|
| D1 | 5 | Consenso ventole | Driver, polarità e contattore motore |
| D2 | 4 | Pompa filetti | Driver, polarità, tensione e corrente |
| D5 | 14 | Richiesta coil | Driver dedicato e protezioni indipendenti |
| D6 | 12 | Valvola sopra | Driver, bobina, polarità e interblocco |
| D7 | 13 | Valvola sotto | Driver, bobina, polarità e interblocco |
| D8 | 15 | Consenso compressore | Driver e livello basso al boot |
| D0 | 16 | LED ON/OFF | Driver e alimentatore |

Questa tabella non autorizza il cablaggio. D8 influisce sull'avvio. Non è noto se i moduli abbiano ingresso attivo alto o basso, né se siano compatibili con 3,3 V.

## Preparazione software

La struttura separa tre livelli:

1. Dashboard: invia la richiesta.
2. Macchina a stati: verifica ordine, direzione e tempi simulati.
3. Piano delle uscite: indica le uscite richieste, senza pilotare pin. La coil resta bloccata.

Mancano ancora il driver GPIO reale, inizializzazione e polarità, monitoraggio delle condizioni fisiche, gestione di guasti e perdita controllo, watchdog indipendente, autenticazione dei comandi e gestione dei LED nella dashboard. L'API attuale è priva di autenticazione. Non collegarla a carichi reali. Il software non sostituisce l'arresto fisico o le protezioni indipendenti.

## Sequenza delle prove da pianificare

1. Identificare ogni componente, leggere le targhe e verificare l'idoneità dei moduli.
2. Definire alimentazione, protezioni, terra, circuiti di interblocco e stato sicuro al reset.
3. A carichi scollegati, testare i livelli dei segnali con strumenti e carichi di prova adeguati. Verificare accensione, reset, perdita di alimentazione e blocco.
4. Collaudare alimentazione e parte a 230 V con personale qualificato.
5. Collaudare pneumatica, limiti di pressione, scarico e valvole.
6. Progettare e collaudare la coil solo dopo la verifica dei consensi di flusso e termici indipendenti. Il timer software non basta.

## Dati da raccogliere

Foto e sigle di ventole, pompa, compressore, elettrovalvole, coil, LED, alimentatori, moduli relè e MOSFET; tensioni, correnti, potenze, polarità e pressioni; come sono collegati i condotti sopra e sotto l'ala. La dashboard LED separata non è ancora stata realizzata.

Riferimenti: [pin D1 mini](https://docs.wemos.cc/en/latest/d1/d1_mini_3.1.0.html), [indicazioni hardware ESP8266](https://documentation.espressif.com/esp8266_hardware_design_guidelines_en.html).
