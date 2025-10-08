#include <ESP8266WiFi.h>
#include <LittleFS.h>

const char* ssid = "Vodafone-F194";
const char* password = "4PH64amtaTmn6xtp";

WiFiServer server(80);

void setup() {
  Serial.begin(115200);
  Serial.println("\nStarte...");

  // WLAN verbinden
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWLAN verbunden!");
  Serial.print("IP-Adresse: ");
  Serial.println(WiFi.localIP());

  // LittleFS starten
  if (!LittleFS.begin()) {
    Serial.println("❌ Fehler beim Mounten von LittleFS");
    return;
  }
  Serial.println("✅ LittleFS erfolgreich gemountet");

  server.begin();
  Serial.println("Webserver gestartet");
}

void loop() {
  WiFiClient client = server.available();
  if (!client) return;

  while(!client.available()) delay(1);

  String req = client.readStringUntil('\r');
  client.flush();
  Serial.println("➡️ Request: " + req);

  String path = "/";
  if (req.indexOf("GET ") >= 0) {
    int start = req.indexOf("GET ") + 4;
    int end = req.indexOf(" ", start);
    path = req.substring(start, end);
  }

  if (path == "/") path = "/index.html";

  File file = LittleFS.open(path, "r");
  if (!file) {
    Serial.println("❌ Datei nicht gefunden: " + path);
    client.println("HTTP/1.1 404 Not Found\r\n\r\nDatei nicht gefunden");
    return;
  }

  // Content-Type bestimmen
  String contentType = "text/plain";
  if (path.endsWith(".html")) contentType = "text/html";
  else if (path.endsWith(".png")) contentType = "image/png";
  else if (path.endsWith(".jpg")) contentType = "image/jpeg";
  else if (path.endsWith(".css")) contentType = "text/css";

  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: " + contentType);
  client.println("Connection: close");
  client.println();

  // Datei senden
  while(file.available()) {
    client.write(file.read());
  }
  file.close();
  Serial.println("✅ Datei gesendet: " + path);
}