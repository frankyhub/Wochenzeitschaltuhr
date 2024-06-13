// ****************************************************************
// Sketch Esp32 Dual Betriebsstundenzähler Modular(Tab)
// ****************************************************************
// Hardware: Esp32
// Software: Esp32 Arduino Core 1.0.0 - 1.0.2 / 1.0.4
// Getestet auf: ESP32 NodeMCU-32s
//*******************************************************************/
// Diese Version von Betriebsstundenzähler sollte als Tab eingebunden werden.
// #include <Preferences.h> muss im Haupttab aufgerufen werden
// Die Funktionalität des ESP32 Webservers ist erforderlich.
// Die Funktion "setupHobbsMeter();" muss im Setup aufgerufen werden.
/**************************************************************************************/
uint32_t totalmin[2];

void setupHobbsMeter() {
  totalmin[0] = preferences.getULong("HourMeter1");              // Betriebstunden(minuten) lesen
  totalmin[1] = preferences.getULong("HourMeter2");
  server.on("/hourmeter", HTTP_GET, []() {                       // Betriebstunden(minuten) zurücksetzen
    if (server.hasArg("hm1")) {
      preferences.putULong("HourMeter1", (totalmin[0] = 0));
    }
    if (server.hasArg("hm2")) {
      preferences.putULong("HourMeter2", (totalmin[1] = 0));
    }
    server.send(204, "");
  });
}

void hobbsMeter(bool &state_0, bool &state_1) {                  // Aufrufen mit Led Status
  static uint32_t lastMillis[2], lastmin[2] {0, 0};
  auto currentMillis = millis();
  if (currentMillis - lastMillis[0] >= 6e4) {
    lastMillis[0] = currentMillis;
    if (state_0 == aktiv) totalmin[0]++;                         // Betriebstundenzähler Relais 1 wird um eine Minute erhöht
    if (state_1 == aktiv) totalmin[1]++;                         // Betriebstundenzähler Relais 2 wird um eine Minute erhöht
  }
  if (currentMillis - lastMillis[1] >= 1728e5 && (totalmin[0] != lastmin[0] || totalmin[1] != lastmin[1])) {   // bei Änderungen aller 2 Tage Betriebsstunden speichern
    lastMillis[1] = currentMillis;
    lastmin[0] = totalmin[0];
    lastmin[1] = totalmin[1];
    save();
  }
}

String added() {
  return R"(,")" + operatingTime(totalmin[0]) + R"(",")" +  operatingTime(totalmin[1]) + R"(")";
}

String operatingTime(uint32_t &tmin) {                           // Betriebstunden(minuten) formatieren
  char buf[9];
  snprintf(buf, sizeof(buf), "%d,%d", tmin / 60, tmin / 6 % 10);
  return buf;
}

void save() {                                                    // Betriebstunden(minuten) speichern
  preferences.putULong("HourMeter1", totalmin[0]);
  preferences.putULong("HourMeter2", totalmin[1]);
}