# VENTO — firmware ESP8266

La prima bozza è in `vento8266/`. È una simulazione: non configura né pilota GPIO o carichi. La dashboard, servita dalla scheda, invia comandi a una macchina a stati che gira sull'ESP. Il firmware non è adatto al comando di hardware reale.

La scheda prova a turno le reti configurate. Mantiene anche un access point `VENTO-SETUP` con password casuale in `vento8266/secrets.h`; l'indirizzo dell'access point è normalmente `http://192.168.4.1`. L'indirizzo ottenuto sulla rete locale viene stampato sulla seriale a 115200 baud e può cambiare. La password e le credenziali delle reti sono salvate anche nel firmware caricato sulla scheda e vanno protette. Il file `secrets.h` e i prodotti di compilazione sono esclusi da Git; non condividerli. L'access point non sostituisce autenticazione dell'app: usare solo reti attendibili per questa demo. La funzione NTP non è usata perché il timer non richiede l'ora di rete.

Dalla radice del progetto:

1. Creare `firmware/vento8266/secrets.h` da `secrets.example.h` e inserire credenziali e password AP univoca.
2. Eseguire `python3 tools/embed_dashboard.py` per incorporare l'HTML aggiornato.
3. Compilare e caricare con PlatformIO e ambiente `d1_mini_simulazione`.
4. Aprire la seriale a 115200 baud per leggere l'indirizzo di rete.

I filetti scompaiono subito allo spegnimento. Il timer di 40 secondi e lo spurgo di 3 secondi sono simulazioni. Per i carichi reali servono progetto e verifica della catena di sicurezza indipendente, driver, protezioni, interblocchi e collaudi. L'ESP non verifica i sensori.

Test della logica senza hardware: `c++ -std=c++11 -Wall -Wextra -pedantic tests/state_test.cpp -o /tmp/vento-state-test && /tmp/vento-state-test`.

`GalleriaVento/` è la vecchia bozza ESP32 e non è quella installata.

Per la build ESP32, l’aggiornamento OTA autenticato è disponibile dopo la prima installazione USB. Consulta la sezione OTA nel [README principale](../README.md). La build ESP8266 non abilita OTA.
