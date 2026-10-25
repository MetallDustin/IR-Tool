#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <LittleFS.h>
#include <IRrecv.h>
#include <IRsend.h>
#include <IRutils.h>

// Anschlüsse des D1 mini Pro. Bei einer anderen Verdrahtung hier ändern.
const uint16_t IR_RECEIVE_PIN = D5;  // GPIO14: OUT des IR-Empfängers
const uint16_t IR_SEND_PIN    = D6;  // GPIO12: Basiswiderstand der Sendestufe

// Eigenes WLAN des ESP8266. Das Passwort muss mindestens 8 Zeichen lang sein.
// Für ein anderes WLAN-Namen oder Passwort nur diese beiden Werte ändern.
const char* AP_SSID = "ESP-IR";
const char* AP_PASSWORD = "12345678";  // mindestens 8 Zeichen

// Maximale Anzahl aufgezeichneter Impulse. Erhöhen benötigt mehr RAM.
const uint16_t RAWBUF_SIZE = 1024;

// Anzahl der dauerhaft verfügbaren Speicherplätze.
// Das Webinterface ist aktuell auf 5 sichtbare Slot-Tasten ausgelegt.
const uint8_t SLOT_COUNT = 5;

// Sendedauer: 2000 ms = 2 Sekunden. Für z. B. 5 Sekunden auf 5000 ändern.
// Das Signal wird während dieser Zeit immer wieder als vollständiger IR-Rahmen gesendet.
const uint16_t SEND_DURATION_MS = 2000;

// Pause zwischen zwei IR-Rahmen. Kleiner = häufigere Wiederholung,
// größer = mehr Abstand. 35 ms entspricht vielen Fernbedienungen gut.
const uint16_t SEND_GAP_MS = 35;

// Kennung und Größe des Dateikopfs der LittleFS-Slots.
const uint32_t SLOT_MAGIC = 0x49525332;  // "IRS2"
const size_t SLOT_HEADER_SIZE = sizeof(uint32_t) + sizeof(uint16_t) +
                                sizeof(uint16_t) + sizeof(uint16_t) +
                                sizeof(uint16_t) + sizeof(uint64_t);
uint16_t rawData[RAWBUF_SIZE];
uint16_t rawLength = 0;
uint16_t carrierFrequency = 38;
bool recording = false;
bool persistent = false;
uint8_t activeSlot = 0;
String protocolName = "unbekannt";
decode_type_t decodedType = UNKNOWN;
uint64_t decodedValue = 0;
uint16_t decodedBits = 0;

IRrecv irrecv(IR_RECEIVE_PIN);
IRsend irsend(IR_SEND_PIN);
decode_results results;
ESP8266WebServer server(80);

