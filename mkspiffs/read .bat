esptool -b 1500000 --port COM14 read_flash 0x110000 0xF0000 spiffs.bin & mkspiffs -u files spiffs1.bin & mkspiffs -u files spiffs.bin

