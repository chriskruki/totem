#ifndef PATTERN_H
#define PATTERN_H

#include <Arduino.h>
#include <FastLED.h>
#include "config.h"
#include "SegmentManager.h"

// Forward declarations
class ColorPalette;
class SegmentManager;

// ============================================================================
// ACTION PATTERN SYSTEM
// ============================================================================

/**
 * @brief Base class for one-time triggerable actions
 *
 * ActionPatterns are different from regular patterns - they are triggered once
 * and run to completion, then become inactive. Multiple instances can run simultaneously.
 */
class ActionPattern
{
protected:
  CRGB *leds;      // Pointer to main LED array
  int numLeds;     // Number of main LEDs
  CRGB *poleLeds;  // Pointer to pole LED array (optional)
  int poleNumLeds; // Number of pole LEDs

  unsigned long startTime;      // When this action was triggered
  unsigned long lastUpdate;     // Last update time
  unsigned long updateInterval; // Update interval in milliseconds
  bool isActive;                // Is this action currently running
  bool isComplete;              // Has this action finished
  uint8_t brightness;           // Action brightness (0-255)
  float speed;                  // Action speed multiplier

public:
  /**
   * @brief Constructor for ActionPattern
   * @param leds Pointer to main LED array
   * @param numLeds Number of main LEDs
   * @param poleLeds Pointer to pole LED array (optional)
   * @param poleNumLeds Number of pole LEDs
   * @param updateInterval Update interval in milliseconds
   */
  ActionPattern(CRGB *leds, int numLeds, CRGB *poleLeds = nullptr, int poleNumLeds = 0, unsigned long updateInterval = 16);

  /**
   * @brief Virtual destructor
   */
  virtual ~ActionPattern() = default;

  /**
   * @brief Trigger this action to start
   * @param currentTime Current time in milliseconds
   */
  virtual void trigger(unsigned long currentTime);

  /**
   * @brief Update the action - must be implemented by derived classes
   * @param currentTime Current time in milliseconds
   * @return true if action was updated, false otherwise
   */
  virtual bool update(unsigned long currentTime) = 0;

  /**
   * @brief Check if action is currently active
   * @return true if active, false otherwise
   */
  bool getActive() const { return isActive; }

  /**
   * @brief Check if action has completed
   * @return true if complete, false otherwise
   */
  bool getComplete() const { return isComplete; }

  /**
   * @brief Set action brightness
   * @param brightness Brightness value (0-255)
   */
  virtual void setBrightness(uint8_t brightness) { this->brightness = brightness; }

  /**
   * @brief Set action speed
   * @param speed Speed multiplier
   */
  virtual void setSpeed(float speed) { this->speed = speed; }

  /**
   * @brief Get action name
   * @return Action name as String
   */
  virtual String getName() const = 0;

  /**
   * @brief Get action description
   * @return Action description as String
   */
  virtual String getDescription() const = 0;
};

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
 * @brief Ripple effect - bouncing ring pattern
 *
 * Creates a single active ring that bounces between innermost and outermost rings
 */
class RipplePattern : public Pattern
{
private:
  SegmentManager *segmentManager;
  float currentRingPosition; // Current ring position (0.0 to NUM_RINGS-1)
  bool bouncingOutward;      // Direction of bounce (true = outward, false = inward)
  float bounceSpeed;         // Speed of ring movement
  uint8_t ringIntensity;     // Brightness of the active ring
  unsigned long lastUpdate;  // Last update time for smooth animation

public:
  RipplePattern(CRGB *leds, int numLeds, SegmentManager *segManager, unsigned long interval = 1000);
  bool update(unsigned long currentTime) override;
  void setBounceSpeed(float speed);
  String getName() const override { return "Ripple"; }
  String getDescription() const override { return "Bouncing ring between inner and outer rings"; }
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

// ============================================================================
// POLE-SPECIFIC PATTERNS
// ============================================================================

/**
 * @brief Base class for pole-specific patterns
 *
 * Provides helper methods for pole spiral geometry calculations and palette support
 */
class PolePattern : public Pattern
{
protected:
  CRGB *poleLeds;        // Pointer to pole LED array
  int poleNumLeds;       // Number of pole LEDs
  ColorPalette *palette; // Color palette for this pattern

  /**
   * @brief Get spiral column for LED index
   * @param index LED index (0 to POLE_NUM_LEDS-1)
   * @return Column position (0 to POLE_SPIRAL_REPEAT-1)
   */
  uint8_t getPoleColumn(uint16_t index) const;

  /**
   * @brief Get spiral height level for LED index
   * @param index LED index (0 to POLE_NUM_LEDS-1)
   * @return Height level (0 to POLE_HEIGHT_LEVELS-1)
   */
  uint8_t getPoleHeight(uint16_t index) const;

  /**
   * @brief Get LED index for specific column and height
   * @param column Column (0 to POLE_SPIRAL_REPEAT-1)
   * @param height Height level (0 to POLE_HEIGHT_LEVELS-1)
   * @return LED index, or -1 if invalid
   */
  int getPoleIndex(uint8_t column, uint8_t height) const;

  /**
   * @brief Get color from palette at position
   * @param position Position in palette (0.0 to 1.0)
   * @return CRGB color
   */
  CRGB getPaletteColor(float position) const;

public:
  PolePattern(CRGB *leds, int numLeds, CRGB *poleLeds, int poleNumLeds, unsigned long updateInterval = 50);
  virtual ~PolePattern() = default;

