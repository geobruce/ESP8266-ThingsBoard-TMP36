#include "ThingsBoard.h"

#include <ESP8266WiFi.h>


#define WIFI_AP             "Your-WIFI-SSID"
#define WIFI_PASSWORD       "Your-WIFI-PASS"

// See https://thingsboard.io/docs/getting-started-guides/helloworld/
// to understand how to obtain an access token
#define TOKEN               "Your-Device-Token"
#define THINGSBOARD_SERVER  "demo.thingsboard.io"

// Baud rate for debug serial
#define SERIAL_DEBUG_BAUD   115200

// Initialize ThingsBoard client
WiFiClient espClient;
// Initialize ThingsBoard instance
ThingsBoard tb(espClient);
// the Wifi radio's status
int status = WL_IDLE_STATUS;

void setup() {
  // initialize serial for debugging
  Serial.begin(SERIAL_DEBUG_BAUD);
  WiFi.begin(WIFI_AP, WIFI_PASSWORD);
  InitWiFi();
}


#define TEMP_BUFFER_SIZE 20
float temps[TEMP_BUFFER_SIZE];
int full = 0;
int tempIndex = 0;

void updateTemp() {
  temps[tempIndex] = getTemp();
  tempIndex++;
  if (tempIndex >= TEMP_BUFFER_SIZE) {
    full = 1;
    tempIndex = 0;
  }
}
float getAvgTemp() {
  int maxIndex = 0;
  float totTemp = 0;
  if (full > 0) {
    maxIndex = TEMP_BUFFER_SIZE;
  } else {
    maxIndex = tempIndex;
  }
  for (int i = 0; i < maxIndex; i++) {
    totTemp += temps[i];
  }
  Serial.print("Avgtemp = ");
  Serial.println(totTemp / maxIndex);
  return totTemp / maxIndex;
}

float getTemp() { 
  int sensorValue = analogRead(A0);
  float voltage = (float)sensorValue / 1024.00;
  float temp = 25.0  + (voltage - 0.750) * 100.0;
  Serial.print("temp = ");
  Serial.println(temp);
  return temp;
}



double lastTime = millis();
void loop() {
  // delay(1000);
  while ((millis() - lastTime) < 1000) {
    updateTemp();
    delay(50);
  }
  lastTime = millis();

  if (WiFi.status() != WL_CONNECTED) {
    reconnect();
  }


  if (!tb.connected()) {
    // Connect to the ThingsBoard
    Serial.print("Connecting to: ");
    Serial.print(THINGSBOARD_SERVER);
    Serial.print(" with token ");
    Serial.println(TOKEN);
    if (!tb.connect(THINGSBOARD_SERVER, TOKEN)) {
      Serial.println("Failed to connect");
      return;
    }
  }

  Serial.println("Sending data...");

  // Uploads new telemetry to ThingsBoard using MQTT.
  // See https://thingsboard.io/docs/reference/mqtt-api/#telemetry-upload-api
  // for more details

  tb.sendTelemetryFloat("temperature", getAvgTemp());
  //  tb.sendTelemetryFloat("humidity", 42.5);

  tb.loop();
}

void InitWiFi()
{
  Serial.println("Connecting to AP ...");
  // attempt to connect to WiFi network

  WiFi.begin(WIFI_AP, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected to AP");
}

void reconnect() {
  // Loop until we're reconnected
  status = WiFi.status();
  if ( status != WL_CONNECTED) {
    WiFi.begin(WIFI_AP, WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
    Serial.println("Connected to AP");
  }
}
