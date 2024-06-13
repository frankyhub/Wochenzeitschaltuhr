// ****************************************************************
// Sketch Esp32 Admin Modular(Tab)
//*******************************************************************/
// Diese Version von Admin sollte als Tab eingebunden werden.
// #include "SPIFFS.h" #include <WebServer.h> müssen im Haupttab aufgerufen werden
// Die Funktionalität des ESP32 Webservers ist erforderlich.
// Die Spiffs.ino muss im ESP32 Webserver enthalten sein
// Funktion "admin();" muss im setup() nach spiffs() aber vor dem Verbindungsaufbau aufgerufen werden.
// Die Funktion "runtime();" muss mindestens zweimal innerhalb 49 Tage aufgerufen werden.
// Entweder durch den Client(Webseite) oder zur Sicherheit im "loop();"
/**************************************************************************************/

#include <rom/rtc.h>

const char* const PROGMEM flashChipMode[] = {"QIO", "QOUT", "DIO", "DOUT", "Unbekannt"};
const char* const PROGMEM resetReason[] = {"ERR", "Power on", "Unknown", "Software", "Watch dog", "Deep Sleep", "SLC module", "Timer Group 0", "Timer Group 1",
                                           "RTC Watch dog", "Instrusion", "Time Group CPU", "Software CPU", "RTC Watch dog CPU", "Extern CPU",
                                           "Voltage not stable", "RTC Watch dog RTC"
                                          };

void admin() {                          // Funktionsaufruf "admin();" muss im Setup eingebunden werden
  WiFi.mode(WIFI_STA);
  File file = SPIFFS.open("/config.json");
  if (file) {
    String Hostname = file.readStringUntil('\n');
    if (Hostname != "") {
      WiFi.setHostname(Hostname.substring(2, Hostname.length() - 2).c_str());
      ArduinoOTA.setHostname(WiFi.getHostname());
    }
  }
  file.close();
  server.on("/admin/renew", handlerenew);
  server.on("/admin/once", handleonce);
  server.on("/reconnect", []() {
    server.send(204, "");
    WiFi.reconnect();
  });
  server.on("/restart", []() {
    server.send(204, "");
    //save();          //Einkommentieren wenn Werte vor dem Neustart gesichert werden sollen
    ESP.restart();
  });
}

void handlerenew() {
  server.send(200, "application/json", R"([")" + runtime() + R"(",")" + temperatureRead() + R"(",")" + WiFi.RSSI() + R"("])");     // Json als Array
}

void handleonce() {
  if (server.arg(0) != "") {
    WiFi.setHostname(server.arg(0).c_str());
    File file = SPIFFS.open("/config.json", FILE_WRITE);
    file.printf("[\"%s\"]", WiFi.getHostname());
    file.close();
  }
  String fname = String(__FILE__).substring( 3, String(__FILE__).lastIndexOf ('\\'));
  String temp = R"({"File":")" + fname.substring(fname.lastIndexOf ('\\') + 1, fname.length()) + R"(", "Build":")" +  (String)__DATE__ + " " + (String)__TIME__ +
                R"(", "LocalIP":")" +  WiFi.localIP().toString() + R"(", "Hostname":")" + WiFi.getHostname() + R"(", "SSID":")" + WiFi.SSID() +
                R"(", "GatewayIP":")" +  WiFi.gatewayIP().toString() +  R"(", "Channel":")" +  WiFi.channel() + R"(", "MacAddress":")" +  WiFi.macAddress() +
                R"(", "SubnetMask":")" +  WiFi.subnetMask().toString() + R"(", "BSSID":")" +  WiFi.BSSIDstr() + R"(", "ClientIP":")" + server.client().remoteIP().toString() +
                R"(", "DnsIP":")" + WiFi.dnsIP().toString() + R"(", "Reset1":")" + resetReason[rtc_get_reset_reason(0)] +
                R"(", "Reset2":")" + resetReason[rtc_get_reset_reason(1)] + R"(", "CpuFreqMHz":")" + ESP.getCpuFreqMHz() +
                R"(", "FreeHeap":")" + formatBytes(ESP.getFreeHeap()) + R"(", "ChipSize":")" +  formatBytes(ESP.getFlashChipSize()) +
                R"(", "ChipSpeed":")" + ESP.getFlashChipSpeed() / 1000000 + R"(", "ChipMode":")" + flashChipMode[ESP.getFlashChipMode()] +
                R"(", "IdeVersion":")" + ARDUINO + R"(", "SdkVersion":")" + ESP.getSdkVersion() + R"("})";
  server.send(200, "application/json", temp);     // Json als Objekt
}

String runtime() {
  static uint8_t rolloverCounter = 0;
  static auto letzteMillis = 0;
  auto aktuelleMillis = millis();
  if (aktuelleMillis < letzteMillis) {       // prüft Millis Überlauf
    rolloverCounter++;
    DEBUG_F("Millis Überlauf");
  }
  letzteMillis = aktuelleMillis;
  auto sek = (0xFFFFFFFF / 1000 ) * rolloverCounter + (aktuelleMillis / 1000);
  char buf[20];
  snprintf(buf, sizeof(buf), sek < 86400 || sek > 172800 ? "%ld Tage %02ld:%02ld:%02ld" : "%ld Tag %02ld:%02ld:%02ld", sek / 86400, sek / 3600 % 24, sek / 60 % 60, sek % 60);
  return buf;
}