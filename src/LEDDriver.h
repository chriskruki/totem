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

  // Current mode
  uint8_t currentMode;

  // Color state for mode 2 (color wheel)
  uint8_t currentR, currentG, currentB;

  // Blink state for mode 3
  bool blinkState;
  unsigned long lastBlinkTime;

  // Calibration state
  bool inCalibrationMode;
  unsigned long calibrationStartTime;
  unsigned long lastCalibrationBlink;
  bool calibrationBlinkState;
  int xMin, xMax, yMin, yMax; // Calibrated joystick bounds

  // Triple-click detection
  int clickCount;
  unsigned long lastClickTime;
  unsigned long firstClickTime;

  // Joystick state
  struct JoystickState
  {
    int x;
    int y;
    bool buttonPressed;
    bool lastButtonState;
    unsigned long lastButtonChange;
    unsigned long lastRead;
  } joystickState;

  // Private mode processing methods
  void processBrightnessMode();
  void processColorWheelMode();
  void processBlinkMode();
  void processPointerMode();
  void processCalibrationMode();

  // Calibration helper methods
  void startCalibrationMode();
  void exitCalibrationMode();
  bool detectDoubleClick(bool buttonPressed, unsigned long currentTime);
  void saveCalibration();
  void loadCalibration();

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

  /**
   * @brief Get current mode
   * @return Current mode (0=Config, 1=Color, 2=Blink)
   */
  uint8_t getCurrentMode() const { return currentMode; }

  /**
   * @brief Set mode manually (for testing)
   * @param mode Mode to set (0=Config, 1=Color, 2=Blink)
   */
  void setMode(uint8_t mode);

  /**
   * @brief Get current RGB color values
   * @param r Reference to red value
   * @param g Reference to green value
   * @param b Reference to blue value
   */
  void getCurrentColor(uint8_t &r, uint8_t &g, uint8_t &b) const;

  /**
   * @brief Check if in calibration mode
   * @return True if currently calibrating
   */
  bool isInCalibrationMode() const { return inCalibrationMode; }

  /**
   * @brief Get calibration bounds
   * @param xMin Reference to X minimum value
   * @param xMax Reference to X maximum value
   * @param yMin Reference to Y minimum value
   * @param yMax Reference to Y maximum value
   */
  void getCalibrationBounds(int &xMin, int &xMax, int &yMin, int &yMax) const;
};

#endif // LED_DRIVER_H
