#ifndef LED_DRIVER_H
#define LED_DRIVER_H

#include <FastLED.h>
#include "config.h"
#include "PatternManager.h"
#include "SegmentManager.h"
#include "EyeRenderer.h"

/**
 * @brief Custom LED Driver class for ESP32 with FastLED
 *
 * This class provides methods to control LED strips and will be extended
 * to support joystick input for interactive LED control.
 */
class LEDDriver
{
private:
  CRGB leds[NUM_LEDS];          // Main LED array (Eye + Clock)
  CRGB poleLeds[POLE_NUM_LEDS]; // Pole LED array
  uint8_t brightness;           // Current brightness level
  unsigned long lastUpdate;     // Last update timestamp
  bool needsUpdate;             // Flag to indicate if display needs refresh

  // Current mode
  uint8_t currentMode;

  // Color state for mode 2 (color wheel)
  uint8_t currentR, currentG, currentB;

  // Blink state for mode 3
  bool blinkState;
  unsigned long lastBlinkTime;

  // Pattern manager for advanced patterns
  PatternManager *patternManager;

  // Global settings state (for clock/eye)
  uint8_t globalBrightness;
  float globalSpeed;
  int selectedPatternIndex;
  int selectedPaletteIndex;

  // Pole-specific settings (separate from clock)
  uint8_t poleBrightness;
  float poleSpeed;
  int selectedPolePatternIndex;
  int selectedPolePaletteIndex;
  int selectedJoltPaletteIndex;

  // Mode system state
  uint8_t currentMainMode; // Current main mode (Explorer/Interaction)
  uint8_t currentSubMode;  // Current sub-mode within main mode

  // Settings mode state
  enum SettingsPhase
  {
    PHASE_1_QUADRANTS,
    PHASE_2_BRIGHTNESS,
    PHASE_2_SPEED,
    PHASE_2_PATTERN,
    PHASE_2_PALETTE
  };
  SettingsPhase settingsPhase;
  int currentQuadrant;         // Which quadrant is being hovered/selected
  int previewedItem;           // Which item is being previewed in Phase 2
  bool itemPreviewed;          // Whether an item is currently being previewed
  unsigned long holdStartTime; // When user started holding in a quadrant
  bool isHolding;              // Whether user is currently holding
  bool flashState;             // For flashing effect during selection
  unsigned long lastFlashTime; // Last flash update time

  // Sticky pointer state (for settings mode)
  int stickyPointerPosition; // Last pointer position when joystick was active
  bool hasStickyPointer;     // Whether sticky pointer should be shown

  // Pointer mode state for Mode 2 (chase pointer)
  int pointerPosition;
  unsigned long lastPointerMove;

  // Calibration state
  bool inCalibrationMode;
  unsigned long calibrationStartTime;
  unsigned long lastCalibrationBlink;
  bool calibrationBlinkState;
  int xMin, xMax, yMin, yMax; // Calibrated joystick bounds

  // Button hold detection
  bool buttonHeldDown;
  unsigned long buttonPressStartTime;
  bool holdActionTriggered;

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

  // Segment manager for multi-ring operations
  SegmentManager segmentManager;

  // Eye renderer for pointer mode
  EyeRenderer *eyeRenderer;

  // Action Pattern system for fireworks
  ActionPattern *activeFireworks[MAX_ACTIVE_FIREWORKS];
  int activeFireworkCount;

  // Firework mode state
  bool inFireworkMode;
  unsigned long fireworkModeStartTime;
  bool lastJoystickUpState;

  // Pole pattern control

  // Mode processing methods
  void processCurrentMode();
  void processExplorerMode();
  void processInteractionMode();

  // Explorer sub-mode methods
  void processClockPatternExplorer();
  void processClockSettings();
  void processPolePatternExplorer();
  void processPoleSettings();

  // Interaction sub-mode methods
  void processEyeballMode();
  void processFireworkMode();
  void processJoltMode();

  // Special mode methods
  void processSettingsMode();
  void processCalibrationMode();

  // Mode switching methods
  void cycleSingleClick(); // Cycle sub-mode
  void cycleHoldAction();  // Cycle main mode (triggered by button hold)
  String getCurrentModeDescription() const;
  String getCurrentSubModeDescription() const;

  // Firework mode helper methods (accessed via Interaction Mode)
  void exitFireworkMode();
  void launchFirework(unsigned long currentTime);
  void updateActiveFireworks(unsigned long currentTime);
  void cleanupInactiveFireworks();

  // Jolt mode helper methods
  uint8_t calculateJoltMagnitude(int joystickY);
  void renderJoltEffect(uint8_t magnitude);
  void renderJoltPole(uint8_t magnitude);
  void renderJoltEyeClock(uint8_t magnitude);

  // Pole pattern control methods
  void setPolePatternIndex(int patternIndex);
  void updatePolePatternSelection();

  // Legacy mode methods (for reuse in new system)
  void processMainModeOld();
  void processEyeModeOld();
  void processBrightnessSpeedModeOld();
  void processPatternModeOld();

  // Settings mode helper methods
  void processSettingsPhase1();
  void processSettingsPhase2();
  void renderQuadrantPreviews();
  void renderBrightnessPhase2();
  void renderSpeedPhase2();
  void renderPatternPhase2();
  void renderPalettePhase2();
  void renderSelectionTicks(int numItems);
  void renderQuadrantPointer();
  void renderPhase2Pointer();
  int getCurrentSettingPosition(); // Get current setting position for default display

