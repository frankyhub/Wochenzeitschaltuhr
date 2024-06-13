// ****************************************************************
// Sketch Esp32 Login Manager Modular(Tab) mit optischer Anzeige
//*******************************************************************/
// Diese Version von Login Manager sollte als Tab eingebunden werden.
// #include <Preferences.h> #include <WebServer.h> müssen im Haupttab aufgerufen werden
// Die Funktionalität des ESP32 Webservers ist erforderlich.
// Die Funktion "Connect();" muss im Setup eingebunden werden.
// Die Oneboard LED leuchtet im AP Modus dauerhaft.
// Die Funktion "reStation()" sollte im "loop" aufgerufen werden.
/**************************************************************************************/

const char HTML1[] PROGMEM = R"(<!DOCTYPE HTML><html lang="de"><head><meta charset="UTF-8"><meta name="viewport" content="width=device-width, initial-scale=1">
<style>button,input{padding:.5em 2em}body{background:#a9a9a9}</style><title>Login Manager</title></head><body>)";
const char HTML2[] PROGMEM = R"(<h2>SSID Passwort</h2><form action="/" method="post"><p>SSID:<br><input name="ssid" placeholder="Name vom Netzwerk" required>
</label></p><p>Passwort:<br><input name="passwort" pattern="[!-~]{8,64}" placeholder="PW vom Netzwerk" required></label></p><p><p>
Einstellen wie du schalten möchtest</p><input type="radio" id="ml" name="aktiv" value="0" checked><label for="ml">LOW Aktiv</label>
<input type="radio" id="mh" name="aktiv" value="1"><label for="mh">HIGH Aktiv</label></p><button>Absenden</button></form></body></html>)";
const char HTML3[] PROGMEM = "<h3> Ihre Eingaben wurden erfolgreich übertragen. Der Server wird neu gestartet. </h3>";
const char HTML4[] PROGMEM = "<center><h2>Erfolgreich angemeldet!</h2><p>lade die index.html hoch</p></center>";

int LED_BUILTIN = 2;

void Connect() {
  char ssid[33];
  char password[65];
  preferences.begin("config", false);
  preferences.getString("ssid", ssid, sizeof ssid);
  preferences.getString("password", password, sizeof password);
  DEBUG((String)"NVS-SSID: " + ssid);
  DEBUG((String)"NVS-Passwort: " + password);
  uint8_t i = 0;
  WiFi.mode(WIFI_STA);                                   //Stationst-Modus --> Esp32 im Heimnetzwerk einloggen
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {                // Led blinkt während des Verbindungsaufbaus
    pinMode(LED_BUILTIN, OUTPUT);                        // OnBoardLed ESP32 NodeMCU-32s
    digitalWrite(LED_BUILTIN, 1);
    delay(500);
    digitalWrite(LED_BUILTIN, 0);
    delay(500);
    i++;
    Serial.printf(" % i sek\n", i);
    if (WiFi.status() != WL_CONNECTED && i > 20) {       // Ist der Router nicht erreichbar, wird ein eigenes Netzwerk erstellt.
      digitalWrite(LED_BUILTIN, 1);                      // Dauerleuchten der Led zeigt den AP Modus an.
      WiFi.disconnect();
      WiFi.mode(WIFI_AP);                                // Soft-Access-Point-Modus --> Access-Point Adresse http://192.168.4.1/
      DEBUG("\nStarte Soft AP");
      if (WiFi.softAP("EspConfig")) {
        Serial.printf("Verbinde dich mit dem Netzwerk \"EspConfig\"\nGib die IP %s im Browser ein\n\n", WiFi.softAPIP().toString().c_str());
      }
      break;
    }
  }
  if (WiFi.status() == WL_CONNECTED) {
    DEBUG("\nVerbunden mit: " + WiFi.SSID() + "\nEsp32 IP: " + WiFi.localIP().toString() + "\n");
    Serial.printf("\nGib diese URL in deinem Browser ein: %s\n\n", WiFi.localIP().toString().c_str());
  }
  server.on("/", HTTP_GET, handleRoot);
  server.on("/", HTTP_POST, handleConfig);
}

void handleConfig() {
  if (server.hasArg("ssid") && server.hasArg("passwort")) {
    preferences.putString("ssid", server.arg(0));
    preferences.putString("password", server.arg(1));
    preferences.putBool("active", server.arg(2).toInt());
    server.send(200, "text/html", (String)HTML1 + HTML3);
    delay(500);
    save();          // vor dem Neustart Betriebsstunden speichern
    ESP.restart();
  }
}

void handleRoot() {                                            // Html Startseite
  if (WiFi.status() != WL_CONNECTED) {
    server.send(200, "text/html", (String)HTML1 + HTML2);      // besteht keine Verbindung zur Station wird HTML2 gesendet
  }
  else {
    if (!handleFile(server.urlDecode(server.uri()))) {         // index.html aus Spiffs senden   // ohne "spiffs.ino" Zeile auskommentieren
      server.send(200, "text/html", (String)HTML1 + HTML4);    // existiert keine index.html wird HTML4 gesendet
    }                                                                                            // ohne "spiffs.ino" Zeile auskommentieren
  }
}

void reStation() {                                             // der Funktionsaufruf "reStation();" sollte im "loop" stehen
  static unsigned long lastMillis = 0;                         // nach Stromausfall startet der Esp.. schneller als der Router
  unsigned long currentMillis = millis();
  if (currentMillis - lastMillis >= 3e5) {                     // im AP Modus aller 5 Minuten prüfen ob der Router verfügbar ist
    if (WiFi.status() != WL_CONNECTED) Connect();
    lastMillis = currentMillis;
  }
}
