#ifndef PATTERN_H
#define PATTERN_H

#include <Arduino.h>
#include <FastLED.h>
#include "config.h"
#include "SegmentManager.h"

// Forward declarations
class ColorPalette;
class SegmentManager;

/**
 * @brief Base class for all LED patterns
 *
 * This abstract class defines the interface for LED patterns.
 * Each pattern must implement the update() method to define its behavior.
 */
class Pattern
{
protected:
  CRGB *leds;                   // Pointer to LED array
  int numLeds;                  // Number of LEDs
  unsigned long lastUpdate;     // Last update time
  unsigned long updateInterval; // Update interval in milliseconds
  bool isActive;                // Pattern active state
  ColorPalette *currentPalette; // Current color palette
  uint8_t brightness;           // Pattern brightness (0-255)
  float speed;                  // Pattern speed multiplier

public:
  /**
   * @brief Constructor for Pattern
   * @param leds Pointer to LED array
   * @param numLeds Number of LEDs
   * @param updateInterval Update interval in milliseconds
   */
  Pattern(CRGB *leds, int numLeds, unsigned long updateInterval = 50);

  /**
   * @brief Virtual destructor
   */
  virtual ~Pattern() = default;

  /**
   * @brief Pure virtual update method - must be implemented by derived classes
   * @param currentTime Current time in milliseconds
   * @return true if pattern was updated, false otherwise
   */
  virtual bool update(unsigned long currentTime) = 0;

  /**
   * @brief Initialize the pattern
   */
  virtual void initialize();

  /**
   * @brief Reset the pattern to initial state
   */
  virtual void reset();

  /**
   * @brief Set the color palette for this pattern
   * @param palette Pointer to ColorPalette
   */
  void setPalette(ColorPalette *palette);

  /**
   * @brief Set pattern brightness
   * @param brightness Brightness value (0-255)
   */
  void setBrightness(uint8_t brightness);

  /**
   * @brief Set pattern speed multiplier
   * @param speed Speed multiplier (0.1 to 10.0)
   */
  void setSpeed(float speed);

  /**
   * @brief Set pattern active state
   * @param active Active state
   */
  void setActive(bool active);

  /**
   * @brief Get pattern active state
   * @return true if pattern is active
   */
  bool getActive() const { return isActive; }

  /**
   * @brief Get pattern name
   * @return Pattern name as string
   */
  virtual String getName() const = 0;

  /**
   * @brief Get pattern description
   * @return Pattern description as string
   */
  virtual String getDescription() const = 0;
};

/**
 * @brief Solid color pattern - fills all LEDs with a single color
 */
class SolidPattern : public Pattern
{
private:
  CRGB color;

public:
  SolidPattern(CRGB *leds, int numLeds, CRGB color = CRGB::White);
  bool update(unsigned long currentTime) override;
  void setColor(CRGB newColor);
  String getName() const override { return "Solid"; }
  String getDescription() const override { return "Single solid color"; }
};

/**
 * @brief Rainbow pattern - cycles through rainbow colors
 */
class RainbowPattern : public Pattern
{
private:
  uint8_t hue;
  uint8_t deltaHue;

public:
  RainbowPattern(CRGB *leds, int numLeds);
  bool update(unsigned long currentTime) override;
  String getName() const override { return "Rainbow"; }
  String getDescription() const override { return "Cycling rainbow colors"; }
};

/**
 * @brief Chase pattern - moving dot with trail
 */
class ChasePattern : public Pattern
{
private:
  int position;
  int direction;
  uint8_t trailLength;
  CRGB chaseColor;

public:
  ChasePattern(CRGB *leds, int numLeds, CRGB color = CRGB::White, uint8_t trailLength = 5);
  bool update(unsigned long currentTime) override;
  void setChaseColor(CRGB color);
  void setTrailLength(uint8_t length);
  String getName() const override { return "Chase"; }
  String getDescription() const override { return "Moving dot with fading trail"; }
};

/**
 * @brief Pulse pattern - breathing effect
 */
class PulsePattern : public Pattern
{
private:
  uint8_t pulseValue;
  int8_t pulseDirection;
  CRGB pulseColor;

public:
  PulsePattern(CRGB *leds, int numLeds, CRGB color = CRGB::White);
  bool update(unsigned long currentTime) override;
  void setPulseColor(CRGB color);
  String getName() const override { return "Pulse"; }
  String getDescription() const override { return "Breathing/pulsing effect"; }
};

/**
 * @brief Twinkle pattern - random sparkling effect
 */
