#!/usr/bin/env python3
"""Carica via OTA senza inserire la password nella riga di comando digitata."""
import argparse
import os
from pathlib import Path
import re
import socket
import subprocess
import sys

parser = argparse.ArgumentParser(description='Carica sull’ESP32 un firmware già compilato.')
parser.add_argument('--host', default='bench.local', help='Nome locale o IP della scheda')
parser.add_argument('--environment', choices=['freenove_esp32_simulazione', 'freenove_esp32_ventole', 'freenove_esp32_valvole', 'freenove_esp32_sequenza_banco'], default='freenove_esp32_simulazione', help='Build ESP32 già compilata da caricare')
args = parser.parse_args()
root = Path(__file__).resolve().parent.parent
config = root / 'firmware/vento8266/ota_config.h'
try:
    contents = config.read_text()
except OSError:
    sys.exit('Manca firmware/vento8266/ota_config.h. Consulta il README.')
match = re.search(r'const\s+char\s+OTA_PASSWORD\[\]\s*=\s*"([A-Za-z0-9_-]{16,64})"', contents)
if not match or match.group(1) == 'SOSTITUISCI_CON_PASSWORD_PRIVATA':
    sys.exit('Configura una password OTA privata da 16 a 64 caratteri: lettere, numeri, _ o -.')
password = match.group(1)
core = Path(os.environ.get('PLATFORMIO_CORE_DIR', str(Path.home() / '.platformio')))
espota = core / 'packages/framework-arduinoespressif32/tools/espota.py'
image = root / '.pio/build' / args.environment / 'firmware.bin'
if not espota.is_file() or not image.is_file():
    sys.exit('Manca il programma OTA o il firmware compilato. Compila prima con PlatformIO.')
try:
    address = socket.gethostbyname(args.host)
except OSError:
    sys.exit('Nome non risolto. Usa --host seguito dall’indirizzo IP della scheda.')
print('Caricamento OTA in corso...', flush=True)
cmd = [sys.executable, str(espota), '-i', address, '-p', '3232', '-a', password, '-f', str(image)]
result = subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True)
output = result.stdout.replace(password, '[nascosta]')
if result.returncode:
    print(output, end='')
    sys.exit('Caricamento OTA non riuscito. Verifica rete, porta e password.')
print('Caricamento OTA riuscito. La scheda si riavvia.')
