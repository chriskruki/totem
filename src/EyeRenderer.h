#ifndef EYE_RENDERER_H
#define EYE_RENDERER_H

#include <FastLED.h>
#include "config.h"
#include "SegmentManager.h"

/**
 * @brief Simple eye rendering system for pointer mode
 *
 * Creates a 5-LED iris that moves around the eye rings based on joystick position.
 * Uses preset positions for different directions to create a low-res "pixel eye" effect.
 */
class EyeRenderer
{
public:
  /**
   * @brief Constructor
   * @param leds Pointer to LED array
   * @param segManager Pointer to SegmentManager
   */
  EyeRenderer(CRGB *leds, SegmentManager *segManager);

  /**
   * @brief Update eye position based on joystick input
   * @param joystickX Joystick X position (-2048 to +2048 range)
   * @param joystickY Joystick Y position (-2048 to +2048 range)
   */
  void updateEyePosition(int joystickX, int joystickY);

  /**
   * @brief Render the eye to the LED array
   */
  void renderEye();

  /**
   * @brief Set eye colors
   * @param irisColor Color of the iris LEDs
   * @param scleraColor Color of the background (off LEDs will be black)
   */
  void setEyeColors(CRGB irisColor, CRGB scleraColor);

private:
  CRGB *leds;                     ///< Pointer to LED array
  SegmentManager *segmentManager; ///< Pointer to segment manager

  // Current eye direction (0-8: Center, N, NE, E, SE, S, SW, W, NW)
  uint8_t currentDirection;

  // Eye colors
  CRGB irisColor;   ///< Iris color
  CRGB scleraColor; ///< Sclera (background) color

  // Preset iris positions for each direction
  struct IrisPosition
  {
    uint8_t ledCount;       ///< Number of LEDs to light up (usually 5)
    uint16_t ledIndices[5]; ///< Absolute LED indices to light up
  };

  static const uint8_t NUM_DIRECTIONS = 9; // Center + 8 directions
  IrisPosition irisPositions[NUM_DIRECTIONS];

  /**
   * @brief Initialize preset iris positions for all directions
   */
  void initializeIrisPositions();

  /**
   * @brief Convert joystick coordinates to direction index
   * @param joystickX Joystick X position
   * @param joystickY Joystick Y position
   * @return Direction index (0-8)
   */
  uint8_t calculateDirection(int joystickX, int joystickY);

  /**
   * @brief Get angle from joystick coordinates
   * @param x X coordinate (relative to center)
   * @param y Y coordinate (relative to center)
   * @return Angle in degrees (0-360, 0 = North)
   */
  float getAngleDegrees(float x, float y);
};

#endif // EYE_RENDERER_H
