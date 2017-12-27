//******************************************************************************
//
// esp-t
//
// Learning project for ESP32 on ARDUINO IDE
//
// Read DS18B20 sensors, save and send the data to a remote target using MQTT
//
//******************************************************************************

//------------------------------------------------------------------------------
// Variables

int LED_BUILTIN = 2;

// End Variables
//------------------------------------------------------------------------------

// Flash the LED at led_pin with a led_delay led_count times.
void LedFlash(int led_pin, int led_delay, int led_count)
{
  for (int i = 0; i < led_count; ++i)
  {
    digitalWrite(led_pin, HIGH);
    delay(led_delay);
    digitalWrite(led_pin, LOW);
    delay(led_delay);
  }
}



// Arduino setup
void setup() {
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);

  // Flash LED threetimes at end of setup
  LedFlash(LED_BUILTIN, 100, 3);
}

// Arduino loop
void loop() {
  // put your main code here, to run repeatedly:

  // Flash LED once:
  LedFlash(LED_BUILTIN, 500, 1);
}

// END
