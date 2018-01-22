//******************************************************************************
//
// esp-t sensor
//
// DS18B20 related functions
//
//******************************************************************************

// Search for sensors:
void SearchDS18B20(void * parameters) {
  sensors.begin(); // Start up the Dallas library
  uint8_t ds18_count = sensors.getDeviceCount(); // number of DS18B20 sensors
  Serial.print("Found ");
  Serial.print(ds18_count);
  Serial.println(" devices.");

  one_wire_bus.reset_search();
  for (int i = 0; i < ds18_count; ++i) {
    if (!one_wire_bus.search(ds18_sensors[i])) Serial.println("Unable to find address for sensor.");
    PrintAddress(ds18_sensors[i]);
    Serial.println();
    sensors.setResolution(ds18_sensors[i], TEMPERATURE_PRECISION);
  }
  vTaskDelete( NULL );
}

// Read Temperatures:
void GetTemperatures(void * parameters) {
  Serial.println("Temperatures: ");
  for (int i = 0; i < 2; ++i) {
    temperatures[i] = sensors.getTempC(ds18_sensors[i]);
//    Serial.print(time_stamp);
    Serial.print(" ");
    Serial.println(temperatures[i]);
  }
  vTaskDelete( NULL );
}


// function to print a device address
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