class TwinklePattern : public Pattern
{
private:
  uint8_t *twinkleState;
  unsigned long *twinkleTime;
  uint8_t density; // Percentage of LEDs that can twinkle (0-100)

public:
  TwinklePattern(CRGB *leds, int numLeds, uint8_t density = 20);
  ~TwinklePattern();
  bool update(unsigned long currentTime) override;
  void setDensity(uint8_t density);
  String getName() const override { return "Twinkle"; }
  String getDescription() const override { return "Random sparkling stars"; }
};

/**
 * @brief Fire pattern - simulates flickering fire effect
 */
class FirePattern : public Pattern
{
private:
  uint8_t *heat;
  uint8_t cooling;
  uint8_t sparking;

public:
  FirePattern(CRGB *leds, int numLeds);
  ~FirePattern();
  bool update(unsigned long currentTime) override;
  void setCooling(uint8_t cooling);
  void setSparking(uint8_t sparking);
  String getName() const override { return "Fire"; }
  String getDescription() const override { return "Flickering fire simulation"; }
};

/**
 * @brief Wave pattern - sine wave moving along strip
 */
class WavePattern : public Pattern
{
private:
  uint8_t wavePosition;
  uint8_t waveLength;
  CRGB waveColor;

public:
  WavePattern(CRGB *leds, int numLeds, CRGB color = CRGB::Blue, uint8_t waveLength = 20);
  bool update(unsigned long currentTime) override;
  void setWaveColor(CRGB color);
  void setWaveLength(uint8_t length);
  String getName() const override { return "Wave"; }
  String getDescription() const override { return "Sine wave animation"; }
};

/**
 * @brief Multi-ring synchronized pattern
 *
 * Runs the same pattern across all rings at synchronized positions
 */
class MultiRingPattern : public Pattern
{
private:
  SegmentManager *segmentManager;
  float currentPosition;
  uint8_t patternWidth;

public:
  MultiRingPattern(CRGB *leds, int numLeds, SegmentManager *segManager, uint8_t width = 3);
  bool update(unsigned long currentTime) override;
  void setPatternWidth(uint8_t width);
  String getName() const override { return "MultiRing"; }
  String getDescription() const override { return "Synchronized pattern across all rings"; }
};

/**
 * @brief Spiral pattern from center outward
 *
 * Creates spiral effects starting from EYE_0 and expanding outward
 */
class SpiralPattern : public Pattern
{
private:
  SegmentManager *segmentManager;
  float spiralPosition;
  uint8_t spiralWidth;
  bool expandingOut;
  uint8_t currentRing;

public:
  SpiralPattern(CRGB *leds, int numLeds, SegmentManager *segManager, uint8_t width = 2);
  bool update(unsigned long currentTime) override;
  void setSpiralWidth(uint8_t width);
  String getName() const override { return "Spiral"; }
  String getDescription() const override { return "Spiral effect from center outward"; }
};

/**
 * @brief Ripple effect from center
 *
 * Creates ripple waves emanating from the center LED
 */
class RipplePattern : public Pattern
{
private:
  SegmentManager *segmentManager;
  struct Ripple
  {
    float radius;
    uint8_t intensity;
    bool active;
  };
  static const uint8_t MAX_RIPPLES = 3;
  Ripple ripples[MAX_RIPPLES];
  unsigned long lastRippleTime;
  unsigned long rippleInterval;

public:
  RipplePattern(CRGB *leds, int numLeds, SegmentManager *segManager, unsigned long interval = 1000);
  bool update(unsigned long currentTime) override;
  void setRippleInterval(unsigned long interval);
  String getName() const override { return "Ripple"; }
  String getDescription() const override { return "Ripple waves from center"; }
};

/**
 * @brief Eye breathing pattern
 *
 * Makes the eye rings pulse/breathe in sequence
 */
class EyeBreathingPattern : public Pattern
{
private:
  SegmentManager *segmentManager;
  float breathPhase;
  uint8_t currentEyeRing;
  bool breathingIn;

public:
  EyeBreathingPattern(CRGB *leds, int numLeds, SegmentManager *segManager);
  bool update(unsigned long currentTime) override;
  String getName() const override { return "EyeBreathing"; }
  String getDescription() const override { return "Eye rings breathing effect"; }
};

/**
 * @brief Segment test pattern for debugging LED mapping
 *
 * Cycles through each segment individually to verify correct wiring
 */
class SegmentTestPattern : public Pattern
{
private:
  SegmentManager *segmentManager;
  uint8_t currentSegment;
  unsigned long lastSegmentChange;
  unsigned long segmentInterval;

public:
  SegmentTestPattern(CRGB *leds, int numLeds, SegmentManager *segManager, unsigned long interval = SEGMENT_TEST_INTERVAL);
  bool update(unsigned long currentTime) override;
  void setSegmentInterval(unsigned long interval);
  String getName() const override { return "SegmentTest"; }
  String getDescription() const override { return "Test pattern for segment verification"; }
};

#endif // PATTERN_H
