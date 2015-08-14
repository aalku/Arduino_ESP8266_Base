void handleRoot() {
  if (!isIp(server.hostHeader())) {
    Serial.print("Request redirected to captive portal");
    server.sendHeader("Location", String("http://") + toStringIp(server.client().localIP()), true);
    server.send ( 302, "text/plain", "");
    return;
  }
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
  if (!isIp(server.hostHeader())) {
    Serial.print("Request redirected to captive portal");
    server.sendHeader("Location", String("http://") + toStringIp(server.client().localIP()), true);
    server.send ( 302, "text/plain", "");
    return;
  }
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

