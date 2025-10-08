#ifndef WLED_PATTERNS_H
#define WLED_PATTERNS_H

#include <Arduino.h>
#include <FastLED.h>
#include "Pattern.h"
#include "SegmentManager.h"
#include "ColorPalette.h"

/**
 * @file WLEDPatterns.h
 * @brief WLED Effect Pattern Implementations
 * 
 * This file contains LED pattern implementations extracted and adapted from WLED.
 * All patterns inherit from the existing Pattern base class and are fully
 * compatible with the PatternManager and ColorPalette systems.
 * 
 * Source: WLED project (https://github.com/Aircoookie/WLED)
 * Adapted for: Totem LED Driver architecture with SegmentManager support
 */

// ============================================================================
// WLED PATTERN IMPLEMENTATIONS
// ============================================================================

/**
 * @brief Dancing Shadows - Dynamic shadow-like movement
 * Creates moving shadows with palette colors that dance across the strip
 */
class WLEDDancingShadowsPattern : public Pattern
{
private:
  uint16_t counter;
  
public:
  /**
   * @brief Constructor
   * @param leds Pointer to LED array
   * @param numLeds Number of LEDs
   */
  WLEDDancingShadowsPattern(CRGB *leds, int numLeds);
  
  /**
   * @brief Update pattern
   * @param currentTime Current time in milliseconds
   * @return true if pattern was updated
   */
  bool update(unsigned long currentTime) override;
  
  String getName() const override { return "Dancing Shadows"; }
  String getDescription() const override { return "Dynamic shadow-like movement (WLED)"; }
};

/**
 * @brief Color Waves - Smooth color waves traveling across strip
 * Multiple sine waves of different frequencies create flowing color patterns
 */
class WLEDColorWavesPattern : public Pattern
{
private:
  uint16_t counter;
  uint8_t waveSpeed;
  
public:
  /**
   * @brief Constructor
   * @param leds Pointer to LED array
   * @param numLeds Number of LEDs
   */
  WLEDColorWavesPattern(CRGB *leds, int numLeds);
  
  /**
   * @brief Update pattern
   * @param currentTime Current time in milliseconds
   * @return true if pattern was updated
   */
  bool update(unsigned long currentTime) override;
  
  /**
   * @brief Set wave speed
   * @param speed Wave speed (1-255)
   */
  void setWaveSpeed(uint8_t speed) { waveSpeed = speed; }
  
  String getName() const override { return "Color Waves"; }
  String getDescription() const override { return "Smooth flowing color waves (WLED)"; }
};

/**
 * @brief Noise Pattern - Perlin noise color field
 * Uses FastLED noise functions to create organic, shifting color patterns
 */
class WLEDNoisePattern : public Pattern
{
private:
  uint16_t scale;
  uint16_t noiseX;
  uint16_t noiseY;
  uint16_t noiseZ;
  
public:
  /**
   * @brief Constructor
   * @param leds Pointer to LED array
   * @param numLeds Number of LEDs
   */
  WLEDNoisePattern(CRGB *leds, int numLeds);
  
  /**
   * @brief Update pattern
   * @param currentTime Current time in milliseconds
   * @return true if pattern was updated
   */
  bool update(unsigned long currentTime) override;
  
  /**
   * @brief Set noise scale
   * @param newScale Scale factor for noise (affects detail level)
   */
  void setScale(uint16_t newScale) { scale = newScale; }
  
  String getName() const override { return "Noise"; }
  String getDescription() const override { return "Perlin noise organic patterns (WLED)"; }
};

/**
 * @brief Meteor Pattern - Smooth meteor trail effect
 * Creates a meteor with smooth fading trail
 */
class WLEDMeteorPattern : public Pattern
{
private:
  int meteorPosition;
  uint8_t meteorSize;
  uint8_t trailDecay;
  bool randomDecay;
  int direction;
  
public:
  /**
   * @brief Constructor
   * @param leds Pointer to LED array
   * @param numLeds Number of LEDs
   */
  WLEDMeteorPattern(CRGB *leds, int numLeds);
  
  /**
   * @brief Update pattern
   * @param currentTime Current time in milliseconds
   * @return true if pattern was updated
   */
  bool update(unsigned long currentTime) override;
  
  /**
   * @brief Set meteor size
   * @param size Size of meteor head (1-10)
   */
  void setMeteorSize(uint8_t size) { meteorSize = constrain(size, 1, 10); }
  
  /**
   * @brief Set trail decay rate
   * @param decay Decay rate (1-255, higher = faster decay)
   */
  void setTrailDecay(uint8_t decay) { trailDecay = decay; }
  
  String getName() const override { return "Meteor"; }
  String getDescription() const override { return "Smooth meteor with fading trail (WLED)"; }
};

