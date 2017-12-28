//******************************************************************************
//
// esp-t
//
// Learning project for ESP32 on ARDUINO IDE
//
// Read DS18B20 sensors, save and send the data to a remote target using MQTT
//
//******************************************************************************

#include "FS.h"
#include "SPIFFS.h"

//------------------------------------------------------------------------------
// Variables

int LED_BUILTIN = 2;
char data_file[] = "/data.txt";
char config_file[] = "/config.txt";
char just_a_record[] = "1234;24.58;5678";


// End Variables
//------------------------------------------------------------------------------


// Arduino setup
void setup() {
  // initialize digital pin LED_BUILTIN as an output.
  Serial.begin(115200);
  Serial.println("Starting Setup...");

  pinMode(LED_BUILTIN, OUTPUT);

  if (!SPIFFS.begin()) {
    Serial.println("SPIFFS Mount Failed");
    return;
  }

  if( ! WriteDataFile(SPIFFS, data_file, just_a_record)) {
    Serial.println("Write failed.");
  }

  listDir(SPIFFS, "/", 0);
  readFile(SPIFFS, "/hello.txt");
  readFile(SPIFFS, data_file);


  // Flash LED threetimes at end of setup
  LedFlash(LED_BUILTIN, 100, 3);
  Serial.println("Setup done.");

}


// Arduino loop
void loop() {
  // put your main code here, to run repeatedly:

  // Flash LED once:
  LedFlash(LED_BUILTIN, 500, 1);
}
// END
