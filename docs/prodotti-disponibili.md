# SIB — prodotti disponibili

Dati ricavati dai link condivisi e dalla descrizione fornita. Non sostituiscono schede tecniche o verifica dei componenti.

| Prodotto | Identificazione | Possibile uso | Da verificare |
|---|---|---|---|
| [B0GV6C4WC1](https://www.amazon.it/dp/B0GV6C4WC1) | Gebildet, 14 moduli di pilotaggio MOSFET doppi, pubblicizzati 5–36 V DC, PWM 0–20 kHz, 400 W. Nome fornito dall'utente: pagina non accessibile alla verifica. | Comando ON/OFF della pompa DC, LED o valvole DC compatibili. | Tensione e corrente reali dei carichi, MOSFET e collegamenti del modulo, compatibilità affidabile a 3,3 V, protezione induttiva. Due MOSFET non implicano due uscite indipendenti. Limiti pubblicizzati non sono valori garantiti per ogni condizione di raffreddamento. |
| [B0DPMQWR45](https://www.amazon.it/dp/B0DPMQWR45) | Gebildet, modulo relè a due canali, bobina 5 V, trigger alto/basso. La foto mostra relè SRD-05VDC-SL-C. | Possibile comando di un circuito di consenso/contattore adatto. | Soglia ingresso a 3,3 V, polarità, carico e isolamento complessivo; i 10 A stampati sul relè non garantiscono idoneità per quattro motori. |
| [B09NDHJFYC](https://www.amazon.it/dp/B09NDHJFYC) | Regolatore AC manuale con potenziometro, coerente con il MY-9892 indicato dall'utente. | Possibile regolazione manuale ventole, solo se compatibile con i motori specifici. | Conferma dal produttore delle ventole, dimensionamento motori e protezioni. Non collegare i fili del potenziometro all'ESP. |

Le pagine Amazon non sono accessibili direttamente alla verifica. L'identificazione dei secondi due è coerente con altre inserzioni associate agli stessi codici, ma non costituisce una scheda tecnica del produttore. La pompa e la coil restano da identificare. Il firmware ESP32 resta in simulazione e non comanda le uscite.

## Pompa aria filetti: Bewinner B0BJZYZCHD

L'utente ha fornito il titolo: pompa a vuoto/aria DC, titolo generale 3,7–24 V, portata pubblicizzata 2,5–8 L/min. Un'[altra inserzione associata allo stesso ASIN](https://www.odkarla.cz/bewinner-dc-37-24v-mikro-vzduchove-cerpadlo-elektricke-mini-vzduchove-cerpadlo-60-kpa-100~p1834326) riporta la variante 12 V, ma la pagina Amazon non era accessibile e non è stata verificata la variante dell'ordine. Confermare nella ricevuta/confezione prima di alimentare. Non considerare l'intervallo 3,7–24 V come sicuro per ogni singola pompa.

Restano sconosciute corrente, corrente di spunto, polarità, compatibilità PWM, continuità di funzionamento e compatibilità con aerosol caldo/condensa. La portata pubblicizzata non è una misura effettiva nel sistema. Il modello e il modulo XY-MOS vanno verificati con protezioni idonee prima di un cablaggio reale. La coil richiede un'alimentazione e protezioni definite separatamente.
