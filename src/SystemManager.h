#ifndef SYSTEM_MANAGER_H
#define SYSTEM_MANAGER_H

#include <Arduino.h>
#include "LEDDriver.h"
#include "WiFiManager.h"

/**
 * @brief System Manager
 * 
 * Coordinates between different modules and handles system-level operations.
 * Manages serial commands, system information, and module interactions.
 */
class SystemManager
{
private:
    // Module references
    LEDDriver* ledDriver;
    WiFiManager* wifiManager;
    
    // Demo mode variables
    unsigned long lastColorChange;
    int colorIndex;
    static const CRGB demoColors[];
    static const int NUM_DEMO_COLORS;
    
    // Serial command handlers
    void handleLEDCommands(const String& command);
    void handleSystemCommands(const String& command);
    
    // Helper functions
    void printHelp();
    void printSystemInfo();
    void cycleDemoColors();

public:
    /**
     * @brief Constructor
     * @param ledDriverRef Reference to LED driver instance
     * @param wifiManagerRef Reference to WiFi manager instance
     */
    SystemManager(LEDDriver* ledDriverRef, WiFiManager* wifiManagerRef);
    
    /**
     * @brief Initialize system manager
     */
    void initialize();
    
    /**
     * @brief Update system manager (call in main loop)
     */
    void update();
    
    /**
     * @brief Handle serial commands
     */
    void handleSerialCommands();
    
    /**
     * @brief Get system uptime in milliseconds
     * @return Uptime in milliseconds
     */
    unsigned long getUptime() const { return millis(); }
    
    /**
     * @brief Get free heap memory
     * @return Free heap in bytes
     */
    uint32_t getFreeHeap() const { return ESP.getFreeHeap(); }
    
    /**
     * @brief Get CPU frequency
     * @return CPU frequency in MHz
     */
    uint8_t getCPUFrequency() const { return ESP.getCpuFreqMHz(); }
};

#endif // SYSTEM_MANAGER_H
