//******************************************************************************
//
// esp-t
//
// Learning project for ESP32 on ARDUINO IDE
//
// Read DS18B20 sensors, save and send the data to a remote target using MQTT
//
//******************************************************************************

#include <FS.h>
#include <SPIFFS.h>
#include <Preferences.h>
#include <WiFi.h>
#include <ArduinoOTA.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#define ONE_WIRE_PIN 23
#define TEMPERATURE_PRECISION 12
#define MAX_SENSORS 8


//------------------------------------------------------------------------------
// Variables

int LED_BUILTIN = 2;

Preferences sketch_prefs;

// Wifi related
char ssid_value[32] = "";
char wpa_psk_value[64] = "";
char ota_password_value[32] = "";

// Timer
hw_timer_t * SensorTimer = NULL;
volatile SemaphoreHandle_t SensorTimerSemaphore;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

// Files
char data_file[] = "/data.txt";
char config_file[] = "/config.txt";
char just_a_record[] = "1234;24.58;5678";

OneWire onew(ONE_WIRE_PIN); // Create OneWire Object
DallasTemperature sensors(&onew);
uint8_t ds18_count = 0; // number of DS18B20 sensors
// arrays to hold 8 device addresses and temperatures
DeviceAddress ds18_sensors[MAX_SENSORS];
float temperatures[MAX_SENSORS];

// End Variables
//------------------------------------------------------------------------------

// Sensor Timer ISR
void IRAM_ATTR onTimer() {
  // Increment the counter and set the time of ISR
  portENTER_CRITICAL_ISR(&timerMux);
  portEXIT_CRITICAL_ISR(&timerMux);
  // Give a semaphore that we can check in the loop
  xSemaphoreGiveFromISR(SensorTimerSemaphore, NULL);
}

// Arduino setup
void setup() {
  // initialize digital pin LED_BUILTIN as an output.
  Serial.begin(115200);
  Serial.println("Starting Setup...");

  pinMode(LED_BUILTIN, OUTPUT);

  if (! ReadPreferences()) {
    Serial.println("Reading preferences failed!");
  }

  /*
      if (! SetupWifi(ssid_value, wpa_psk_value)) {
      Serial.println("Wifi connect failed!");
    } else {
      SetupOta(ota_password_value);
    }
  */

  // Timer
  SensorTimerSemaphore = xSemaphoreCreateBinary();
  SensorTimer = timerBegin(0, 80, true);
  timerAttachInterrupt(SensorTimer, &onTimer, true);
  timerAlarmWrite(SensorTimer, 5000000, true);
  timerAlarmEnable(SensorTimer);

  // Sensors
  sensors.begin(); // Start up the Dallas library
  // locate devices on the bus
  ds18_count = sensors.getDeviceCount();
  Serial.print("Found ");
  Serial.print(ds18_count);
  Serial.println(" devices.");

  // Search for sensors:
  onew.reset_search();
  for (int i = 0; i < ds18_count; ++i) {
    if (!onew.search(ds18_sensors[i])) Serial.println("Unable to find address for sensor.");
    PrintAddress(ds18_sensors[i]);
    Serial.println();
    sensors.setResolution(ds18_sensors[i], TEMPERATURE_PRECISION);
  }

  if (! SPIFFS.begin()) {
    Serial.println("SPIFFS mount failed!");
  }

  if (! WriteDataFile(SPIFFS, data_file, just_a_record)) {
    Serial.println("Write failed!");
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
  //  Serial.println("Check for OTA...");
  //  ArduinoOTA.handle();

  // Check for timer interrupt
  if (xSemaphoreTake(SensorTimerSemaphore, 0) == pdTRUE) {
    Serial.println("Timer Interrupt detected!");
    // request to all devices on the bus
    Serial.print("Requesting temperatures...");
    sensors.requestTemperatures();
    Serial.println("DONE");

    // get temperatures
    Serial.println("Temperatures: ");
    for (int i = 0; i < ds18_count; ++i) {

      temperatures[i] = sensors.getTempC(ds18_sensors[i]);
      Serial.println(temperatures[i]);
    }

  }

  // Flash LED once:
  LedFlash(LED_BUILTIN, 500, 1);
}
// END
