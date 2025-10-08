#ifndef POLE_PATTERN_CONFIG_H
#define POLE_PATTERN_CONFIG_H

#include "Pattern.h"
#include "ColorPalette.h"

/**
 * @brief Pole Pattern Configuration
 *
 * This file manages which pole patterns are available in your LED system.
 * Comment out patterns you don't want to use to save memory and reduce overhead.
 *
 * Pattern Order:
 * 0. Column Wave
 * 1. Spiral Chase
 * 2. Helix
 * 3. Fire
 * 4. Bounce
 */

// ============================================================================
// POLE PATTERN ENABLE/DISABLE
// ============================================================================
// Comment out any pattern you don't want to include

#define ENABLE_POLE_COLUMN_WAVE
#define ENABLE_POLE_SPIRAL_CHASE
#define ENABLE_POLE_HELIX
#define ENABLE_POLE_FIRE
#define ENABLE_POLE_BOUNCE

// ============================================================================
// POLE PATTERN MANAGER CLASS
// ============================================================================

class PolePatternManager
{
private:
  static const int MAX_POLE_PATTERNS = 10;
  PolePattern *patterns[MAX_POLE_PATTERNS];
  int patternCount;
  int currentPatternIndex;

  CRGB *mainLeds;
  int mainNumLeds;
  CRGB *poleLeds;
  int poleNumLeds;

public:
  /**
   * @brief Constructor
   */
  PolePatternManager(CRGB *mainLeds, int mainNumLeds, CRGB *poleLeds, int poleNumLeds);

  /**
   * @brief Destructor
   */
  ~PolePatternManager();

  /**
   * @brief Initialize and create all enabled pole patterns
   */
  void initialize();

  /**
   * @brief Add a pole pattern
   * @param pattern Pointer to PolePattern
   * @return True if added successfully
   */
  bool addPattern(PolePattern *pattern);

  /**
   * @brief Get pattern count
   * @return Number of patterns
   */
  int getPatternCount() const { return patternCount; }

  /**
   * @brief Get pattern by index
   * @param index Pattern index
   * @return Pointer to pattern or nullptr
   */
  PolePattern *getPattern(int index);

  /**
   * @brief Get current pattern
   * @return Pointer to current pattern
   */
  PolePattern *getCurrentPattern();

  /**
   * @brief Set current pattern index
   * @param index Pattern index
   */
  void setCurrentPattern(int index);

  /**
   * @brief Update current pattern
   * @param currentTime Current time in milliseconds
   * @return True if pattern was updated
   */
  bool update(unsigned long currentTime);

  /**
   * @brief Set palette for all pole patterns
   * @param palette Pointer to ColorPalette
   */
  void setPalette(ColorPalette *palette);

  /**
   * @brief Set brightness for all pole patterns
   * @param brightness Brightness value (0-255)
   */
  void setBrightness(uint8_t brightness);

  /**
   * @brief Set speed for all pole patterns
   * @param speed Speed multiplier
   */
  void setSpeed(float speed);

  /**
   * @brief Get pattern name by index
   * @param index Pattern index
   * @return Pattern name or "Unknown"
   */
  String getPatternName(int index);
};

#endif // POLE_PATTERN_CONFIG_H
