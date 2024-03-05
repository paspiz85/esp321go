
### Compilazione
Dalla cartella del progetto:
```
arduino-cli compile -e -m esp32
```

### Caricamento
Individuare la porta a cui è collegata la scheda:
```
arduino-cli board list
```
Se la board non compare potrebbe essere necessario installare il driver per Windows e riavviare il sistema.

Dalla cartella del progetto:
```
arduino-cli upload -m esp32 -i build/esp32.esp32.esp32/esp321go.ino.bin -p <porta>
```
Se l'upload fallisce sempre e l'errore è "ESP32: Timed out waiting for packet header" potrebbe essere necessario tenere premuto BOOT durante l'upload.

### Debug
Dalla cartella del progetto:
```
arduino-cli monitor -c baudrate=115200 -p <porta>
```

### Configurazione iniziale arduino-cli
Su Linux (e compatibili) aggiungere arduino-cli nella variabile PATH:
```
PATH=$PATH:$(realpath tools)
```
Inizializzazione configurazione arduino-cli con impostazioni standard:
```
arduino-cli config init
```
Inizializzazione configurazione arduino-cli con impostazioni personalizzate:
```
arduino-cli config init --dest-file arduino/arduino-cli.yaml
```
In tutti i comandi successivi bisognerà usare l'opzione ```--config-file arduino/arduino-cli.yaml```.
Infine aggiornare l'indice alla versione più recente:
```
arduino-cli core update-index
```

### Installazione di board e librerie
Sia la board e che le librerie dovrebbero installarsi alla prima compilazione del progetto, altrimenti:
```
arduino-cli core install esp32:esp32@1.0.6
arduino-cli lib install -v PubSubClient:2.8.0
```
