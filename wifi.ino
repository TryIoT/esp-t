//******************************************************************************
//
// esp-t wifi
//
// Connect to WiFi and setup OTA
//
//******************************************************************************

// Setup WiFi
boolean SetupWifi(const char * ssid, const char * wpa_psk)
{
  const int wifi_timeout = 20; // Try wifi_timeout times to connect

  Serial.print("Connecting to ");
  Serial.println(ssid);
  if (! WiFi.begin(ssid, wpa_psk)) {
    return (false);
  } else {
    for (int i = 0; i < wifi_timeout; ++i) {
      if (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
      } else
      {
        Serial.println("");
        Serial.print("WiFi connected, IP address: ");
        Serial.println(WiFi.localIP());
        return (true);
      }
    }
    return (false);
  }
}

// Setup OTA
void SetupOta(const char * ota_password)
{
  ArduinoOTA.setPassword(ota_password);
  ArduinoOTA
  .onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH)
      type = "sketch";
    else // U_SPIFFS
      type = "filesystem";

    // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
    Serial.println("Start updating " + type);
  })
  .onEnd([]() {
    Serial.println("\nEnd");
  })
  .onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  })
  .onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();
}

