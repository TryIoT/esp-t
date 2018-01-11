//******************************************************************************
//
// esp-t preferences
//
// Read sketch preferences from NVRAM
//
//******************************************************************************

// Read sketch preferences in global variables
boolean ReadPreferences() {
  if ( ! sketch_prefs.begin("sketch", false)) {
    return false;
  } else {
    sketch_prefs.getString("ssid", ssid_value, 32);
    sketch_prefs.getString("wpa_psk", wpa_psk_value, 64);
    sketch_prefs.getString("ota_password", ota_password_value,32);
    sketch_prefs.end();
    return(true);
  }
}
