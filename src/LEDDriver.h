#ifndef LED_DRIVER_H
#define LED_DRIVER_H

#include <FastLED.h>
#include "config.h"

/**
 * @brief Custom LED Driver class for ESP32 with FastLED
 *
 * This class provides methods to control LED strips and will be extended
 * to support joystick input for interactive LED control.
 */
class LEDDriver
{
private:
  CRGB leds[NUM_LEDS];      // LED array
  uint8_t brightness;       // Current brightness level
  unsigned long lastUpdate; // Last update timestamp
  bool needsUpdate;         // Flag to indicate if display needs refresh

  // Joystick state (for future implementation)
  struct JoystickState
  {
    int x;
    int y;
    bool buttonPressed;
    unsigned long lastRead;
  } joystickState;

public:
  /**
   * @brief Constructor for LEDDriver
   */
  LEDDriver();

  /**
   * @brief Initialize the LED driver and FastLED library
   * @return true if initialization successful, false otherwise
   */
  bool initialize();

  /**
   * @brief Update the LED display (call in main loop)
   */
  void update();

  /**
   * @brief Set all LEDs to a solid color
   * @param r Red component (0-255)
   * @param g Green component (0-255)
   * @param b Blue component (0-255)
   */
  void setSolidColor(uint8_t r, uint8_t g, uint8_t b);

  /**
   * @brief Set all LEDs to a solid color using CRGB
   * @param color CRGB color object
   */
  void setSolidColor(CRGB color);

  /**
   * @brief Set individual LED color
   * @param index LED index (0 to NUM_LEDS-1)
   * @param r Red component (0-255)
   * @param g Green component (0-255)
   * @param b Blue component (0-255)
   */
  void setLED(int index, uint8_t r, uint8_t g, uint8_t b);

  /**
   * @brief Set individual LED color using CRGB
   * @param index LED index (0 to NUM_LEDS-1)
   * @param color CRGB color object
   */
  void setLED(int index, CRGB color);

  /**
   * @brief Clear all LEDs (turn them off)
   */
  void clear();

  /**
   * @brief Set brightness level
   * @param brightness Brightness level (0-255)
   */
  void setBrightness(uint8_t brightness);

  /**
   * @brief Get current brightness level
   * @return Current brightness (0-255)
   */
  uint8_t getBrightness() const;

  /**
   * @brief Force immediate LED update
   */
  void show();

  // Future joystick methods (to be implemented)
  /**
   * @brief Read joystick input (placeholder for future implementation)
   */
  void readJoystick();

  /**
   * @brief Process joystick input to control LEDs (placeholder)
   */
  void processJoystickInput();

  /**
   * @brief Get number of LEDs
   * @return Total number of LEDs
   */
  int getNumLEDs() const { return NUM_LEDS; }
};

#endif // LED_DRIVER_H
