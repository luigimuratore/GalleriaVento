# VENTO — preparazione e sopralluogo

## A casa

- Prepara D1 mini, cavo USB dati, computer e telefono. Controlla che la scheda sia riconosciuta dal computer. La dashboard attuale è una demo locale; il firmware ESP8266 con Wi-Fi e controlli non è ancora pronto.
- Se gli header non sono saldati, puoi saldare i connettori della D1 mini. Verifica l'orientamento e lascia la scheda estraibile. Evita saldature provvisorie sui pin del modulo.
- Prepara una millefori vuota o con i soli zoccoli per l'ESP, senza collegamenti permanenti ai carichi. Verifica ingombri e fori. Attendi le misure e i moduli prima di definire morsetti e cablaggio.
- Prepara etichette: ventole, pompa, coil, valvola sopra, valvola sotto, compressore, LED. Porta i disegni del progetto.
- Raccogli in un contenitore i moduli già acquistati e le loro confezioni o schede tecniche.

## Cosa portare

- D1 mini, cavo USB dati, computer e caricatore; telefono.
- Metro, pennarello, nastro di carta, quaderno, fascette.
- Millefori, header, breadboard e cavetti jumper per prototipi a bassa tensione.
- Multimetro funzionante e puntali integri, solo per le misure che sai fare in sicurezza.
- Se utili per il montaggio a bassa tensione: saldatore, stagno, pinza, tronchesino e spellafili.
- Moduli già acquistati, viti e distanziali, se disponibili.

## Foto e dati da raccogliere

- Fotografa ogni targa: quattro ventole, pompa, compressore, valvole, alimentatori e LED. Registra modello, tensione, corrente o potenza, e pressione per la pneumatica.
- Coil: modello, resistenza dichiarata in ohm, potenza raccomandata e alimentatore previsto. Foto dei liquidi e scheda di sicurezza.
- Moduli relè e MOSFET: foto fronte e retro, sigla, etichette e morsetti.
- Fotografa quadro elettrico, prese disponibili, percorso dei cavi e zona di installazione; fai verificare alimentazione, terra e protezioni a personale qualificato.
- Misura vano per elettronica, posizione dell'ala, lunghezze dei tubi, distanza dei componenti e percorso dei cavi. Annota come saranno fissati e protetti.
- Annota posizione della striscia LED, apertura per la manutenzione, ventilazione della sala e posizione dell'arresto fisico.
- Verifica dove sono i condotti sopra e sotto l'ala e quali valvole sono necessarie. Non mettere in pressione un circuito non verificato.

## Prima di alimentare i carichi

- Scegliere i driver in base ai componenti reali e verificare logica a 3,3 V. Il GPIO15/D8 deve restare basso all'avvio.
- Progettare alimentatori, fusibili, terra, comando motori e protezioni indipendenti. Verificare lo stato sicuro anche al reset.
- Definire il circuito della coil e il tracciante. Non collegare la coil direttamente a 12 V sulla base dell'ipotesi del disegno.
- Collaudare prima il solo controllo con carichi simulati a bassa tensione. Non mettere 230 V su breadboard o jumper da prototipo.