/**
 * @brief Glitter Pattern - Sparkles on top of base pattern
 * Adds random white sparkles over the current palette colors
 */
class WLEDGlitterPattern : public Pattern
{
private:
  uint8_t glitterDensity;
  uint8_t glitterBrightness;
  
public:
  /**
   * @brief Constructor
   * @param leds Pointer to LED array
   * @param numLeds Number of LEDs
   */
  WLEDGlitterPattern(CRGB *leds, int numLeds);
  
  /**
   * @brief Update pattern
   * @param currentTime Current time in milliseconds
   * @return true if pattern was updated
   */
  bool update(unsigned long currentTime) override;
  
  /**
   * @brief Set glitter density
   * @param density Probability of sparkle (0-255)
   */
  void setDensity(uint8_t density) { glitterDensity = density; }
  
  String getName() const override { return "Glitter"; }
  String getDescription() const override { return "Sparkles over palette colors (WLED)"; }
};

/**
 * @brief Two Dots Pattern - Two dots bouncing and chasing
 * Two colored dots move in opposite directions with trails
 */
class WLEDTwoDotsPattern : public Pattern
{
private:
  float dot1Position;
  float dot2Position;
  float dot1Speed;
  float dot2Speed;
  bool dot1Direction;
  bool dot2Direction;
  uint8_t fadeRate;
  
public:
  /**
   * @brief Constructor
   * @param leds Pointer to LED array
   * @param numLeds Number of LEDs
   */
  WLEDTwoDotsPattern(CRGB *leds, int numLeds);
  
  /**
   * @brief Update pattern
   * @param currentTime Current time in milliseconds
   * @return true if pattern was updated
   */
  bool update(unsigned long currentTime) override;
  
  /**
   * @brief Set fade rate
   * @param rate Fade rate for trails (1-255)
   */
  void setFadeRate(uint8_t rate) { fadeRate = rate; }
  
  String getName() const override { return "Two Dots"; }
  String getDescription() const override { return "Two bouncing dots with trails (WLED)"; }
};

/**
 * @brief Colortwinkles Pattern - Palette-based twinkling
 * Random LEDs fade in and out with colors from the current palette
 */
class WLEDColortwinklesPattern : public Pattern
{
private:
  struct Twinkle {
    uint16_t ledIndex;
    uint8_t brightness;
    uint8_t colorIndex;
    int8_t fadeDirection;
    uint8_t fadeSpeed;
  };
  
  static const int MAX_TWINKLES = 20;
  Twinkle twinkles[MAX_TWINKLES];
  uint8_t twinkleCount;
  uint8_t spawnProbability;
  
  /**
   * @brief Spawn a new twinkle if slot available
   */
  void spawnTwinkle();
  
  /**
   * @brief Update a single twinkle
   * @param t Reference to twinkle struct
   */
  void updateTwinkle(Twinkle &t);
  
public:
  /**
   * @brief Constructor
   * @param leds Pointer to LED array
   * @param numLeds Number of LEDs
   */
  WLEDColortwinklesPattern(CRGB *leds, int numLeds);
  
  /**
   * @brief Update pattern
   * @param currentTime Current time in milliseconds
   * @return true if pattern was updated
   */
  bool update(unsigned long currentTime) override;
  
  /**
   * @brief Set spawn probability
   * @param prob Probability of new twinkle spawning (0-255)
   */
  void setSpawnProbability(uint8_t prob) { spawnProbability = prob; }
  
  String getName() const override { return "Colortwinkles"; }
  String getDescription() const override { return "Palette-based twinkling (WLED)"; }
};

/**
 * @brief Flow Pattern - Flowing palette colors
 * Smooth flowing effect where palette colors cycle through the strip
 */
class WLEDFlowPattern : public Pattern
{
private:
  uint16_t flowOffset;
  uint8_t flowSpeed;
  uint8_t blurAmount;
  
public:
  /**
   * @brief Constructor
   * @param leds Pointer to LED array
   * @param numLeds Number of LEDs
   */
  WLEDFlowPattern(CRGB *leds, int numLeds);
  
  /**
   * @brief Update pattern
   * @param currentTime Current time in milliseconds
   * @return true if pattern was updated
   */
  bool update(unsigned long currentTime) override;
  
  /**
   * @brief Set flow speed
   * @param speed Speed of flow (1-255)
   */
  void setFlowSpeed(uint8_t speed) { flowSpeed = speed; }
  
  /**
   * @brief Set blur amount
   * @param blur Amount of blur to apply (0-255)
   */
  void setBlurAmount(uint8_t blur) { blurAmount = blur; }
  
  String getName() const override { return "Flow"; }
  String getDescription() const override { return "Smooth flowing palette colors (WLED)"; }
};

#endif // WLED_PATTERNS_H

