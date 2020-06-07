
#include <ESP8266WiFi.h>
#include <Arduino.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <EEPROM.h>
#include <DNSServer.h>
#include <WiFiClient.h>
#include <ESP8266HTTPClient.h>


const IPAddress ap_ip(192,168,4,1);
const IPAddress ap_subnet(255,255,255,0);
const char* ap_ssid     = "Board_010";
const char* ap_password = "123456789";
const char* board_id = "Board_010";
String host = "18.222.127.160";
boolean isSettingsMode;
DNSServer dnsServer;

AsyncWebServer server(80);

// Replaces placeholder for future development
String processor(const String& var){
  return "1";
}

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    html {
     font-family: Arial;
     display: inline-block;
     margin: 0px auto;
     text-align: center;
    }
    h2 { font-size: 3.0rem; }
    p { font-size: 3.0rem; }
    .units { font-size: 1.2rem; }
    .dht-labels{
      font-size: 1.0rem;
      vertical-align:middle;
      padding-bottom: 15px;
    }
  </style>
</head>
<body>
  <h2>Pillbox Wifi Settings</h2>
  <form method=post action="/connect">
  <p>
    <span class="dht-labels">Wi-Fi Username:</span> 
    <input type=text name=ssid>
  </p>
  <p>
    <span class="dht-labels">Wi-Fi Password:</span>
    <input type=text name=password>
  </p>
  <p>
  <input type=submit>
  </form>
</body>
</html>)rawliteral";


void writeToStorage(String ssid, String pass) 
{
  for (int i = 0; i < 96; ++i) {
    EEPROM.write(i, 0);
  }
  
  Serial.println("Writing SSID:");
  Serial.println(ssid);
  Serial.println("Writing Password:");
  Serial.println(pass);

  for (int i = 0; i < ssid.length(); ++i) {
    EEPROM.write(i, ssid[i]);
  }

  for (int i = 0; i < pass.length(); ++i) {
    EEPROM.write(32 + i, pass[i]);
  }
  EEPROM.commit();
  ESP.restart();
}

boolean readFromStorageAndConnect() {
  Serial.println("Reading from storage and attempting connection.");
  String ssid_name = "";
  String ssid_pass = "";

  if(EEPROM.read(0) != 0) {
    for (int i = 0; i < 32; ++i) {
      ssid_name += char(EEPROM.read(i));
    }

    for(int i = 32; i < 96; ++i) {
      ssid_pass += char(EEPROM.read(i));
    }

    WiFi.begin(ssid_name.c_str(), ssid_pass.c_str());
    Serial.println("Attempting Connection:");
    Serial.print("ssid name: ");
    Serial.println(ssid_name);
    Serial.print("ssid password: ");
    Serial.println(ssid_pass);
    int counter = 0;
    while (counter < 25) {
      if (WiFi.status() == WL_CONNECTED) {
        Serial.println("Connected to Wi-Fi");
        digitalWrite(0, LOW);
        return true;
      }
      delay(500);
      Serial.print(".");
      counter++;
    }
    Serial.println("Connection Failed.");
    return false;
  }
  else {
    Serial.println("Wi-Fi details were not found. entering setup mode.");
    return false;   
  }
  
}


void setup() {
  Serial.begin(115200);
  EEPROM.begin(512);
  pinMode(0, OUTPUT);
  pinMode(14, INPUT_PULLUP);
  digitalWrite(0, HIGH);
  delay(100);
  if(readFromStorageAndConnect()) {
    isSettingsMode = false;
    return;
  }
  isSettingsMode = true;
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(ap_ip, ap_ip, ap_subnet);
  WiFi.softAP(ap_ssid, ap_password);
  dnsServer.start(53, "*", ap_ip);

  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html, processor);
  });
      server.on("/connect", HTTP_POST, [](AsyncWebServerRequest *request){
    Serial.println("form submitted.");
    if(request->getParam("ssid", true)->value() != "" && request->getParam("password", true)->value() != "") {
      request->send(200, "text/plain", "Logging in...");

      writeToStorage(String(request->getParam("ssid", true)->value()), String(request->getParam("password", true)->value()));
       
      return;
    }
    Serial.println("form illegal. going to login page.");
    request->send_P(400, "text/html", index_html, processor);
    
  });
  
  // Start server
  server.begin();
}

void loop() {
  if(isSettingsMode) {
    dnsServer.processNextRequest();
  }
  else {
    int value = digitalRead(14);
    Serial.print("Pin 14 status:");
    Serial.println(value);
    if (value == 0) {
      Serial.println("POSTING to client");
      WiFiClient client;
      const int httpPort = 80;
      if (!client.connect(host, httpPort)) {
        Serial.println("HTTP connection to server failed.");
        return;
      }
      Serial.println("client connection success");
      String postData = "board_id=" + String(board_id);
      String url = "/api/autopublish";
      String address = host + ":" + String(httpPort) + url;
      Serial.print("sending POST request to address: ");
      Serial.println(address);
      HTTPClient http;
      http.begin("http://18.222.127.160:80/api/autopublish");
      http.addHeader("Content-Type", "application/x-www-form-urlencoded");
      auto httpCode = http.POST(postData);
      Serial.println(httpCode);
      String payload = http.getString();
      Serial.println(payload);
      http.end();
      delay(10000);
    }
    delay(500);
 
  }
}