  /**
   * @brief Set color palette for this pattern
   * @param palette Pointer to ColorPalette
   */
  virtual void setPalette(ColorPalette *palette) { this->palette = palette; }
};

/**
 * @brief Column wave pattern for pole
 *
 * Creates waves that travel up columns (LED 0,13,26,39... for column 0, etc.)
 */
class PoleColumnWavePattern : public PolePattern
{
private:
  float wavePosition;    // Current wave position (0.0 to POLE_SPIRAL_REPEAT)
  uint8_t waveWidth;     // Width of wave in columns
  bool reverseDirection; // Wave direction (up/down)

public:
  PoleColumnWavePattern(CRGB *leds, int numLeds, CRGB *poleLeds, int poleNumLeds);
  bool update(unsigned long currentTime) override;
  String getName() const override { return "PoleColumnWave"; }
  String getDescription() const override { return "Column waves traveling up the pole"; }
};

/**
 * @brief Spiral chase pattern for pole
 *
 * Creates a chase effect that follows the physical spiral geometry
 */
class PoleSpiralChasePattern : public PolePattern
{
private:
  float chasePosition; // Current chase position
  uint8_t chaseLength; // Length of chase tail
  uint8_t hueShift;    // Color cycling

public:
  PoleSpiralChasePattern(CRGB *leds, int numLeds, CRGB *poleLeds, int poleNumLeds);
  bool update(unsigned long currentTime) override;
  String getName() const override { return "PoleSpiralChase"; }
  String getDescription() const override { return "Chase effect following spiral geometry"; }
};

/**
 * @brief Helix pattern for pole
 *
 * Creates multiple helical waves around the pole
 */
class PoleHelixPattern : public PolePattern
{
private:
  float helixPhase;   // Current helix phase
  uint8_t numHelixes; // Number of parallel helixes
  float helixSpeed;   // Speed of helix rotation

public:
  PoleHelixPattern(CRGB *leds, int numLeds, CRGB *poleLeds, int poleNumLeds);
  bool update(unsigned long currentTime) override;
  String getName() const override { return "PoleHelix"; }
  String getDescription() const override { return "Multiple helical waves around pole"; }
};

/**
 * @brief Fire effect for pole
 *
 * Creates a fire effect that travels up the pole
 */
class PoleFirePattern : public PolePattern
{
private:
  uint8_t heat[POLE_NUM_LEDS]; // Heat map for fire effect
  uint8_t cooling;             // Cooling rate
  uint8_t sparking;            // Sparking probability

public:
  PoleFirePattern(CRGB *leds, int numLeds, CRGB *poleLeds, int poleNumLeds);
  bool update(unsigned long currentTime) override;
  String getName() const override { return "PoleFire"; }
  String getDescription() const override { return "Fire effect traveling up pole"; }
};

/**
 * @brief Bounce pattern for pole
 *
 * Creates two waves that bounce up and down in opposite directions
 */
class PoleBouncePattern : public PolePattern
{
private:
  float wave1Position; // Position of first wave (0.0 to 1.0)
  float wave2Position; // Position of second wave (0.0 to 1.0)
  bool wave1Direction; // True = up, False = down
  bool wave2Direction; // True = up, False = down
  uint8_t waveLength;  // Length of each wave
  uint8_t hueOffset;   // Color offset between waves

public:
  PoleBouncePattern(CRGB *leds, int numLeds, CRGB *poleLeds, int poleNumLeds);
  bool update(unsigned long currentTime) override;
  String getName() const override { return "PoleBounce"; }
  String getDescription() const override { return "Two waves bouncing up and down in opposite directions"; }
};

/**
 * @brief Firework action - one-time triggerable firework effect
 *
 * Creates a complete firework effect: white trail up pole, rainbow explosion from center outward, sparkly remnants
 * This is an ActionPattern that runs once when triggered and then becomes inactive
 */
class FireworkAction : public ActionPattern
{
private:
  // Action phases
  enum FireworkPhase
  {
    PHASE_LAUNCH,    // White trail traveling up pole
    PHASE_EXPLOSION, // Rainbow cascade from eye center to clock
    PHASE_SPARKLES   // Lingering sparkly remnants
  };

  FireworkPhase currentPhase;
  unsigned long phaseStartTime;

  // Launch phase (pole)
  float launchPosition;                         // Position of launch trail (0.0 to 1.0)
  static const uint32_t LAUNCH_DURATION = 1000; // 1 second launch time

  // Explosion phase (eye rings + clock)
  float explosionRadius;                          // Current explosion radius
  uint8_t explosionHue;                           // Current hue for rainbow effect
  static const uint32_t EXPLOSION_DURATION = 267; // Explosion animation time (3x faster)

  // Sparkle phase
  struct Sparkle
  {
    uint16_t ledIndex; // LED position
    uint8_t intensity; // Current brightness
    uint8_t hue;       // Sparkle color
    uint8_t decayRate; // How fast it fades
    bool active;       // Is this sparkle active
  };

  static const int MAX_SPARKLES = 30; // Reduced for multiple instances
  Sparkle sparkles[MAX_SPARKLES];
  static const uint32_t SPARKLE_DURATION = 1000; // 1 second of sparkles

  // Helper methods
  void updateLaunchPhase(unsigned long currentTime);
  void updateExplosionPhase(unsigned long currentTime);
  void updateSparklePhase(unsigned long currentTime);
  void initializeSparkles();
  void addRandomSparkles(int count);
  float getDistanceFromCenter(uint16_t ledIndex);
  uint16_t getRandomEyeOrClockLED();

public:
  FireworkAction(CRGB *leds, int numLeds, CRGB *poleLeds = nullptr, int poleNumLeds = 0);
  void trigger(unsigned long currentTime) override;
  bool update(unsigned long currentTime) override;
  String getName() const override { return "FireworkAction"; }
  String getDescription() const override { return "One-time firework launch and explosion"; }
};

#endif // PATTERN_H
