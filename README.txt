Прошиваем Wireless Stick Lite V3 из Heltec ESP32 Boards
Прошивка 2.0rc4 от 09.04.24
В MyParkGate_V2 добавлен мьютекс на отправку по ИК. Теперь при отправке ИП блокируются задачи приема HTTP, NTP, 
  отправки HTTP,Neopixel и приема по LoRa

Прошивка 2.0rc2 от 29.12.23
В серверной версии добавлен мьютекс на отправку по ИК. Теперь при отправке ИП блокируются задачи приема HTTP, NTP, отправки HTTP и Neopixel

Прошивка 2.0rc2 от 13.12.23
1. MyParkGate_V2 и MyParkServer_V2
1.1 Обработка кнопки(геркона) только длинных нажатий более 1 сек
1.2 Добавден счетчик нажатий
   - Первое нажатие у MyParkGate_V2 чистится сохраняемы список сенсоров, у MyParkServer_V2 чистится список подключенных шлюзов
   - Второе нажатие - режим точки доступа WiFi
   - Третье нажатие - перезагрузка
   - Если ничего не нажимаем минуту - то сброс счетчика нажатий в нуль 
1.3 При старте на табло посылается сперва 0, потом число восстанавливаемое из списка сенсоров и шлюзов
2. MyParkServer_V2
2.1 Сохраняется список шлюзов по аналогии с сенсорами в MyParkGate_V2

Прошивка 2.0rc1 от 12.12.23
Версия с карманным табло и общим табло
Настройка системы.docx