# Inbetriebnahme und Funktionstest

## Vor dem Einschalten

- [ ] IR-Empfänger ist an 3V3, GND und D5 angeschlossen.
- [ ] D6 geht nur über den Basiswiderstand zum Transistor.
- [ ] IR-LED besitzt einen Vorwiderstand.
- [ ] Transistor-Pinbelegung wurde anhand des Datenblatts geprüft.
- [ ] Alle GND-Anschlüsse sind verbunden.

## Firmware-Test

1. Sketch kompilieren.
2. Auf den D1 mini Pro hochladen.
3. Seriellen Monitor mit 115200 Baud öffnen.
4. Prüfen, ob `ESP-IR gestartet` erscheint.
5. Mit `ESP-IR` verbinden und `http://192.168.4.1` öffnen.

## Aufnahme-Test

1. Slot 1 auswählen.
2. **IR-Signal aufnehmen** drücken.
3. Eine Fernbedienungstaste in Richtung Empfänger drücken.
4. Prüfen, ob Protokoll, Frequenz und Impulszahl angezeigt werden.
5. **Signal dauerhaft speichern** drücken.

## Speicher-Test

1. Einen anderen Slot auswählen.
2. Zurück auf Slot 1 wechseln.
3. Prüfen, ob das gespeicherte Signal wieder geladen wird.
4. ESP8266 neu starten.
5. Slot 1 erneut öffnen und prüfen, ob das Signal noch vorhanden ist.

## Sende-Test

1. Das Zielgerät auf die IR-LED ausrichten.
2. **Gespeichertes Signal senden** drücken.
3. Das Zielgerät muss reagieren, als wäre die Fernbedienungstaste ungefähr zwei Sekunden gehalten worden.
4. Bei schlechtem Empfang Abstand und Ausrichtung prüfen.

## Fehlerbehebung

| Problem | Prüfung |
|---|---|
| WLAN erscheint nicht | Reset drücken, seriellen Monitor prüfen, Versorgung kontrollieren |
| Aufnahme bleibt leer | Empfänger-Pinfolge, GND und Abstand zur Fernbedienung prüfen |
| Signal wird gespeichert, Gerät reagiert aber nicht | LED-Polung, Transistor-Pinfolge, Vorwiderstand und Ausrichtung prüfen |
| Nur kurze Aktion statt Haltefunktion | Zielgerät kann Wiederholungen anders auswerten; Raw-Signal neu aufnehmen |
| Slots sind nach Update leer | Signal neu aufnehmen; alte Slot-Dateien können inkompatibel sein |
| Kompilierfehler mit `IRrecv`/`IRsend` | ESP8266-Board und `IRremoteESP8266`-Bibliothek prüfen |

