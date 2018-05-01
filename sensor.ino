//******************************************************************************
//
// esp-t sensor
//
// DS18B20 related functions and sensor cycle
//
//******************************************************************************

// Convert DS18 Address to a string in hex notation
void Ds18AddressToString(char *ds18_address, DeviceAddress device_address)
{
  char hex_byte[3];
  for (uint8_t i = 0; i < 8; i++)
  {
    sprintf(hex_byte, "%.2X", device_address[i]);
    strcat(ds18_address, hex_byte);
  }
}

// sensor task to be run on core 1
void SensorCycle(void * parameters) {

  // array of structs for storing sensor data
  struct ds18_record {
    DeviceAddress address;
    char hex_address[17];
    float temperature;
    time_t timestamp;
  } ds18_values[MAX_SENSORS];

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
    sensors.setResolution(ds18_values[i].address, TEMPERATURE_PRECISION);
    char ds18_address[17] = "";
    Ds18AddressToString(ds18_address, ds18_values[i].address);
    strcpy(ds18_values[i].hex_address, ds18_address);
    Serial.println(ds18_address);
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
          // retrieve temperatures:
          ds18_values[i].temperature = sensors.getTempC(ds18_values[i].address);
          ds18_values[i].timestamp = time(NULL);
        } // end for

        for (int i = 0; i < ds18_count; ++i) {
          // json and mqtt:
          StaticJsonBuffer<200> jsonBuffer;
          JsonObject& json_root = jsonBuffer.createObject();

          json_root["hexaddress"] = ds18_values[i].hex_address;
          json_root["temperature"] = ds18_values[i].temperature;
          json_root["timestamp"] = ds18_values[i].timestamp;

          json_root.prettyPrintTo(Serial);

          char mqtt_message[100];
          json_root.printTo(mqtt_message);
          mqtt_client.publish(mqtt_topic, mqtt_message);
          mqtt_client.loop();
        } // end for
      } // end if
    } // end while
  } else {
    Serial.println("No devices... exiting task.");
    vTaskDelete( NULL );
  }
}