const char MAIN_PAGE[] PROGMEM = R"rawliteral(
<!doctype html>
<html lang="de">
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width,initial-scale=1">
  <title>ESP-IR</title>
  <style>
    *{box-sizing:border-box}
    body{font-family:-apple-system,BlinkMacSystemFont,"Segoe UI",Arial,sans-serif;max-width:390px;margin:0 auto;padding:env(safe-area-inset-top) 10px env(safe-area-inset-bottom);background:#000;color:#fff}
    .card{background:#181818;border-radius:18px;padding:16px;margin:8px 0;box-shadow:0 3px 14px #0008}
    h1{font-size:25px;margin:2px 0 4px} p{margin:4px 0 14px;color:#bbb}
    button{width:100%;min-height:54px;padding:12px;margin:6px 0;border:0;border-radius:13px;font-size:17px;font-weight:600;cursor:pointer;touch-action:manipulation}
    #record{background:#1677ff;color:white} #send{background:#20a050;color:white} #save{background:#805ad5;color:white}
    button:disabled{background:#aaa!important;cursor:not-allowed}
    #status{padding:11px;background:#292929;border-radius:11px;margin:12px 0;white-space:pre-line;font-size:14px;line-height:1.35}
    #details{font-size:12px;color:#bbb;line-height:1.4;margin:8px 2px 12px;min-height:17px}
    .slots{display:grid;grid-template-columns:repeat(5,1fr);gap:5px;margin:8px 0 12px}
    .slot{min-height:44px;padding:4px;margin:0;background:#333;color:#fff;font-size:14px;border-radius:10px}
    .slot.active{background:#fff;color:#000}
    small{display:block;color:#999;font-size:12px;line-height:1.35;margin-top:10px}
    @media (max-width:260px){body{padding-left:6px;padding-right:6px}.card{padding:12px}button{font-size:15px}.slots{gap:3px}.slot{font-size:12px}}
  </style>
</head>
<body>
  <div class="card">
    <h1>ESP-IR</h1>
    <p>IR-Signale aufnehmen und wieder senden</p>
    <div>Speicherplatz:</div>
    <div class="slots">
      <button class="slot" id="slot0" onclick="changeSlot(0)">1</button>
      <button class="slot" id="slot1" onclick="changeSlot(1)">2</button>
      <button class="slot" id="slot2" onclick="changeSlot(2)">3</button>
      <button class="slot" id="slot3" onclick="changeSlot(3)">4</button>
      <button class="slot" id="slot4" onclick="changeSlot(4)">5</button>
    </div>
    <div id="status">Status wird geladen …</div>
    <div id="details"></div>
    <button id="record" onclick="recordSignal()">IR-Signal aufnehmen</button>
    <button id="save" onclick="saveSignal()" disabled>Signal dauerhaft speichern</button>
    <button id="send" onclick="sendSignal()" disabled>Gespeichertes Signal senden</button>
    <small>Drücke „Aufnehmen“ und anschließend eine Taste auf der Fernbedienung.</small>
  </div>
  <script>
    async function update(){
      const s=await (await fetch('/api/status')).json();
      for(let i=0;i<5;i++) document.getElementById('slot'+i).classList.toggle('active',i==s.slot);
      document.getElementById('status').textContent=s.recording?'Warte auf IR-Signal …':
        (s.saved?(s.persistent?'Dauerhaft gespeichert':'Aufgenommen, noch nicht dauerhaft gespeichert')+
        ' ('+s.length+' Impulse)':'Noch kein Signal gespeichert');
      document.getElementById('details').textContent=s.saved?
        'Protokoll: '+s.protocol+'  ·  Frequenz: '+s.frequency+' kHz  ·  Länge: '+s.length+' Impulse  ·  Sendedauer: '+(s.sendDuration/1000)+' s':'';
      document.getElementById('send').disabled=!s.saved||s.recording;
      document.getElementById('save').disabled=!s.saved||s.recording||s.persistent;
      document.getElementById('record').disabled=s.recording;
    }
    async function recordSignal(){await fetch('/record',{method:'POST'});update();}
    async function saveSignal(){await fetch('/save',{method:'POST'});update();}
    async function sendSignal(){await fetch('/send',{method:'POST'});update();}
    async function changeSlot(n){
      await fetch('/slot?n='+n,{method:'POST'}); update();
    }
    setInterval(update,1000); update();
  </script>
</body>
</html>
)rawliteral";

void handleRoot() {
  server.send_P(200, "text/html; charset=utf-8", MAIN_PAGE);
}

void handleStatus() {
  String json = "{\"recording\":" + String(recording ? "true" : "false") +
                ",\"saved\":" + String(rawLength > 0 ? "true" : "false") +
                ",\"persistent\":" + String(persistent ? "true" : "false") +
                ",\"slot\":" + String(activeSlot) +
                ",\"length\":" + String(rawLength) +
                ",\"frequency\":" + String(carrierFrequency) +
                ",\"protocol\":\"" + protocolName + "\"" +
                ",\"sendDuration\":" + String(SEND_DURATION_MS) + "}";
  server.send(200, "application/json", json);
}

void handleRecord() {
  rawLength = 0;
  persistent = false;
  protocolName = "unbekannt";
  decodedType = UNKNOWN;
  decodedValue = 0;
  decodedBits = 0;
  recording = true;
  Serial.println("Warte auf IR-Signal ...");
  server.send(200, "text/plain", "Aufnahme gestartet");
}

String slotPath(uint8_t slot) {
  return "/irslot" + String(slot) + ".bin";
}

bool loadSlot(uint8_t slot) {
  File file = LittleFS.open(slotPath(slot), "r");
  if (!file || file.size() < SLOT_HEADER_SIZE) return false;

  uint32_t magic = 0;
  uint16_t storedLength = 0;
  uint16_t storedFrequency = 38;
  uint16_t storedType = UNKNOWN;
  uint16_t storedBits = 0;
  uint64_t storedValue = 0;
  file.read((uint8_t*)&magic, sizeof(magic));
  file.read((uint8_t*)&storedLength, sizeof(storedLength));
  file.read((uint8_t*)&storedFrequency, sizeof(storedFrequency));
  file.read((uint8_t*)&storedType, sizeof(storedType));
  file.read((uint8_t*)&storedBits, sizeof(storedBits));
  file.read((uint8_t*)&storedValue, sizeof(storedValue));
  if (magic != SLOT_MAGIC) {
    file.close();
    return false;
  }
  if (storedLength == 0 || storedLength >= RAWBUF_SIZE ||
      file.available() < (int)(storedLength * sizeof(uint16_t))) {
    file.close();
    return false;
  }

  rawLength = storedLength;
  carrierFrequency = storedFrequency;
  decodedType = (decode_type_t)storedType;
  decodedBits = storedBits;
  decodedValue = storedValue;
  protocolName = typeToString(decodedType);
  file.read((uint8_t*)rawData, rawLength * sizeof(uint16_t));
  file.close();
  persistent = true;
  return true;
}

void saveSlot(uint8_t slot) {
  File file = LittleFS.open(slotPath(slot), "w");
  if (!file) {
    persistent = false;
    return;
  }
  uint32_t magic = SLOT_MAGIC;
  file.write((uint8_t*)&magic, sizeof(magic));
  file.write((uint8_t*)&rawLength, sizeof(rawLength));
  file.write((uint8_t*)&carrierFrequency, sizeof(carrierFrequency));
  uint16_t storedType = (uint16_t)decodedType;
  file.write((uint8_t*)&storedType, sizeof(storedType));
  file.write((uint8_t*)&decodedBits, sizeof(decodedBits));
  file.write((uint8_t*)&decodedValue, sizeof(decodedValue));
  file.write((uint8_t*)rawData, rawLength * sizeof(uint16_t));
  file.close();
  persistent = true;
}

void handleSlot() {
  if (!server.hasArg("n")) {
    server.send(400, "text/plain", "Slot fehlt");
    return;
  }
  int requested = server.arg("n").toInt();
  if (requested < 0 || requested >= SLOT_COUNT) {
    server.send(400, "text/plain", "Ungueltiger Slot");
    return;
  }
  activeSlot = requested;
  rawLength = 0;
  protocolName = "unbekannt";
  persistent = loadSlot(activeSlot);
  server.send(200, "text/plain", "Slot geladen");
}

void handleSave() {
  if (rawLength == 0) {
    server.send(400, "text/plain", "Kein Signal zum Speichern");
    return;
  }
  saveSlot(activeSlot);
  Serial.println("IR-Signal dauerhaft im Slot gespeichert.");
  server.send(200, "text/plain", "Signal dauerhaft gespeichert");
}

void handleSend() {
  if (rawLength == 0) {
    server.send(400, "text/plain", "Kein Signal gespeichert");
    return;
  }

  Serial.println("Sende IR-Signal ...");
  uint32_t sendStart = millis();
  do {
    bool nativeSent = true;
    switch (decodedType) {
      case SAMSUNG:
        irsend.sendSAMSUNG(decodedValue, decodedBits, 0);
        break;
      case SAMSUNG36:
        irsend.sendSamsung36(decodedValue, decodedBits, 0);
        break;
      case EPSON:
        irsend.sendEpson(decodedValue, decodedBits, 0);
        break;
      default:
        nativeSent = false;
        break;
    }
    if (!nativeSent) {
      irsend.sendRaw(rawData, rawLength, carrierFrequency);
    }
    if (millis() - sendStart < SEND_DURATION_MS) {
      delay(SEND_GAP_MS);
    }
  } while (millis() - sendStart < SEND_DURATION_MS);
  server.send(200, "text/plain", "Signal " + String(SEND_DURATION_MS) + " ms gesendet");
}

void setup() {
  Serial.begin(115200);
  delay(100);

  irrecv.enableIRIn();
  irsend.begin();
  if (!LittleFS.begin()) {
    Serial.println("Fehler: LittleFS konnte nicht gestartet werden.");
  }
  loadSlot(activeSlot);

  WiFi.mode(WIFI_AP);
  WiFi.softAP(AP_SSID, AP_PASSWORD);

  server.on("/", HTTP_GET, handleRoot);
  server.on("/api/status", HTTP_GET, handleStatus);
  server.on("/record", HTTP_POST, handleRecord);
  server.on("/save", HTTP_POST, handleSave);
  server.on("/slot", HTTP_POST, handleSlot);
  server.on("/send", HTTP_POST, handleSend);
  server.begin();

  Serial.println();
  Serial.println("ESP-IR gestartet");
  Serial.print("WLAN: ");
  Serial.println(AP_SSID);
  Serial.print("Passwort: ");
  Serial.println(AP_PASSWORD);
  Serial.println("Webinterface: http://192.168.4.1");
}

void loop() {
  server.handleClient();

  if (recording && irrecv.decode(&results)) {
    uint16_t length = results.rawlen - 1;

    if (length < RAWBUF_SIZE) {
      for (uint16_t i = 1; i < results.rawlen; i++) {
        rawData[i - 1] = results.rawbuf[i] * kRawTick;
      }
      rawLength = length;
      recording = false;
      persistent = false;
      protocolName = typeToString(results.decode_type);
      decodedType = results.decode_type;
      decodedValue = results.value;
      decodedBits = results.bits;
      Serial.println("IR-Signal gespeichert.");
    }

    irrecv.resume();
  }
}

