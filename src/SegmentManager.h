#ifndef SEGMENT_MANAGER_H
#define SEGMENT_MANAGER_H

#include <FastLED.h>
#include "config.h"

/**
 * @brief Structure defining a LED segment/ring
 */
struct LEDSegment
{
  uint16_t rawStartIndex;     // Starting raw LED index (physical wiring)
  uint16_t count;             // Number of LEDs in this segment
  uint16_t rawEndIndex;       // Last raw LED index (physical wiring)
  uint8_t segmentType;        // Segment type (SEGMENT_CLOCK, SEGMENT_EYE_X, etc.)
  const char *name;           // Human-readable name
  const uint16_t *logicalMap; // Pointer to logical→raw mapping array (or nullptr for EYE_0)
};

/**
 * @brief Manages LED segments and provides utilities for multi-ring operations
 */
class SegmentManager
{
public:
  /**
   * @brief Constructor - initializes all segments
   */
  SegmentManager();

  /**
   * @brief Get segment information by segment type
   * @param segmentType The segment type (SEGMENT_EYE_4, etc.)
   * @return Pointer to LEDSegment structure, or nullptr if invalid
   */
  const LEDSegment *getSegment(uint8_t segmentType) const;

  /**
   * @brief Get segment containing the specified LED index
   * @param ledIndex The LED index to check
   * @return Pointer to LEDSegment structure, or nullptr if not found
   */
  const LEDSegment *getSegmentByLED(uint16_t ledIndex) const;

  /**
   * @brief Convert angular position (0.0-1.0) to LED index within a segment
   * @param segmentType The segment type
   * @param position Angular position (0.0 = start, 1.0 = full circle)
   * @return LED index within the segment, or -1 if invalid
   */
  int16_t getSegmentLEDByPosition(uint8_t segmentType, float position) const;

  /**
   * @brief Convert LED index within segment to angular position (0.0-1.0)
   * @param segmentType The segment type
   * @param ledIndex LED index within the segment (0-based)
   * @return Angular position (0.0-1.0), or -1.0 if invalid
   */
  float getPositionBySegmentLED(uint8_t segmentType, uint16_t ledIndex) const;

  /**
   * @brief Get the absolute LED index from segment type and relative index
   * @param segmentType The segment type
   * @param relativeIndex LED index within the segment (0-based)
   * @return Absolute LED index in the strip, or -1 if invalid
   */
  int16_t getAbsoluteLEDIndex(uint8_t segmentType, uint16_t relativeIndex) const;

  /**
   * @brief Set color for a specific position in a segment
   * @param leds FastLED array
   * @param segmentType The segment type
   * @param position Angular position (0.0-1.0)
   * @param color Color to set
   * @param width Number of LEDs to light up (centered on position)
   */
  void setSegmentPositionColor(CRGB *leds, uint8_t segmentType, float position,
                               CRGB color, uint8_t width = 1) const;

  /**
   * @brief Clear all LEDs in a specific segment
   * @param leds FastLED array
   * @param segmentType The segment type
   */
  void clearSegment(CRGB *leds, uint8_t segmentType) const;

  /**
   * @brief Fill entire segment with a color
   * @param leds FastLED array
   * @param segmentType The segment type
   * @param color Color to fill with
   */
  void fillSegment(CRGB *leds, uint8_t segmentType, CRGB color) const;

  /**
   * @brief Apply a gradient across a segment
   * @param leds FastLED array
   * @param segmentType The segment type
   * @param startColor Starting color
   * @param endColor Ending color
   */
  void fillSegmentGradient(CRGB *leds, uint8_t segmentType,
                           CRGB startColor, CRGB endColor) const;

  /**
   * @brief Get total number of segments
   * @return Number of segments
   */
  uint8_t getSegmentCount() const { return NUM_TOTAL_RINGS; }

  /**
   * @brief Get all segments (for iteration)
   * @return Pointer to segments array
   */
  const LEDSegment *getAllSegments() const { return segments; }

  /**
   * @brief Get raw LED index from logical index within a segment
   * @param segmentType The segment type
   * @param logicalIndex Logical LED index (0 = 12 o'clock)
   * @return Raw LED index, or -1 if invalid
   */
  int16_t getRawLEDIndex(uint8_t segmentType, uint16_t logicalIndex) const;

  /**
   * @brief Check if a segment type is valid
   * @param segmentType The segment type to check
   * @return True if valid, false otherwise
   */
  bool isValidSegment(uint8_t segmentType) const;

  /**
   * @brief Get segment name for debugging
   * @param segmentType The segment type
   * @return Segment name string
   */
  const char *getSegmentName(uint8_t segmentType) const;

  /**
   * @brief Print segment information to Serial (for debugging)
   */
  void printSegmentInfo() const;

  /**
   * @brief Get raw LED indices at a specific angle (0-360 degrees) for a segment
   * @param segmentType The segment type
   * @param angleDegrees Angle in degrees (0 = 12 o'clock, 90 = 3 o'clock, etc.)
   * @param width Number of LEDs to return (centered on angle, default = 1)
   * @param rawIndices Output array to store raw LED indices
   * @param maxIndices Maximum size of rawIndices array
   * @return Number of indices written to rawIndices array
   */
  uint8_t getRawLEDsAtAngle(uint8_t segmentType, float angleDegrees, uint8_t width,
                            uint16_t *rawIndices, uint8_t maxIndices) const;

  /**
   * @brief Get raw LED indices at a specific angle across multiple segments
   * @param segmentTypes Array of segment types to query
   * @param numSegments Number of segments in segmentTypes array
   * @param angleDegrees Angle in degrees (0 = 12 o'clock, 90 = 3 o'clock, etc.)
   * @param width Number of LEDs per segment (centered on angle)
   * @param rawIndices Output array to store raw LED indices
   * @param maxIndices Maximum size of rawIndices array
   * @return Number of indices written to rawIndices array
   */
  uint8_t getRawLEDsAtAngleMulti(const uint8_t *segmentTypes, uint8_t numSegments,
                                 float angleDegrees, uint8_t width,
                                 uint16_t *rawIndices, uint8_t maxIndices) const;

private:
  LEDSegment segments[NUM_TOTAL_RINGS]; ///< Array of all LED segments

  /**
   * @brief Initialize segment definitions
   */
  void initializeSegments();
};

#endif // SEGMENT_MANAGER_H
