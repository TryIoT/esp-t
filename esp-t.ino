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

#define SERIAL_SPEED 115200
#define ONE_WIRE_PIN 23
#define TEMPERATURE_PRECISION 12
#define MAX_SENSORS 8
#define LED 2
#define TIMER0_PRESCALER 80

//------------------------------------------------------------------------------
// Variables

Preferences sketch_prefs;

// Wifi related, set to preferences later
char ssid_value[32] = "";
char wpa_psk_value[64] = "";
char ota_password_value[32] = "";

// Timer
hw_timer_t * sensor_timer = NULL;
volatile SemaphoreHandle_t sensor_timer_semaphore;
portMUX_TYPE timer_mux = portMUX_INITIALIZER_UNLOCKED;
const uint32_t sample_interval = 5000000;

// File IO
char data_file[] = "/data.txt";
char just_a_record[] = "1234;24.58;5678";

// Onewire and sensors
OneWire one_wire_bus(ONE_WIRE_PIN);
DallasTemperature sensors(&one_wire_bus);
uint8_t ds18_count = 0; // number of DS18B20 sensors
DeviceAddress ds18_sensors[MAX_SENSORS]; // array to hold device addresses
float temperatures[MAX_SENSORS]; // array to hold temperatures

// End Variables
//------------------------------------------------------------------------------

// Sensor Timer ISR
void IRAM_ATTR onTimer() {
  // Increment the counter and set the time of ISR
  portENTER_CRITICAL_ISR(&timer_mux);
  portEXIT_CRITICAL_ISR(&timer_mux);
  // Give a semaphore that we can check in the loop
  xSemaphoreGiveFromISR(sensor_timer_semaphore, NULL);
}

// Arduino setup
void setup() {
  // initialize digital pin LED as an output.
  Serial.begin(SERIAL_SPEED);
  Serial.println("Starting Setup...");

  pinMode(LED, OUTPUT);

  if (! ReadPreferences()) {
    Serial.println("Reading preferences failed!");
  }

  /* Wifi temp disabled !!!
      if (! SetupWifi(ssid_value, wpa_psk_value)) {
      Serial.println("Wifi connect failed!");
    } else {
      SetupOta(ota_password_value);
    }
  */

  // Timer
  sensor_timer_semaphore = xSemaphoreCreateBinary();
  sensor_timer = timerBegin(0, TIMER0_PRESCALER, true);
  timerAttachInterrupt(sensor_timer, &onTimer, true);
  timerAlarmWrite(sensor_timer, sample_interval, true);
  timerAlarmEnable(sensor_timer);

  // Sensors
  sensors.begin(); // Start up the Dallas library
  // locate devices on the bus
  ds18_count = sensors.getDeviceCount();
  Serial.print("Found ");
  Serial.print(ds18_count);
  Serial.println(" devices.");

  // Search for sensors:
  one_wire_bus.reset_search();
  for (int i = 0; i < ds18_count; ++i) {
    if (!one_wire_bus.search(ds18_sensors[i])) Serial.println("Unable to find address for sensor.");
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
  LedFlash(LED, 100, 3);
  Serial.println("Setup done.");
}


// Arduino loop
void loop() {
  //  ArduinoOTA.handle();

  // Check for timer interrupt
  if (xSemaphoreTake(sensor_timer_semaphore, 0) == pdTRUE) {
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
  LedFlash(LED, 500, 1);
}
// END
