// ****************************************************************
// Sketch Esp32 Datei Manager Modular(Tab)
// ****************************************************************
// Hardware: Esp32
// Software: Esp32 Arduino Core 1.0.0 - 1.0.2 / 1.0.4
// Geprüft: mit 4MB Flash
// Getestet auf: ESP32 NodeMCU-32s
//******************************************************************
// Diese Version von Spiffs sollte als Tab eingebunden werden.
// #include <SPIFFS.h> #include <WebServer.h> müssen im Haupttab aufgerufen werden
// Die Funktionalität des ESP32 Webservers ist erforderlich.
// "server.onNotFound()" darf nicht im Setup des ESP8266 Webserver stehen.
// Die Funktion "spiffs();" muss im Setup aufgerufen werden.
/**************************************************************************************/
const char* Header = "HTTP/1.1 303 OK\r\nLocation:spiffs.html\r\nCache-Control: no-cache\r\n\r\n";
const char Helper[] PROGMEM = R"(<form action="/upload" method="post" enctype="multipart/form-data">
<input type="file" name="upload"><input type="submit" value="Upload"></form>Lade die spiffs.html hoch.)";

void spiffs() {       // Funktionsaufruf "spiffs();" muss im Setup vor "Connect();" eingebunden werden
  SPIFFS.begin(true);
  server.on("/json", handleList);
  server.on("/format", formatSpiffs);
  server.on("/upload", HTTP_POST, []() {}, handleFileUpload);
  server.onNotFound([]() {
    if (!handleFile(server.urlDecode(server.uri())))
      server.send(404, "text/plain", "FileNotFound");
  });
}

void handleList() {               // Senden aller Daten an den Client
  File root = SPIFFS.open("/");
  String temp = R"({"dir":[)";
  File f = root.openNextFile();
  while (f) {                            // Auflistung aller im Spiffs vorhandenen Dateien
    if (temp != R"({"dir":[)") temp += ",";
    temp += R"({"name":")" + String(f.name() + 1) + R"(","size":")" + formatBytes(f.size()) + R"("})";
    f = root.openNextFile();
  }
  temp += R"(,{"usedBytes":")" + formatBytes(SPIFFS.usedBytes() * 1.05) + R"(",)" +              // Berechnet den verwendeten Speicherplatz + 5% Sicherheitsaufschlag
          R"("totalBytes":")" + formatBytes(SPIFFS.totalBytes()) + R"(","freeBytes":")" +        // Zeigt die Größe des Speichers
          (SPIFFS.totalBytes() - (SPIFFS.usedBytes() * 1.05)) + R"("}]})";                       // Berechnet den freien Speicherplatz + 5% Sicherheitsaufschlag
  server.send(200, "application/json", temp);
}

bool handleFile(String&& path) {
  if (server.hasArg("delete")) {
    SPIFFS.remove(server.arg("delete"));        // Datei löschen
    server.sendContent(Header);
    return true;
  }
  if (!SPIFFS.exists("/spiffs.html"))server.send(200, "text/html", Helper); //Upload the spiffs.html
  if (path.endsWith("/")) path += "index.html";
  return SPIFFS.exists(path) ? ({File f = SPIFFS.open(path, "r"); server.streamFile(f, contentType(path)); f.close(); true;}) : false;
}

void handleFileUpload() {                                  // Dateien vom Rechnenknecht oder Klingelkasten ins SPIFFS schreiben
  static File fsUploadFile;
  HTTPUpload& upload = server.upload();
  if (upload.status == UPLOAD_FILE_START) {
    if (upload.filename.length() > 30) {                          // Dateinamen auf 30 Zeichen kürzen
      int x = upload.filename.length() - 30;
      upload.filename = upload.filename.substring(x, 30 + x);
    }
    DEBUG("FileUpload Name: " + upload.filename);
    fsUploadFile = SPIFFS.open("/" + server.urlDecode(upload.filename), "w");
  } else if (upload.status == UPLOAD_FILE_WRITE) {
    DEBUG("FileUpload Data: " + (String)upload.currentSize);
    if (fsUploadFile)
      fsUploadFile.write(upload.buf, upload.currentSize);
  } else if (upload.status == UPLOAD_FILE_END) {
    if (fsUploadFile)
      fsUploadFile.close();
    DEBUG("FileUpload Size: " + (String)upload.totalSize);
    server.sendContent(Header);
  }
}

void formatSpiffs() {       //Formatiert den Speicher
  SPIFFS.format();
  server.sendContent(Header);
}

const String formatBytes(size_t const& bytes) {            // lesbare Anzeige der Speichergrößen
  return (bytes < 1024) ? String(bytes) + " Byte" : (bytes < (1024 * 1024)) ? String(bytes / 1024.0) + " KB" : String(bytes / 1024.0 / 1024.0) + " MB";
}

const String &contentType(String& filename) {                   // ermittelt den Content-Typ
  if (filename.endsWith(".htm") || filename.endsWith(".html")) filename = "text/html";
  else if (filename.endsWith(".css")) filename = "text/css";
  else if (filename.endsWith(".js")) filename = "application/javascript";
  else if (filename.endsWith(".json")) filename = "application/json";
  else if (filename.endsWith(".png")) filename = "image/png";
  else if (filename.endsWith(".gif")) filename = "image/gif";
  else if (filename.endsWith(".jpg")) filename = "image/jpeg";
  else if (filename.endsWith(".ico")) filename = "image/x-icon";
  else if (filename.endsWith(".xml")) filename = "text/xml";
  else if (filename.endsWith(".pdf")) filename = "application/x-pdf";
  else if (filename.endsWith(".zip")) filename = "application/x-zip";
  else if (filename.endsWith(".gz")) filename = "application/x-gzip";
  else filename = "text/plain";
  return filename;
}

bool freeSpace(uint16_t const& printsize) {
  DEBUG_F(formatBytes(SPIFFS.totalBytes() - (SPIFFS.usedBytes() * 1.05)) + " im Spiffs frei");
  return (SPIFFS.totalBytes() - (SPIFFS.usedBytes() * 1.05) > printsize) ? true : false;
}