

/*************************************************************************************************
                                      PROGRAMMINFO
**************************************************************************************************
Funktion: ESP32 WEB-Server Zeitschaltuhr 

**************************************************************************************************
Version: 29.05.2022 A163
**************************************************************************************************
Board: ESP32vn IoT UNO
**************************************************************************************************
 T4 = GPIO13
 T5 = GPIO12

**************************************************************************************************
C++ Arduino IDE V1.8.19
**************************************************************************************************
Einstellungen:
https://dl.espressif.com/dl/package_esp32_index.json
http://dan.drown.org/stm32duino/package_STM32duino_index.json
**************************************************************************************************//

#include <WebServer.h>
#include <ArduinoOTA.h>
#include <SPIFFS.h>
#include "time.h"
#include <Preferences.h>

#define DEBUGGING             // Auskommentieren wenn keine Serielle Ausgabe erforderlich ist

#ifdef DEBUGGING
#define DEBUG(...) Serial.println(__VA_ARGS__)
#define DEBUG_F(...) Serial.printf("Funktion: %s meldet in Zeile: %d -> ", __PRETTY_FUNCTION__, __LINE__); Serial.println(__VA_ARGS__)
#else
#define DEBUG(...)
#define DEBUG_F(...)
#endif

struct tm tm;
Preferences preferences;
WebServer server(80);

void setup() {
  Serial.begin(115200);
  preferences.begin("config", false);
  DEBUG((String)"\nSketchname: " + (__FILE__) + "\nBuild: " + (__TIMESTAMP__) + "\n" );
  spiffs();
  admin();
  Connect();
  setupTime();
  setupSchaltUhr();
  setupHobbsMeter();
  ArduinoOTA.onStart([]() {
    save();                  // vor dem Sketch Update Betriebsstunden speichern
  });
  ArduinoOTA.begin();
  server.begin();
  DEBUG("HTTP Server gestartet\n\n");
}

void loop() {
  ArduinoOTA.handle();
  server.handleClient();
  if (millis() < 0x2FFF || millis() > 0xFFFFF0FF) runtime();
  getLocalTime(&tm);        // Aktuallisiert die Uhrzeit
  dualSchaltuhr();
  reStation();
}
