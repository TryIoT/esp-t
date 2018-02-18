//******************************************************************************
//
// esp-t sensor
//
// DS18B20 related functions and sensor cycle
//
//******************************************************************************


// Print a DS18B20 device address
void PrintAddress(DeviceAddress deviceAddress)
{
  for (uint8_t i = 0; i < 8; i++)
  {
    // zero pad the address if necessary
    if (deviceAddress[i] < 16) Serial.print("0");
    Serial.print(deviceAddress[i], HEX);
  }
}


// sensor task to be run on core 1
void SensorCycle(void * parameters) {

  // array of structs for storing sensor data
  struct ds18_record {
    DeviceAddress address;
    float temperature;
    time_t timestamp;
  } ds18_values[MAX_SENSORS];

  // json
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& json_root = jsonBuffer.createObject();

  // Setup onewire bus and Dallas stuff:
  Serial.print("Setup OneWire and Dallas...");
  OneWire one_wire_bus(ONE_WIRE_PIN);
  DallasTemperature sensors(&one_wire_bus);
  Serial.println("Done.");

  // search for number of sensors ...
  Serial.print("Starting up Dallas and searching for sensors...");
  sensors.begin(); // Start up the Dallas library
  uint8_t ds18_count = sensors.getDeviceCount(); // number of DS18B20 sensors
  Serial.println("Done.");
  Serial.print("Found ");
  Serial.print(ds18_count);
  Serial.println(" devices.");

  // ...and get their addresses and set the resolution
  one_wire_bus.reset_search();
  for (int i = 0; i < ds18_count; ++i) {
    if (!one_wire_bus.search(ds18_values[i].address)) Serial.println("Unable to find address for sensor.");
    PrintAddress(ds18_values[i].address);
    Serial.println();
    sensors.setResolution(ds18_values[i].address, TEMPERATURE_PRECISION);
  }

  delay(3000);

  if (ds18_count > 0) {
    Serial.println("Entering endless sensor loop...");
    // If there are sensors, start the endless loop:
    while (true) {
      // Check for timer interrupt
      if (xSemaphoreTake(sensor_timer_semaphore, 0) == pdTRUE) {
        // request to all devices on the bus
        Serial.print("Requesting temperatures...");
        sensors.requestTemperatures();
        Serial.println("DONE");

        Serial.println("Temperatures: ");
        for (int i = 0; i < ds18_count; ++i) {
          ds18_values[i].temperature = sensors.getTempC(ds18_values[i].address);
          ds18_values[i].timestamp = time(NULL);
          // JSON:
          json_root["id"] = abc.id;
          json_root["temperature"] = ds18_values[i].temperature;
          json_root["timestamp"] = ds18_values[i].timestamp;

          Serial.print(ds18_values[i].timestamp);
          Serial.print(" ");
          Serial.println(ds18_values[i].temperature);
        }
      } // if semaphore check
    } // while loop
  } else {
    Serial.println("No devices... exiting task.");
    vTaskDelete( NULL );
  }
}


