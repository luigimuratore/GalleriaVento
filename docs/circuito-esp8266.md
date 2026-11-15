# VENTO — bozza circuito con ESP8266MOD

Aggiornamento: D0 comanda i LED ON/OFF. L’ingresso di stato non è previsto per ora; D3 resta libero come possibile opzione futura da verificare per l’avvio. Vedi [mappa pin](../schemi/circuito-pin.svg) ed [esempio DC](../schemi/circuito-dc.svg).

Questa è una bozza funzionale. Non è uno schema di cablaggio esecutivo. Le tensioni e le correnti dei carichi non sono ancora note. Non acquistare regolatori, alimentatori o protezioni di potenza prima di verificare le targhe.

## 1. Cosa deve fare il circuito

1. Accendere le quattro ventole.
2. Avviare la pompa dell'aria per i filetti.
3. Dare il consenso al generatore di nebbia solo se il flusso è presente.
4. Comandare il compressore e le valvole per dirigere l'aria sopra o sotto l'ala.
5. Accendere la striscia LED.
6. Portare i carichi in uno stato sicuro quando si preme l'arresto fisico o si presenta un guasto.

## 2. Schema a blocchi

[Apri lo schema grafico con i dispositivi](../schemi/schema-funzionale.svg). Mostra i collegamenti funzionali. Non mostra morsetti, cavi o protezioni dettagliate.

```text
                      RETE 230 V AC
                           │
              sezionatore + protezioni
                           │
        ┌──────────────────┴──────────────────┐
        │                                     │
 contattore per motori                 alimentatore certificato
        │                               230 V AC → DC
 regolatore compatibile                        │
 con le ventole                               ├── DC → driver pompa → pompa filetti
        │                                     ├── DC → driver → LED
   4 VENTOLE                                  ├── DC → driver → valvola SOPRA NC
                                              ├── DC → driver → valvola SOTTO NC
                                              ├── DC → comando compressore, se DC
                                              ├── DC → driver protetto tracciante
                                              └── convertitore → 3,3 V stabile
                                                                  │
                                                              ESP8266MOD
                                                                  │
                                                  GPIO4 / GPIO5 → espansore I/O
                                                                  │
                                  comandi driver + lettura consensi e sensori

 ARRESTO FISICO → catena hardware indipendente → disabilitazione energie pericolose
 SENSORE FLUSSO + PROTEZIONE TERMICA → catena indipendente → consenso riscaldatore
 TERRA DI PROTEZIONE → parti metalliche e apparecchi che la richiedono
```

Lo schema mostra le funzioni, non i collegamenti dei morsetti. Se il compressore funziona a 230 V, serve un comando separato adatto al suo motore. La tensione DC può essere 12 V o 24 V solo dopo la scelta dei carichi. Se i carichi hanno tensioni diverse, servono rami e alimentatori appropriati.

## 3. Controllore: proposta senza espansore

La foto mostra una scheda di formato **D1 mini**, con modulo ESP8266MOD. La revisione esatta non è certa dalla sola foto. La tabella usa la mappatura tipica D1 mini: verifica i collegamenti sulla tua scheda prima di usarla.

Si può realizzare una versione essenziale senza espansore. La proposta usa due valvole indipendenti, LED comandati da D0 e nessun ingresso di stato per ora. Il compressore ha un proprio comando, soggetto alla verifica del circuito pneumatico. Per quattro valvole indipendenti, controllo LED via app e diagnostica separata servono più segnali o altre scelte progettuali.

| Pin D1 mini | GPIO | Uso proposto | Nota |
|---|---:|---|---|
| D1 | 5 | Consenso ventole | Tramite driver e contattore adatti. |
| D2 | 4 | Pompa filetti | Tramite driver adatto alla pompa. |
| D5 | 14 | Richiesta generatore nebbia | Il consenso hardware indipendente resta obbligatorio. |
| D6 | 12 | Valvola sopra | Tramite driver protetto. |
| D7 | 13 | Valvola sotto | Tramite driver protetto. |
| D8 | 15 | Richiesta compressore | Pin di avvio: il driver deve mantenere GPIO15 basso al boot. Da verificare con la scheda reale. |
| D0 | 16 | LED ON/OFF | Tramite driver DC adeguato alla striscia. |

