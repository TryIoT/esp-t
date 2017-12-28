//******************************************************************************
//
// esp-t led
//
// led related functions
//
//******************************************************************************

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
// END
