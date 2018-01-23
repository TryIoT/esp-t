//******************************************************************************
//
// esp-t sensor
//
// DS18B20 related functions
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
  Serial.print(" - ");
  if (deviceAddress[7] < 16) Serial.print("0");
  Serial.print(deviceAddress[7], HEX);
}


// sensor task to be run on core 1
void SensorCycle(void * parameters) {
  // Setup onewire bus and Dallas stuff:
  OneWire one_wire_bus(ONE_WIRE_PIN);
  DallasTemperature sensors(&one_wire_bus);
  DeviceAddress ds18_sensors[MAX_SENSORS]; // array to hold device addresses
  float temperatures[MAX_SENSORS]; // array to hold temperatures

  // search for number of sensors ...
  sensors.begin(); // Start up the Dallas library
  uint8_t ds18_count = sensors.getDeviceCount(); // number of DS18B20 sensors
  Serial.print("Found ");
  Serial.print(ds18_count);
  Serial.println(" devices.");

  // ...and get their addresses and set the resolution
  one_wire_bus.reset_search();
  for (int i = 0; i < ds18_count; ++i) {
    if (!one_wire_bus.search(ds18_sensors[i])) Serial.println("Unable to find address for sensor.");
    PrintAddress(ds18_sensors[i]);
    Serial.println();
    sensors.setResolution(ds18_sensors[i], TEMPERATURE_PRECISION);
  }

  if (ds18_count > 0) {
    // If there are sensors, start the endless loop:
    while (true) {
      // Check for timer interrupt
      if (xSemaphoreTake(sensor_timer_semaphore, 0) == pdTRUE) {
        // request to all devices on the bus
        Serial.print("Requesting temperatures...");
        sensors.requestTemperatures();
        time_t time_stamp = time(NULL);
        Serial.println("DONE");

        // print temperatures
        Serial.println("Temperatures: ");
        for (int i = 0; i < ds18_count; ++i) {
          temperatures[i] = sensors.getTempC(ds18_sensors[i]);
          Serial.print(time_stamp);
          Serial.print(" ");
          Serial.println(temperatures[i]);
        }
      } // if semaphore check
    } // while loop
  } else {
    Serial.println("No devices... exiting task.");
    vTaskDelete( NULL );
  }
}


