#include <ESP8266WiFi.h>
#include <WiFiClient.h> 
#include <ESP8266WebServer.h>
#include <EEPROM.h>
#include <ESP8266mDNS.h>

/* Set these to your desired credentials. */
const char *softAP_ssid = "ESPap";
const char *softAP_password = "thereisnospoon";

char ssid[32] = "";
char password[32] = "";

ESP8266WebServer server(80);

boolean connect = true;

void handleRoot() {
  server.send(200, "text/html", "");
  for (int i = 0; i < 10; i++) {
    server.sendContent("<h1>Hello world!!</h1>");
    delay(500);
  }
}

void handleWifi() {
  server.send(200, "text/html", "");
  server.sendContent("<h1>Wifi config</h1>");
  server.sendContent("<label>SoftAP config</label>");
  server.sendContent("<ul>");
  server.sendContent(String() + "<li>SSID " + String(softAP_ssid) + "</li>");
  server.sendContent(String() + "<li>IP " + toStringIp(WiFi.softAPIP()) + "</li>");
  server.sendContent("</ul>");
  server.sendContent("<label>WLAN config</label>");
  server.sendContent("<ul>");
  server.sendContent(String() + "<li>SSID " + String(ssid) + "</li>");
  server.sendContent(String() + "<li>IP " + toStringIp(WiFi.localIP()) + "</li>");
  server.sendContent("</ul>");
  server.sendContent("<label>WLAN list (refresh if any missing)</label>");
  server.sendContent("<ul>");
  Serial.println("scan start");
  int n = WiFi.scanNetworks();
  Serial.println("scan done");
  if (n > 0) {
    for (int i = 0; i < n; i++) {
      server.sendContent(String() + "<li>SSID " + String(WiFi.SSID(i)) + String((WiFi.encryptionType(i) == ENC_TYPE_NONE)?" ":" *") + " (" + WiFi.RSSI(i) + ")</li>");
    }
  } else {
    server.sendContent(String() + "<li>No WLAN found</li>");
  }
  server.sendContent("</ul>");
  server.sendContent("<form method='POST' action='wifisave'><label>Connect to network:</label>");
  server.sendContent("<input type='text' placeholder='network' name='n'/>");
  server.sendContent("<input type='password' placeholder='password' name='p'/>");
  server.sendContent("<input type='submit' value='Connect/Disconnect'/></form>");
}

String toStringIp(IPAddress ip) {
  String res = "";
  for (int i = 0; i < 3; i++) {
    res += String((ip >> (8 * i)) & 0xFF) + ".";
  }
  res += String(((ip >> 8 * 3)) & 0xFF);
  return res;
}

void handleWifiSave() {
  Serial.println("wifi save");
  server.arg("n").toCharArray(ssid, sizeof(ssid) - 1);
  server.arg("p").toCharArray(password, sizeof(password) - 1);
  server.sendHeader("Location", "wifi", true);
  server.send ( 302, "text/plain", "");
  EEPROM.begin(512);
  EEPROM.put(0, ssid);
  EEPROM.put(0+sizeof(ssid), password);
  EEPROM.commit();
  EEPROM.end();
  connect=true;
}

void handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += ( server.method() == HTTP_GET ) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";

  for ( uint8_t i = 0; i < server.args(); i++ ) {
    message += " " + server.argName ( i ) + ": " + server.arg ( i ) + "\n";
  }

  server.send ( 404, "text/plain", message );
}

void setup() {
  delay(1000);
  Serial.begin(115200);
  Serial.println();
  Serial.printf("Sketch size: %u\n", ESP.getSketchSize());
  Serial.printf("Free size: %u\n", ESP.getFreeSketchSpace());
  Serial.println();
  Serial.print("Configuring access point...");
  /* You can remove the password parameter if you want the AP to be open. */
  WiFi.softAP(softAP_ssid, softAP_password);
  delay(500);
  Serial.print("AP IP address: ");
  Serial.println(WiFi.softAPIP());
  server.on("/", handleRoot);
  server.on("/wifi", handleWifi);
  server.on("/wifisave", handleWifiSave);
  server.onNotFound ( handleNotFound );
  server.begin();
  Serial.println("HTTP server started");
  connect = true;
  EEPROM.begin(512);
  EEPROM.get(0, ssid);
  EEPROM.get(0+sizeof(ssid), password);
  EEPROM.end();
  Serial.println("Recovered credentials:");
  Serial.println(ssid);
  Serial.println(strlen(password)>0?"********":"<no password>");
}

int status = WL_IDLE_STATUS;

void connectWifi() {
  Serial.println("Connecting as wifi client...");
  WiFi.disconnect();
  WiFi.begin ( ssid, password );
  int connRes = WiFi.waitForConnectResult();
  Serial.print ( "connRes: " );
  Serial.println ( connRes );
}

void loop() {
  if (connect) {
      Serial.println ( "Connect requested" );
      connect = false;
      connectWifi();
  }
  {
    int s = WiFi.status();
    if (status != s) {
      Serial.print ( "Status: " );
      Serial.println ( s );
      status = s;
      if (s == WL_CONNECTED) {
        Serial.println ( "" );
        Serial.print ( "Connected to " );
        Serial.println ( ssid );
        Serial.print ( "IP address: " );
        Serial.println ( WiFi.localIP() );

        if (!MDNS.begin("esp8266")) {
          Serial.println("Error setting up MDNS responder!");
        } else {
          Serial.println("mDNS responder started");
          // Add service to MDNS-SD
          MDNS.addService("http", "tcp", 80);
        }
      } else if (s == WL_NO_SSID_AVAIL) {
        WiFi.disconnect();
      }
    }
  }

  //HTTP
  server.handleClient();
}
