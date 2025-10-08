#ifndef PATTERN_MANAGER_H
#define PATTERN_MANAGER_H

#include <Arduino.h>
#include <FastLED.h>
#include "config.h"
#include "Pattern.h"
#include "ColorPalette.h"
#include "SegmentManager.h"

/**
 * @brief Pattern manager class for handling multiple LED patterns and palettes
 *
 * This class manages a collection of patterns and color palettes,
 * providing functionality to switch between them, adjust parameters,
 * and coordinate their execution.
 */
class PatternManager
{
private:
  static const int MAX_PATTERNS = 25; // Increased to accommodate WLED patterns

  CRGB *leds;  // Pointer to LED array
  int numLeds; // Number of LEDs

  Pattern *patterns[MAX_PATTERNS]; // Array of pattern pointers
  int patternCount;                // Number of loaded patterns
  int currentPatternIndex;         // Current active pattern index

  PaletteManager paletteManager;  // Color palette manager
  SegmentManager *segmentManager; // Pointer to segment manager

  bool autoSwitch;                  // Auto-switch patterns
  unsigned long autoSwitchInterval; // Auto-switch interval (ms)
  unsigned long lastAutoSwitch;     // Last auto-switch time

  uint8_t globalBrightness; // Global brightness override
  float globalSpeed;        // Global speed multiplier

  // Pattern transition
  bool inTransition;                // Currently transitioning
  unsigned long transitionStart;    // Transition start time
  unsigned long transitionDuration; // Transition duration (ms)
  int fromPatternIndex;             // Transition from pattern
  int toPatternIndex;               // Transition to pattern

public:
  /**
   * @brief Constructor
   * @param leds Pointer to LED array
   * @param numLeds Number of LEDs
   * @param segManager Pointer to SegmentManager (optional)
   */
  PatternManager(CRGB *leds, int numLeds, SegmentManager *segManager = nullptr);

  /**
   * @brief Destructor
   */
  ~PatternManager();

  /**
   * @brief Initialize pattern manager with default patterns
   */
  void initialize();

  /**
   * @brief Update current pattern
   * @param currentTime Current time in milliseconds
   * @return true if LEDs were updated
   */
  bool update(unsigned long currentTime);

  /**
   * @brief Add a pattern to the manager
   * @param pattern Pointer to Pattern object
   * @return true if added successfully
   */
  bool addPattern(Pattern *pattern);

  /**
   * @brief Get pattern by index
   * @param index Pattern index
   * @return Pointer to Pattern or nullptr if invalid
   */
  Pattern *getPattern(int index);

  /**
   * @brief Get pattern by name
   * @param name Pattern name
   * @return Pointer to Pattern or nullptr if not found
   */
  Pattern *getPattern(const String &name);

  /**
   * @brief Get current pattern
   * @return Pointer to current Pattern
   */
  Pattern *getCurrentPattern();

  /**
   * @brief Set current pattern by index
   * @param index Pattern index
   * @param useTransition Use smooth transition
   * @return true if set successfully
   */
  bool setCurrentPattern(int index, bool useTransition = false);

  /**
   * @brief Set current pattern by name
   * @param name Pattern name
   * @param useTransition Use smooth transition
   * @return true if set successfully
   */
  bool setCurrentPattern(const String &name, bool useTransition = false);

  /**
   * @brief Get current pattern index
   * @return Current pattern index
   */
  int getCurrentPatternIndex() const { return currentPatternIndex; }

  /**
   * @brief Get number of patterns
   * @return Number of patterns
   */
  int getPatternCount() const { return patternCount; }

  /**
   * @brief Get pattern name by index
   * @param index Pattern index
   * @return Pattern name or empty string if invalid
   */
  String getPatternName(int index);

  /**
   * @brief Cycle to next pattern
   * @param useTransition Use smooth transition
   */
  void nextPattern(bool useTransition = false);

  /**
   * @brief Cycle to previous pattern
   * @param useTransition Use smooth transition
   */
  void previousPattern(bool useTransition = false);

  /**
   * @brief Get palette manager reference
   * @return Reference to PaletteManager
   */
  PaletteManager &getPaletteManager() { return paletteManager; }

  /**
   * @brief Set current palette for active pattern
   * @param paletteIndex Palette index
   * @return true if set successfully
   */
  bool setCurrentPalette(int paletteIndex);

  /**
   * @brief Set current palette by name
   * @param paletteName Palette name
   * @return true if set successfully
   */
  bool setCurrentPalette(const String &paletteName);

  /**
   * @brief Cycle to next palette
   */
  void nextPalette();

  /**
   * @brief Cycle to previous palette
   */
  void previousPalette();

  /**
   * @brief Set global brightness for all patterns
   * @param brightness Brightness value (0-255)
   */
  void setGlobalBrightness(uint8_t brightness);

  /**
   * @brief Get global brightness
   * @return Global brightness (0-255)
   */
  uint8_t getGlobalBrightness() const { return globalBrightness; }

  /**
   * @brief Set global speed multiplier for all patterns
   * @param speed Speed multiplier (0.1 to 10.0)
   */
  void setGlobalSpeed(float speed);

  /**
   * @brief Get global speed multiplier
   * @return Global speed multiplier
   */
  float getGlobalSpeed() const { return globalSpeed; }

  /**
   * @brief Enable/disable auto-switching between patterns
   * @param enable Enable auto-switching
   * @param intervalMs Interval between switches in milliseconds
   */
  void setAutoSwitch(bool enable, unsigned long intervalMs = 30000);

  /**
   * @brief Check if auto-switching is enabled
   * @return true if auto-switching is enabled
   */
  bool getAutoSwitch() const { return autoSwitch; }

  /**
   * @brief Start pattern transition
   * @param toIndex Target pattern index
   * @param durationMs Transition duration in milliseconds
   */
  void startTransition(int toIndex, unsigned long durationMs = 1000);

  /**
   * @brief Check if currently in transition
   * @return true if transitioning
   */
  bool isInTransition() const { return inTransition; }

  /**
   * @brief Print all pattern names to Serial
   */
  void printPatterns();

  /**
   * @brief Print current status to Serial
   */
  void printStatus();

  /**
   * @brief Handle serial command for pattern/palette control
   * @param command Command string
   * @return true if command was handled
   */
  bool handleSerialCommand(const String &command);

private:
  /**
   * @brief Update pattern transition
   * @param currentTime Current time in milliseconds
   */
  void updateTransition(unsigned long currentTime);

  /**
   * @brief Apply global settings to current pattern
   */
  void applyGlobalSettings();
};

#endif // PATTERN_MANAGER_H