Questa è un'assegnazione preliminare, non uno schema di cablaggio. D0 comanda i LED in ON/OFF tramite un driver adatto. D3 e D4 restano liberi perché i loro livelli influenzano l'avvio; D4 può anche essere collegato al LED della scheda. D8 richiede particolare attenzione: se il circuito esterno lo porta alto all'accensione, la scheda può non avviarsi. TX e RX restano liberi per programmazione e diagnostica. I segnali sono a 3,3 V. Non applicare 5 V ai GPIO.

I pin non alimentano i carichi. Servono driver compatibili e livelli di riposo definiti con componenti esterni. Il consenso di potenza deve restare disabilitato durante accensione, reset e blocco del programma. Prevedere protezioni indipendenti dal software e un watchdog esterno adeguato. Un unico ingresso di stato informa l'ESP che la catena è disponibile; non sostituisce la catena hardware né permette di identificare singolarmente i guasti.

La pompa può partire in modalità ON/OFF. Eventuale PWM va verificato con pompa e driver reali. L'ESP8266 genera il PWM via software: frequenze elevate e Wi-Fi condividono le risorse del processore. Non assumere che due frequenze indipendenti siano disponibili senza modifiche o componenti aggiuntivi.

## 4. Distinta preliminare

| Componente | Quantità | Criterio di scelta |
|---|---:|---|
| Scheda con ESP8266MOD | 1 | Già disponibile. Identificare la scheda esatta. |
| Espansore I/O | 0 | Non previsto nella versione essenziale a due valvole. |
| Alimentatore AC/DC certificato | 1 o più | Uscite e corrente in base ai carichi, incluse correnti di spunto e margine. |
| Convertitore DC/DC | 1 o più | Tensione di ingresso e uscita compatibili con la scheda. Alimentazione stabile e adatta ai picchi Wi-Fi. |
| Sezionatore, protezioni e arresto fisico | Da definire | Scelta e dimensionamento secondo impianto e analisi dei rischi. |
| Contattore ventole | 1 | Categoria e corrente compatibili con motori e spunto complessivo. |
| Regolatore ventole | Da definire | Solo del tipo espressamente ammesso dal costruttore delle quattro ventole. |
| Driver pompa DC | 1 | Compatibile con 3,3 V, tensione e corrente della pompa; protezione per carico induttivo. |
| Driver LED DC | 1 | Compatibile con 3,3 V e potenza totale della striscia. |
| Driver valvole | 2 oppure 4 | Uno per ogni comando indipendente; tensione e corrente delle bobine compatibili. |
| Soppressione transitori | Per ogni bobina/motore | Diodo o altro circuito idoneo per corrente, tensione e tempi di rilascio richiesti. |
| Comando compressore | 1 se necessario | In base a tensione, corrente di spunto e tipo di motore. |
| Valvole pneumatiche normalmente chiuse | 2 oppure 4 | Scelta in base allo schema dell'ala e alla pressione effettiva. |
| Regolatore pressione e manometro | 1 gruppo | Compatibili con compressore, pressione e portata. |
| Limitazione pressione e scarico | Da definire | In base ai componenti e all'analisi dei rischi. |
| Sensore di flusso aria | 1 | Adatto a portata, pressione e segnale; funzione verificabile. |
| Protezione termica indipendente | Da definire | Compatibile con il generatore effettivo e capace di interromperne la potenza. |
| Generatore di nebbia | 1 | Preferibile un sistema completo con fluido previsto dal costruttore. |
| Fusibili o protezioni per i rami DC | Da definire | In base a cavi, carichi e alimentatori. |
| Contenitore, morsetti, cablaggio, pressacavi | Da definire | Adatti alle tensioni, alle correnti e all'ambiente di impiego. |

