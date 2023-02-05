#include <Arduino.h>
#include "secrets.h"
#include <WiFiClientSecure.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <WebServer.h>
#include <LittleFS.h>
#include <ESPAsyncWebServer.h>

WebServer webServer(80);

// Moisture Sensor Pin
#define Moisture_PIN 33

// Motor Driver
#define MOTOR_PIN1 39 // GPIO for control input 1
#define MOTOR_PIN2 37 // GPIO for control input 2

// Button Pin
#define Button_Pin 7

int moisture = 0;
int moistureThreshold = 0;

// ***********************************************************
void waterGarden()
{
  digitalWrite(MOTOR_PIN1, HIGH);
  digitalWrite(MOTOR_PIN2, LOW);
  delay(3000);
  digitalWrite(MOTOR_PIN1, LOW);
  digitalWrite(MOTOR_PIN2, LOW);
}

// *********************************************************************
// this function examines the URL from the client and based on the extension
// determines the type of response to send.
bool loadFromLittleFS(String path)
{
  bool bStatus;
  String contentType;

  bStatus = false; // set bStatus to false assuming this will not work.

  contentType = "text/plain"; // assume this will be the content type returned, unless path extension indicates something else

  // DEBUG:  print request URI to user:
  Serial.print("Requested URI: ");
  Serial.println(path.c_str());

  // if no path extension is given, assume index.html is requested.
  if (path.endsWith("/"))
    path += "index.html";

  // look at the URI extension to determine what kind of data to send to client.
  if (path.endsWith(".src"))
    path = path.substring(0, path.lastIndexOf("."));
  else if (path.endsWith(".html"))
    contentType = "text/html";
  else if (path.endsWith(".htm"))
    contentType = "text/html";
  else if (path.endsWith(".css"))
    contentType = "text/css";
  else if (path.endsWith(".js"))
    contentType = "application/javascript";
  else if (path.endsWith(".png"))
    contentType = "image/png";
  else if (path.endsWith(".gif"))
    contentType = "image/gif";
  else if (path.endsWith(".jpg"))
    contentType = "image/jpeg";
  else if (path.endsWith(".ico"))
    contentType = "image/x-icon";
  else if (path.endsWith(".xml"))
    contentType = "text/xml";
  else if (path.endsWith(".pdf"))
    contentType = "application/pdf";
  else if (path.endsWith(".zip"))
    contentType = "application/zip";

  // try to open file in LittleFS filesystem
  File dataFile = LittleFS.open(path.c_str(), "r");
  // if dataFile <> 0, then it was opened successfully.
  if (dataFile)
  {
    if (webServer.hasArg("download"))
      contentType = "application/octet-stream";
    // stream the file to the client.  check that it was completely sent.
    if (webServer.streamFile(dataFile, contentType) != dataFile.size())
    {
      Serial.println("Error streaming file: " + String(path.c_str()));
    }
    // close the file
    dataFile.close();
    // indicate success
    bStatus = true;
  }
  return bStatus;
}

// *********************************************************************
void handleWebRequests()
{
  if (!loadFromLittleFS(webServer.uri()))
  {
    // file not found.  Send 404 response code to client.
    String message = "File Not Found\n\n";
    message += "URI: ";
    message += webServer.uri();
    message += "\nMethod: ";
    message += (webServer.method() == HTTP_GET) ? "GET" : "POST";
    message += "\nArguments: ";
    message += webServer.args();
    message += "\n";
    for (uint8_t i = 0; i < webServer.args(); i++)
    {
      message += " NAME:" + webServer.argName(i) + "\n VALUE:" + webServer.arg(i) + "\n";
    }
    webServer.send(404, "text/plain", message);
    Serial.println(message);
  }
}

// ***********************************************************
void checkInput()
{
  if (moistureThreshold > moisture)
  {
    waterGarden();
  }

  float slope = 2.48;      // slope from linear fit
  float intercept = -0.72; // intercept from linear fit
  float voltage = (float(analogRead(Moisture_PIN)) / 1023.0) * 3.3;
  moisture = ((1.0 / voltage) * slope) + intercept;

  String json;
  json.reserve(88);
  json = "{\"time\":";
  json += millis();
  json += ", \"moisture\":";
  json += moisture;
  json += "}";
  Serial.println(json);
  webServer.send(200, "text/json", json); // Sends the json string to the web server

  delay(500); // Sampling frequency
}

// ***********************************************************
void wifiSetup()
{
  // Initialize File System
  LittleFS.begin();
  Serial.println("File System Initialized");

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  // handler for time and temperature
  webServer.on("/checkInput", HTTP_GET, checkInput);

  // send all other web requests here by default.
  // The filesystem will be searched for the requested resource
  webServer.onNotFound(handleWebRequests);

  // start web server and print it's address.
  webServer.begin();
  Serial.printf("\nWeb server started, open %s in a web browser\n", WiFi.localIP().toString().c_str());
}

// ***********************************************************
void otaSetup()
{
  ArduinoOTA
      .onStart([]()
               {
      String type;
      if (ArduinoOTA.getCommand() == U_FLASH)
        type = "sketch";
      else // U_SPIFFS
        type = "filesystem";

      // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
      Serial.println("Start updating " + type); })
      .onEnd([]()
             { Serial.println("\nEnd"); })
      .onProgress([](unsigned int progress, unsigned int total)
                  { Serial.printf("Progress: %u%%\r", (progress / (total / 100))); })
      .onError([](ota_error_t error)
               {
      Serial.printf("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
      else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
      else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
      else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
      else if (error == OTA_END_ERROR) Serial.println("End Failed"); });

  ArduinoOTA.begin();

  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

// ***********************************************************
void setup()
{
  Serial.begin(115200);

  pinMode(MOTOR_PIN1, OUTPUT);
  pinMode(MOTOR_PIN2, OUTPUT);

  wifiSetup();
  otaSetup();
}

// ***********************************************************
void loop()
{
  ArduinoOTA.handle();
  webServer.handleClient();
}