  // Brightness and Speed mode helpers
  void renderBrightnessPreview(uint8_t level);
  void renderSpeedPreview(uint8_t level);
  void renderCombinedPreview(uint8_t brightnessLevel, uint8_t speedLevel);
  void renderWaveEffectOnClock();
  int determineQuadrant(int x, int y);
  int determineClockPosition(int x, int y);
  void startHolding(int quadrant);
  void stopHolding();
  void selectCurrentItem();
  void applySelectedSetting();

  // Pointer helper methods
  void createPointer(int centerLED, int width);

  // Calibration helper methods
  void startCalibrationMode();
  void exitCalibrationMode();
  bool detectButtonHold(bool buttonPressed, unsigned long currentTime);
  void saveCalibration();
  void loadCalibration();

  // Power management helper methods
  float calculateCurrentDraw();
  float calculatePowerConsumption();
  uint8_t calculateSafeBrightness();
  bool isPowerLimitExceeded();
  void applyPowerLimiting();

public:
  /**
   * @brief Constructor for LEDDriver
   */
  LEDDriver();

  /**
   * @brief Destructor for LEDDriver
   */
  ~LEDDriver();

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
   * @brief Set all LEDs to a solid color with brightness
   * @param r Red component (0-255)
   * @param g Green component (0-255)
   * @param b Blue component (0-255)
   * @param brightness Brightness level (0-255)
   */
  void setSolidColor(uint8_t r, uint8_t g, uint8_t b, uint8_t brightness);

  /**
   * @brief Set all LEDs to a solid color with brightness using CRGB
   * @param color CRGB color object
   * @param brightness Brightness level (0-255)
   */
  void setSolidColor(CRGB color, uint8_t brightness);

  /**
   * @brief Set all LEDs to a HTML Color name
   * @param colorName HTML color name
   */
  void setSolidColor(CRGB::HTMLColorCode colorName);

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
   * @brief Set individual LED color with brightness
   * @param index LED index (0 to NUM_LEDS-1)
   * @param r Red component (0-255)
   * @param g Green component (0-255)
   * @param b Blue component (0-255)
   * @param brightness Brightness level (0-255)
   */
  void setLED(int index, uint8_t r, uint8_t g, uint8_t b, uint8_t brightness);

  /**
   * @brief Set individual LED color with brightness using CRGB
   * @param index LED index (0 to NUM_LEDS-1)
   * @param color CRGB color object
   * @param brightness Brightness level (0-255)
   */
  void setLED(int index, CRGB color, uint8_t brightness);

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
   * @brief Get number of pole LEDs
   * @return Total number of pole LEDs
   */
  int getPoleNumLEDs() const { return POLE_NUM_LEDS; }

  // Pole LED methods
  /**
   * @brief Set color of specific pole LED
   * @param index LED index (0 to POLE_NUM_LEDS-1)
   * @param color RGB color
   */
  void setPolePixel(int index, CRGB color);

  /**
   * @brief Fill all pole LEDs with solid color
   * @param color RGB color to fill
   */
  void fillPole(CRGB color);

  /**
   * @brief Clear all pole LEDs (set to black)
   */
  void clearPole();

  /**
   * @brief Update pole patterns
   */
  void updatePole();

  /**
   * @brief Create firework pattern with pole access
   * @return Pointer to firework pattern with pole LED access
   */
  Pattern *createFireworkPattern();

  // Pole geometry helper methods
  /**
   * @brief Get spiral column for LED index
   * @param index LED index (0 to POLE_NUM_LEDS-1)
   * @return Column position (0 to POLE_SPIRAL_REPEAT-1)
   */
  uint8_t getPoleColumn(uint16_t index);

  /**
   * @brief Get spiral height level for LED index
   * @param index LED index (0 to POLE_NUM_LEDS-1)
   * @return Height level (0 to POLE_HEIGHT_LEVELS-1)
   */
  uint8_t getPoleHeight(uint16_t index);

  /**
   * @brief Get current mode
   * @return Current mode (0=Main, 1=Settings, 2=Pointer)
   */
  uint8_t getCurrentMode() const { return currentMode; }

  /**
   * @brief Set mode manually (for testing)
   * @param mode Mode to set (0=Main, 1=Settings, 2=Pointer)
   */
  void setMode(uint8_t mode);

  /**
   * @brief Get global brightness setting
   * @return Global brightness (1-255)
   */
  uint8_t getGlobalBrightness() const { return globalBrightness; }

  /**
   * @brief Get global speed setting
   * @return Global speed (0.5-10.0)
   */
  float getGlobalSpeed() const { return globalSpeed; }

  /**
   * @brief Get selected pattern index
   * @return Pattern index
   */
  int getSelectedPatternIndex() const { return selectedPatternIndex; }

  /**
   * @brief Get selected palette index
   * @return Palette index
   */
  int getSelectedPaletteIndex() const { return selectedPaletteIndex; }

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

  /**
   * @brief Get current power consumption estimate
   * @return Current power consumption in watts
   */
  float getCurrentPowerConsumption();

  /**
   * @brief Get current estimated current draw
   * @return Current draw in milliamps
   */
  float getCurrentDraw();

  /**
   * @brief Check if power limiting is active
   * @return True if power limiting is reducing brightness
   */
  bool isPowerLimited();

  /**
   * @brief Get pattern manager reference
   * @return Reference to PatternManager
   */
  PatternManager &getPatternManager();

  /**
   * @brief Get segment manager reference
   * @return Reference to SegmentManager
   */
  SegmentManager &getSegmentManager();

  /**
   * @brief Handle pattern-related serial commands
   * @param command Command string
   * @return true if command was handled
   */
  bool handlePatternCommand(const String &command);
};

#endif // LED_DRIVER_H
