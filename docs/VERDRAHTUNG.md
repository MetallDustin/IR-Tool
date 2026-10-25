# Verdrahtung

## Pinübersicht

| D1 mini Pro | Funktion | Anschluss |
|---|---|---|
| `D5` / GPIO14 | IR-Empfang | OUT des IR-Empfängers |
| `D6` / GPIO12 | IR-Sende-Steuerung | 1-kΩ-Widerstand zur Transistorbasis |
| `3V3` | Versorgung Empfänger | VCC des IR-Empfängers |
| `5V`/`VU` | Versorgung LED-Stufe | LED-Vorwiderstand, nur bei USB-Versorgung |
| `G` / GND | gemeinsame Masse | Empfänger-GND und Transistor-Emitter |

## IR-Empfänger

Beim Empfänger sind normalerweise drei Anschlüsse vorhanden:

```text
Empfänger VCC  -> 3V3
Empfänger GND  -> GND
Empfänger OUT  -> D5
```

Die Reihenfolge `OUT/GND/VCC` ist nicht bei allen Modulen gleich. Die Beschriftung oder das Datenblatt des konkreten Moduls hat Vorrang.

## IR-Sendestufe mit NPN-Transistor

```text
D6 ── 1 kΩ ──> Basis        2N2222 / BC337
                 Kollektor ── Kathode IR-LED
GND ──────────── Emitter

5V/VU ── 220 Ω ──> Anode IR-LED
```

Wichtig:

- `5V/VU` ist nur dann vorhanden, wenn das Board über USB versorgt wird.
- Der Transistor muss zur Pinbelegung des konkreten Bauteils passen; bei 2N2222 und BC337 kann die Anschlussreihenfolge unterschiedlich sein.
- Die IR-LED-Anode ist die längere LED-Seite; die Kathode ist meist abgeflacht markiert.
- GND vom Empfänger, ESP8266 und Transistor muss gemeinsam verbunden sein.
- Den GPIO nicht direkt mit der LED verbinden.

## Alternative mit 3,3-V-Versorgung

Wenn die Sendestufe vollständig mit 3,3 V betrieben wird, kann der LED-Vorwiderstand ebenfalls an `3V3` angeschlossen werden. Die LED ist dann meist weniger stark. Der 220-Ω-Widerstand ist ein konservativer Startwert; bei einer leistungsstärkeren LED muss der zulässige Dauer- und Pulsstrom aus dem Datenblatt beachtet werden.

## Schaltplan

Der grafische Schaltplan liegt als [`schematic.svg`](schematic.svg) im Repository.