I moduli relè e MOSFET già citati possono essere valutati solo con il codice preciso e la scheda tecnica. Non assumere che siano compatibili con 3,3 V o adatti ai carichi indicati. L'etichetta «optoisolato» non garantisce isolamento effettivo nell'intero circuito.

## 5. Pneumatica: idea iniziale

```text
compressore → filtro/regolatore → manometro → collettore
                                                 ├→ valvola NC SOPRA → condotto sopra l'ala
                                                 └→ valvola NC SOTTO → condotto sotto l'ala
```

In neutro le due valvole sono chiuse. Sopra e sotto non devono aprirsi insieme se il circuito non lo consente. Prevedere un interblocco fisico oltre al software quando l'apertura simultanea può creare pericolo. La necessità di due o quattro valvole dipende dalla costruzione dell'ala. Va anche definito dove si scarica l'aria in sicurezza e come si limita la pressione quando le valvole sono chiuse.

## 6. Punti aperti prima del cablaggio

- Revisione della scheda D1 mini e verifica della corrispondenza dei pin.
- Modello, tensione, potenza e tipo di motore di ogni ventola.
- Tensione, corrente e modello di pompa e compressore.
- Modello delle valvole, tensione delle bobine e pressione di lavoro.
- Tensione, potenza e lunghezza della striscia LED.
- Tipo di generatore di nebbia. Se si vuole usare la coil: resistenza, potenza ammessa, alimentatore, fluido e protezioni disponibili.
- Schema dei condotti nell'ala e condizioni di ventilazione della sala.

## 7. Limiti importanti

Non collegare la coil direttamente a 12 V senza aver calcolato e verificato corrente e potenza. Non considerare il timer da 40 secondi una protezione contro il funzionamento a secco. Non assumere che lo spurgo di 3 secondi elimini tutta la condensa. Un arresto di emergenza può interrompere anche lo spurgo, se necessario per togliere energia al sistema.

Un MOSFET DC generico non è un regolatore per ventole a 230 V. Un regolatore SCR è utilizzabile solo se compatibile con i motori specifici. La sezione a 230 V, la terra e le protezioni richiedono progetto e verifica da parte di personale qualificato.

## Fonti

- Espressif: [datasheet ESP8266EX](https://documentation.espressif.com/0a-esp8266ex_datasheet_en.html).
- Espressif: [indicazioni hardware ESP8266](https://documentation.espressif.com/esp8266_hardware_design_guidelines_en.html).
- ebm-papst: [FAQ sul controllo dei motori AC](https://www.ebmpapst.com/us/en/support/faq.html).
- CDC: [informazioni sugli aerosol delle sigarette elettroniche](https://www.cdc.gov/tobacco/e-cigarettes/about.html).

- WEMOS: [mappatura pin D1 mini](https://docs.wemos.cc/en/latest/d1/d1_mini_3.1.0.html).
- ESP8266 Arduino Core: [PWM e ingressi/uscite](https://arduino-esp8266.readthedocs.io/en/latest/reference.html).

## Possibile aggiunta futura: ingresso di stato

Per ora non colleghiamo un ingresso di stato. In futuro si possono mantenere due uscite per le valvole, D0 per i LED ON/OFF e valutare D3 (GPIO0) come ingresso di stato cumulativo. D3 deve restare alto durante l'avvio normale: un guasto che lo porti basso all'accensione può impedire l'avvio. L'interfaccia deve mantenere il livello corretto durante boot e consentire la lettura dello stato solo dopo, senza applicare più di 3,3 V al GPIO. È un'ipotesi da progettare e verificare sulla scheda e sui contatti reali, non un collegamento diretto pronto. In caso di dubbio si può tornare a un espansore.

L'ESP deve considerare lo stato non valido finché l'ingresso non è effettivamente disponibile e verificato. Nessun carico pericoloso deve avviarsi in automatico dopo reset. La catena fisica deve fermare le energie pericolose indipendentemente dall'ESP: D3 ne riporta solo lo stato e non costituisce un circuito di sicurezza.
