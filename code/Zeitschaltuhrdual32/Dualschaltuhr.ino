// ****************************************************************
// Sketch Esp32 Dual Zeitschaltuhr Modular(Tab)
// ****************************************************************
// Hardware: Esp32, Relais Modul o. Mosfet IRF3708 o. Fotek SSR-40 DA
// für Relais Modul
// GND an GND
// IN1 an T4 = GPIO13
// IN2 an T5 = GPIO12
// VCC an VIN -> je nach verwendeten Esp.. möglich
// Jumper JD-VCC VCC
// alternativ ext. 5V Netzteil verwenden
//
// für Mosfet IRF3708
// Source an GND
// Mosfet1 Gate an T4 = GPIO13
// Mosfet2 Gate an T5 = GPIO12
//
// für 3V Solid State Relais
// GND an GND
// SSR1 Input + an T4 = GPIO13
// SSR2 Input + an T5 = GPIO12
//
// Software: Esp32 Arduino Core 1.0.0 - 1.0.2 / 1.0.4
// Getestet auf: ESP32 NodeMCU-32s
//*******************************************************************/
// Diese Version von Dualschaltuhr sollte als Tab eingebunden werden.
// #include <WebServer.h> & #include <Preferences.h> müssen im Haupttab aufgerufen werden
// Die Funktionalität des ESP32 Webservers ist erforderlich.
// Der Lokalzeit Tab ist zum ausführen der Zeitschaltuhr einzubinden.
// Die Funktion "setupSchaltUhr();" muss im Setup aufgerufen werden.
// Zum schalten muss die Funktion "dualSchaltuhr();" im loop(); aufgerufen werden.
/**************************************************************************************/

bool aktiv, relState[2];
const uint8_t relPin[] = {T4, T5};         // Pin für Relais eventuell einstellen
const auto item = 10;
char switchTime[item * 2][6];
uint8_t switchActive[item] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1};

void setupSchaltUhr() {
  aktiv = preferences.getBool("active");
  DEBUG((String)"NVS-aktiv: " + aktiv);
  for (auto pin : relPin) digitalWrite(pin, !aktiv), pinMode(pin, OUTPUT);
  relState[0] = !aktiv;
  relState[1] = !aktiv;
  DEBUG((String)"Relais Aktiv Status: " + (relState[0] ? "LOW" : "HIGH"));
  char buffer[3];
  for (auto i = 0; i < item * 2; i++) {
    itoa (i, buffer, 10);
    preferences.getString(buffer, switchTime[i], sizeof(switchTime[i]));
  }
  preferences.getBytes("switchActive", switchActive, sizeof switchActive);
  server.on("/timer", HTTP_POST, []() {
    if (server.args() == 1) {
      switchActive[server.argName(0).toInt() / 2] = server.arg(0).toInt();
    }
    if (server.hasArg("sz0")) {
      for (auto i = 0; i < server.args(); i++) {
        strcpy (switchTime[i], server.arg(i).c_str());
      }
    }
    char buffer[3];
    for (auto i = 0; i < item * 2; i++) {
      itoa (i, buffer, 10);
      preferences.putString(buffer, switchTime[i]);
    }
    preferences.putBytes("switchActive", switchActive, sizeof switchActive);
    String temp = "[";
    for (auto i = 0; i < item * 2; i++) {
      if (temp != "[") temp += ',';
      temp += (String)R"(")" + switchTime[i] + R"(")";
    }
    for (auto i = 0; i < item; i++) {
      temp += (String)R"(,")" + switchActive[i] + R"(")";
    }
    server.send(200, "application/json", temp += added() + "]");
  });
  server.on("/timer", HTTP_GET, []() {
    if (server.hasArg("tog1")) relState[0] = !relState[0];    // Relais1 manuell schalten
    if (server.hasArg("tog2")) relState[1] = !relState[1];    // Relais2 manuell schalten
    server.send(200, "application/json", (String)R"([")" + localTime() + R"(",")" + (relState[0] ? aktiv : !aktiv) + R"(",")" + (relState[1] ? aktiv : !aktiv) + R"("])");
  });
  server.on("/config", []() {
    if (server.hasArg("delall")) {
      preferences.clear();                                    // alle Daten löschen
      preferences.end();
      server.send(204, "");
      ESP.restart();
    } else {
      server.send(200, "text/html", (String)HTML1 + HTML2);   // Passwort, SSID ändern
    }
  });
}

void dualSchaltuhr() {
  static uint8_t oldmin = 60, oldState[] = {aktiv, aktiv};
  hobbsMeter(relState[0], relState[1]);                       // Betriebsstundenzähler Aufrufen mit Led Status
  if (tm.tm_min != oldmin) {
    oldmin = tm.tm_min;
    char buf[6];
    snprintf(buf, sizeof(buf), "%.2d:%.2d", tm.tm_hour, tm.tm_min);
    Serial.println(buf);
    for (auto i = 0; i < item * 2; i++) {
      if (i < item) {
        if (switchActive[i / 2] && !strcmp(switchTime[i], buf)) i % 2 ? relState[0] = !aktiv : relState[0] = aktiv; // Relais1 nach Zeit schalten
      }
      else {
        if (switchActive[i / 2] && !strcmp(switchTime[i], buf)) i % 2 ? relState[1] = !aktiv : relState[1] = aktiv; // Relais2 nach Zeit schalten
      }
    }
  }
  if (relState[0] != oldState[0] || relState[1] != oldState[1]) {
    for (auto i = 0; i < 2; i++) {
      oldState[i] = relState[i];
      digitalWrite(relPin[i], relState[i]);
      DEBUG(digitalRead(relPin[i]) == aktiv ? (String)"Relais" + (1 + i) + " an" : (String)"Relais" + (1 + i) + " aus");
    }
  }
}