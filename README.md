Les sons se trouvent a la racine de la carte SD.
Ils doivent etre au format MP3, en mono et de preference en debit constant 32Kbits/s (pour garder des fichiers peu volumineux).

Il est possible de mettre a jour la carte en compilant le contenu du dossier "source" et en placant le fichier binaire a l'emplacement "/system/update.bin".

Developpement effectue avec Arduino, carte ESP32 Dev Module (esp32, https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json pour les cartes).
Ce programme utilise la bibliotheque ESP32-audioI2S (disponible ici https://github.com/schreibfaul1/ESP32-audioI2S).