# ESP-IR Webinterface

Webinterface für einen ESP8266 D1 mini Pro zum Aufnehmen, dauerhaften Speichern und Senden von Infrarot-Signalen.

## Hardware

- ESP8266 D1 mini Pro
- IR-Empfänger an `D5`
- IR-LED über Transistor an `D6`

## Funktionen

- eigenes WLAN `ESP-IR`
- Bedienung im Browser, auch auf kleinen Displays
- bis zu fünf dauerhaft gespeicherte Signal-Slots
- Anzeige von Protokoll, Frequenz und Impulsanzahl vor dem Speichern
- Samsung, Samsung36 und Epson mit nativem Protokoll
- Raw-Signale als Fallback
- jedes gespeicherte Signal wird beim Senden ungefähr zwei Sekunden lang wiederholt

## Arduino IDE

1. ESP8266-Boardpaket installieren.
2. Bibliothek `IRremoteESP8266` installieren.
3. `esp_ir_webinterface.ino` öffnen.
4. Board `LOLIN(WEMOS) D1 mini Pro` auswählen.
5. Sketch kompilieren und hochladen.

Nach dem Start mit dem WLAN `ESP-IR` verbinden. Das Passwort ist `12345678`. Danach im Browser `http://192.168.4.1` öffnen.

## Verdrahtung

Die IR-LED darf nicht direkt am GPIO betrieben werden. Für die Sendestufe einen geeigneten Transistor mit Vorwiderstand verwenden und eine gemeinsame Masse zwischen ESP, Empfänger und Sendestufe herstellen.

