#include <Arduino.h>
#include "LEDDriver.h"
#include "WiFiManager.h"
#include "SystemManager.h"
#include "config.h"

// Global module instances
LEDDriver ledDriver;
WiFiManager wifiManager(&ledDriver);
SystemManager systemManager(&ledDriver, &wifiManager);

/**
 * @brief Setup function - runs once at startup
 */
void setup()
{
  // Initialize the system manager, which coordinates all modules
  systemManager.initialize();
}

/**
 * @brief Main loop - runs continuously
 */
void loop()
{
  // Update the system manager, which updates all modules
  systemManager.update();

  // Small delay to prevent overwhelming the CPU
  delay(1);
}
