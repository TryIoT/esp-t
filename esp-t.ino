//******************************************************************************
//
// esp-t
//
// Learning project for ESP32 on ARDUINO IDE
//
// Read DS18B20 sensors, save and send the data to a remote target using MQTT
//
//******************************************************************************

#include <time.h>
#include <string.h>
#include <FS.h>
#include <SPIFFS.h>
#include <Preferences.h>
#include <WiFi.h>
#include <ArduinoOTA.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>

#define SERIAL_SPEED 115200
#define ONE_WIRE_PIN 22
#define TEMPERATURE_PRECISION 12
#define MAX_SENSORS 8
#define LED 5
#define TIMER0_PRESCALER 80
#define ARDUINOJSON_ENABLE_PROGMEM 0


//------------------------------------------------------------------------------
// Variables

Preferences sketch_prefs;

// Wifi related, set to preferences later
char ssid_value[32] = "";
char wpa_psk_value[64] = "";
char ota_password_value[32] = "";

WiFiClient esp_client;
PubSubClient mqtt_client(esp_client);
const char * mqtt_server = "10.1.1.13";
const int mqtt_port = 1883;
const char * mqtt_topic = "iserbach4/test";

// Timer
hw_timer_t * sensor_timer = NULL;
volatile SemaphoreHandle_t sensor_timer_semaphore;
portMUX_TYPE timer_mux = portMUX_INITIALIZER_UNLOCKED;
const uint32_t sample_interval = 5000000;

// File IO
char data_file[] = "/data.txt";
char just_a_record[] = "1234;24.58;5678";


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
  Serial.begin(SERIAL_SPEED);
  Serial.println("Starting Setup...");

  pinMode(LED, OUTPUT);

  if (! ReadPreferences()) {
    Serial.println("Reading preferences failed!");
  }

  if (! SetupWifi(ssid_value, wpa_psk_value)) {
    Serial.println("Wifi connect failed!");
  } else {
    // Start OTA:
    SetupOta(ota_password_value);

    // Connect to MQTT:
    mqtt_client.setServer(mqtt_server, mqtt_port);
    while (!mqtt_client.connected()) {
      Serial.println("Connecting to MQTT...");
      if (mqtt_client.connect("ESP32Client")) {
        Serial.println("connected");
      } else {
        Serial.print("failed with state ");
        Serial.print(mqtt_client.state());
        delay(2000);
      } // end if
    } // end while
  } // end else

  /* Commented out, not needed for now, causes serial trouble
    if (! SPIFFS.begin()) {
      Serial.println("SPIFFS mount failed!");
    }

    if (! WriteDataFile(SPIFFS, data_file, just_a_record)) {
      Serial.println("Write failed!");
    }

    listDir(SPIFFS, "/", 0);
    readFile(SPIFFS, "/hello.txt");
    readFile(SPIFFS, data_file);
  */

  // launch sensor task on core 1:
  xTaskCreatePinnedToCore(
    SensorCycle,                  // Function to implement the task
    "SensorCycle",                // Name of the task
    4000,                         // Stack size in words
    NULL,                         // Task input parameter
    5,                            // Priority of the task
    NULL,                         // Task handle.
    1);                           // Core where the task should run

  // Start the timer for the sensor cycle
  sensor_timer_semaphore = xSemaphoreCreateBinary();
  sensor_timer = timerBegin(0, TIMER0_PRESCALER, true);
  timerAttachInterrupt(sensor_timer, &onTimer, true);
  timerAlarmWrite(sensor_timer, sample_interval, true);
  timerAlarmEnable(sensor_timer);

  // Flash LED threetimes at end of setup
  LedFlash(LED, 100, 3);
  Serial.println("Setup done.");
}

// Arduino loop
void loop() {
  ArduinoOTA.handle();

  // Flash LED once:
  LedFlash(LED, 500, 1);
}
// END
