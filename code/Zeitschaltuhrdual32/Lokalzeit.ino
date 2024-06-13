// ****************************************************************
// Sketch Esp32 Lokalzeit Modular(Tab)
// created: Jens Fleischer, 2018-07-15
// last mod: Jens Fleischer, 2018-12-29
// ****************************************************************
// Hardware: Esp32
// Software: Esp32 Arduino Core 1.0.0 - 1.0.2 / 1.0.4
// Getestet auf: ESP32 NodeMCU-32s
//******************************************************************

// Diese Version von Lokalzeit sollte als Tab eingebunden werden.
// #include <WebServer.h> oder #include <WiFi.h> muss im Haupttab aufgerufen werden
// Funktion "setupTime();" muss im setup() nach dem Verbindungsaufbau aufgerufen werden.
/**************************************************************************************/

const char* const PROGMEM ntpServer[] = {"fritz.box", "de.pool.ntp.org", "at.pool.ntp.org", "ch.pool.ntp.org", "ptbtime1.ptb.de", "europe.pool.ntp.org"};

bool getTime() {
  configTime(0, 0, ntpServer[1]);                       // deinen NTP Server einstellen (von 0 - 5 aus obiger Liste)
  if (!getLocalTime(&tm)) {
    return false;
  } else {
    setenv("TZ", "CET-1CEST,M3.5.0/02,M10.5.0/03", 1);  // https://github.com/nayarsystems/posix_tz_db/blob/master/zones.csv
    return true;
  }
}

void setupTime() {
  if (!getTime()) {
    DEBUG_F("Zeit konnte nicht geholt werden\n");
  } else {
    getLocalTime(&tm);
    DEBUG_F(&tm, "Programmstart: %A, %B %d %Y %H:%M:%S");
  }
  server.on("/zeit", []() {
    server.send(200, "application/json",  "\"" + (String)localTime() + "\"");
  });
}

char* localTime() {
  static char buf[9];
  static time_t lastsek = 0;
  getLocalTime(&tm);
  static time_t lastday = tm.tm_mday;
  if (tm.tm_sec != lastsek) {
    lastsek = tm.tm_sec;
    strftime (buf, sizeof(buf), "%T", &tm);           // http://www.cplusplus.com/reference/ctime/strftime/
    if (tm.tm_mday != lastday) {
      lastday = tm.tm_mday;
      getTime();
    }
  }
  return buf;
}