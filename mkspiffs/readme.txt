Пишем файлы в образ
mkspiffs -c files -b 4096 -p 256 -s 0xF0000 spiffs1.bin
Заливаем образ в память
esptool --chip esp32 --port COM5 --baud 921600 write_flash -z 0x110000 spiffs1.bin
Считываем из памяти образ
esptool -b 921600 --port COM5 read_flash 0x110000 0xF0000 spiffs1.bin
Читаем файлы из образа
mkspiffs -u files spiffs1.bin