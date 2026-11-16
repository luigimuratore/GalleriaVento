# SIB — schema generale ESP32

[Apri il disegno dello schema generale](../schemi/schema-generale-esp32.svg).

[Apri la bozza dedicata a ESP32, pompa e coil](../schemi/esp32-pompa-coil.svg).

[Apri la bozza dedicata a ESP32, compressore e valvole](../schemi/esp32-compressore-valvole.svg).

Il disegno è una mappa funzionale. Non indica un cablaggio esecutivo: i collegamenti di potenza e le protezioni vanno scelti in base a componenti, dati di targa e verifica sul posto. L'ESP caricato è in simulazione e non attiva le uscite.

## Le tre linee

| Linea | Ipotesi e uso | Cosa manca |
|---|---|---|
| 230 V AC | Ingresso di rete per alimentatore DC e, tramite protezioni e contattore adatto, le quattro ventole. Il MY-9892 è solo una possibilità, subordinata alla compatibilità del motore. | Progetto di sezionamento, protezioni, terra, contattore, cablaggio e conferma per il regolatore. |
| 12 V DC | Le due strisce LED sono dichiarate a 12 V. Un alimentatore 12 V può essere usato se la sua targa e la sua corrente sono adeguate. | Potenza delle strisce; modelli di alimentatore e driver; fusibili e cavi. **Pompa e valvole non sono confermate a 12 V.** |
| 5 V DC | Proposta: convertitore idoneo dalla linea DC per la scheda relè a bobina 5 V e per la scheda ESP32 tramite il suo ingresso previsto. | Verificare il modello della scheda, la capacità del convertitore, l'architettura della scheda relè e la gestione di USB e alimentazione esterna contemporanee. I GPIO restano a 3,3 V. |

Gli alimentatori possono essere separati se i carichi lo richiedono. I collegamenti di PE e delle protezioni non sono riportati morsetto per morsetto nello schema funzionale.

## Comandi proposti

| Dashboard | GPIO | Segnale richiesto | Componente |
|---|---:|---|---|
| ARIA | 25 | ON/OFF | Interfaccia relè rosso → contattore adeguato → ventole. |
| FUMO | 26 | ON/OFF | Driver XY-MOS per la pompa, **solo dopo identificazione della pompa e del driver**. |
| FUMO | 27 | Richiesta ON/OFF | Driver della coil ancora da scegliere, subordinato a consensi fisici. |
| FLUSSI | 32 | Sopra | Driver della prima elettrovalvola, da scegliere in base alla bobina. |
| FLUSSI | 33 | Sotto | Driver della seconda elettrovalvola, da scegliere in base alla bobina. |
| FLUSSI | 18 | Compressore, se controllato | Driver o contattore adeguato al compressore. |
| Luci | 19 | ON/OFF comune proposto | Driver adeguato per LED 1 e LED 2. La dashboard non ha ancora un comando luci. |

GPIO 19 può comandare entrambe le strisce insieme. Se la configurazione delle strisce lo consente, i due rami sono in parallelo sull'alimentazione corretta, non in serie. Driver, cavi e protezioni vanno dimensionati sulla **somma** delle correnti; verificare la protezione di ogni ramo. Per accenderle in modo indipendente servono due comandi e due uscite di potenza idonee.

Sui moduli XY-MOS identificati dall'utente sono riportati `VIN+`, `VIN-`, `OUT+`, `OUT-`, `TRIG/PWM` e `GND`. Un modulo con queste etichette usa generalmente VIN per l'alimentazione DC, OUT per il carico, TRIG per il segnale e GND per il riferimento. La funzione e l'eventuale massa comune vanno confermate sul modello reale, a circuito disalimentato. Verificare comando affidabile a 3,3 V, protezioni e corrente, prima di usare tali morsetti. Non usare XY-MOS sulla rete 230 V.

## Coil, pompa e valvole

- **Coil:** non scegliere una tensione sulla base dei 12 V dei LED. Servono resistenza, tensione e potenza previste, alimentatore e driver adeguati. Il calore deve dipendere da consenso di flusso e protezione termica **indipendenti** dal firmware. Il timer di 40 secondi non rileva il funzionamento a secco.
- **Pompa:** modello e dati elettrici ignoti. Prima di collegarla servono tensione, corrente e verifica dell'idoneità all'aspirazione di aerosol e condensa. Prevedere soppressione di transitori adatta al motore.
- **Festo:** senza codici e tensione bobina non sappiamo se siano adatte ai 12 V né quali siano i loro porti pneumatici. Le valvole potrebbero essere di tipi diversi; neutro e scarico dipendono dal modello. Festo documenta bobine per varie tensioni, fra cui 12 V e 24 V DC e versioni AC: [esempio ufficiale](https://www.festo.com/media/catalog/277929_documentation.pdf).
- **Compressore:** definire pressione massima, regolatore, manometro, protezione contro sovrapressione e scarico sicuro prima delle prove. Definire un interblocco hardware se aperture simultanee o mancato scarico possono creare pericolo.

## Verifiche prima del cablaggio

1. Foto di targa di alimentatore, pompa, coil e bobine Festo; sigle e morsetti dei moduli; potenza per metro e lunghezza di ciascuna striscia.
2. Conferma del costruttore delle ventole per il MY-9892. Non collegare il potenziometro o i suoi fili all'ESP.
3. Progetto elettrico qualificato: rete 230 V, terra, sezionamento, contattore, fusibili, cavi, arresto e separazione tra tensioni.
4. Scelta di driver, alimentatori e protezioni; verifica dei 3,3 V di ingresso e dello stato spento durante boot, reset e guasti.
5. Firmware con comandi reali e verifiche a banco senza carichi pericolosi. Il firmware attuale non configura le uscite e l'API demo non è ancora adatta ai carichi reali.
