#include "SegmentManager.h"
#include <Arduino.h>

SegmentManager::SegmentManager()
{
  initializeSegments();
}

void SegmentManager::initializeSegments()
{
  // Initialize segments with raw indices and logical mapping arrays
  // Note: Segments are indexed by their segment type constants

  segments[SEGMENT_CLOCK] = {
      CLOCK_RAW_START,
      CLOCK_COUNT,
      CLOCK_RAW_END,
      SEGMENT_CLOCK,
      "CLOCK",
      CLOCK_LED_MAP};

  segments[SEGMENT_EYE_4] = {
      EYE_4_RAW_START,
      EYE_4_COUNT,
      EYE_4_RAW_END,
      SEGMENT_EYE_4,
      "EYE_4",
      EYE_4_LED_MAP};

  segments[SEGMENT_EYE_3] = {
      EYE_3_RAW_START,
      EYE_3_COUNT,
      EYE_3_RAW_END,
      SEGMENT_EYE_3,
      "EYE_3",
      EYE_3_LED_MAP};

  segments[SEGMENT_EYE_2] = {
      EYE_2_RAW_START,
      EYE_2_COUNT,
      EYE_2_RAW_END,
      SEGMENT_EYE_2,
      "EYE_2",
      EYE_2_LED_MAP};

  segments[SEGMENT_EYE_1] = {
      EYE_1_RAW_START,
      EYE_1_COUNT,
      EYE_1_RAW_END,
      SEGMENT_EYE_1,
      "EYE_1",
      EYE_1_LED_MAP};

  segments[SEGMENT_EYE_0] = {
      EYE_0_RAW_START,
      EYE_0_COUNT,
      EYE_0_RAW_END,
      SEGMENT_EYE_0,
      "EYE_0",
      nullptr // Center LED has no mapping array (only 1 LED)
  };
}

const LEDSegment *SegmentManager::getSegment(uint8_t segmentType) const
{
  if (segmentType >= NUM_TOTAL_RINGS)
  {
    return nullptr;
  }
  return &segments[segmentType];
}

const LEDSegment *SegmentManager::getSegmentByLED(uint16_t ledIndex) const
{
  // Search by raw LED index
  for (uint8_t i = 0; i < NUM_TOTAL_RINGS; i++)
  {
    if (ledIndex >= segments[i].rawStartIndex && ledIndex <= segments[i].rawEndIndex)
    {
      return &segments[i];
    }
  }
  return nullptr;
}

int16_t SegmentManager::getRawLEDIndex(uint8_t segmentType, uint16_t logicalIndex) const
{
  const LEDSegment *segment = getSegment(segmentType);
  if (!segment || logicalIndex >= segment->count)
  {
    return -1;
  }

  // Center LED (EYE_0) has no mapping - return raw index directly
  if (segment->logicalMap == nullptr)
  {
    return segment->rawStartIndex;
  }

  // Use logical mapping array to get raw index
  return segment->logicalMap[logicalIndex];
}

int16_t SegmentManager::getSegmentLEDByPosition(uint8_t segmentType, float position) const
{
  const LEDSegment *segment = getSegment(segmentType);
  if (!segment || position < 0.0f || position > 1.0f)
  {
    return -1;
  }

  // Convert position to logical LED index within segment
  int16_t logicalIndex = (int16_t)(position * segment->count);

  // Handle wrap-around (position 1.0 should map to LED 0)
  if (logicalIndex >= segment->count)
  {
    logicalIndex = 0;
  }

  // Convert logical index to raw index using mapping array
  return getRawLEDIndex(segmentType, logicalIndex);
}

float SegmentManager::getPositionBySegmentLED(uint8_t segmentType, uint16_t ledIndex) const
{
  const LEDSegment *segment = getSegment(segmentType);
  if (!segment || ledIndex >= segment->count)
  {
    return -1.0f;
  }

  // Logical indices directly correspond to position
  // (0 = 12 o'clock, position 0.0)
  return (float)ledIndex / (float)segment->count;
}

int16_t SegmentManager::getAbsoluteLEDIndex(uint8_t segmentType, uint16_t relativeIndex) const
{
  // relativeIndex is treated as logical index
  // Return raw index using mapping
  return getRawLEDIndex(segmentType, relativeIndex);
}

void SegmentManager::setSegmentPositionColor(CRGB *leds, uint8_t segmentType, float position,
                                             CRGB color, uint8_t width) const
{
  const LEDSegment *segment = getSegment(segmentType);
  if (!segment || !leds)
  {
    return;
  }

  // Get logical center LED index
  int16_t logicalCenter = (int16_t)(position * segment->count);
  if (logicalCenter >= segment->count)
  {
    logicalCenter = 0;
  }

  // Calculate half-width for centering
  int8_t halfWidth = width / 2;

  // Light up LEDs around the center position
  for (int8_t i = -halfWidth; i <= halfWidth; i++)
  {
    int16_t logicalIndex = logicalCenter + i;

    // Handle wrap-around for circular segments
    if (logicalIndex < 0)
    {
      logicalIndex += segment->count;
    }
    else if (logicalIndex >= segment->count)
    {
      logicalIndex -= segment->count;
    }

    // Convert logical index to raw index
    int16_t rawIndex = getRawLEDIndex(segmentType, logicalIndex);

    if (rawIndex >= 0 && rawIndex < NUM_LEDS)
    {
      leds[rawIndex] = color;
    }
  }
}

void SegmentManager::clearSegment(CRGB *leds, uint8_t segmentType) const
{
  const LEDSegment *segment = getSegment(segmentType);
  if (!segment || !leds)
  {
    return;
  }

  // Clear all raw LEDs in the segment range
  for (uint16_t i = segment->rawStartIndex; i <= segment->rawEndIndex; i++)
  {
    leds[i] = CRGB::Black;
  }
}

void SegmentManager::fillSegment(CRGB *leds, uint8_t segmentType, CRGB color) const
{
  const LEDSegment *segment = getSegment(segmentType);
  if (!segment || !leds)
  {
    return;
  }

  // Fill using logical indices to maintain correct orientation
  for (uint16_t logicalIdx = 0; logicalIdx < segment->count; logicalIdx++)
  {
    int16_t rawIdx = getRawLEDIndex(segmentType, logicalIdx);
    if (rawIdx >= 0 && rawIdx < NUM_LEDS)
    {
      leds[rawIdx] = color;
    }
  }
}

void SegmentManager::fillSegmentGradient(CRGB *leds, uint8_t segmentType,
                                         CRGB startColor, CRGB endColor) const
{
  const LEDSegment *segment = getSegment(segmentType);
  if (!segment || !leds)
  {
    return;
  }

  // Create gradient using logical indices
  for (uint16_t logicalIdx = 0; logicalIdx < segment->count; logicalIdx++)
  {
    float ratio = (float)logicalIdx / (float)(segment->count - 1);
    CRGB blendedColor = blend(startColor, endColor, (uint8_t)(ratio * 255));

    int16_t rawIdx = getRawLEDIndex(segmentType, logicalIdx);
    if (rawIdx >= 0 && rawIdx < NUM_LEDS)
    {
      leds[rawIdx] = blendedColor;
    }
  }
}

bool SegmentManager::isValidSegment(uint8_t segmentType) const
{
  return segmentType < NUM_TOTAL_RINGS;
}

const char *SegmentManager::getSegmentName(uint8_t segmentType) const
{
  const LEDSegment *segment = getSegment(segmentType);
  return segment ? segment->name : "INVALID";
}

void SegmentManager::printSegmentInfo() const
{
  Serial.println("=== LED Segment Information ===");
  Serial.println("Physical Wiring: CLOCK (0-99) -> EYE rings (100-160)");
  Serial.println("Logical Indexing: All segments start at 12 o'clock (index 0)");
  Serial.println("");

  for (uint8_t i = 0; i < NUM_TOTAL_RINGS; i++)
  {
    const LEDSegment *seg = &segments[i];
    Serial.print("Segment ");
    Serial.print(i);
    Serial.print(" (");
    Serial.print(seg->name);
    Serial.print("): Raw LEDs ");
    Serial.print(seg->rawStartIndex);
    Serial.print("-");
    Serial.print(seg->rawEndIndex);
    Serial.print(" (");
    Serial.print(seg->count);
    Serial.print(" LEDs)");

    if (seg->logicalMap != nullptr)
    {
      Serial.print(" [Mapped]");
    }

    Serial.println();
  }

  Serial.print("Total LEDs: ");
  Serial.println(NUM_LEDS);
  Serial.println("===============================");
}

/**
 * @brief Get raw LED indices at a specific angle for a segment
 *
 * Converts angular position (0-360 degrees) to raw LED indices, respecting the
 * logical mapping where 0 degrees = 12 o'clock position.
 */
uint8_t SegmentManager::getRawLEDsAtAngle(uint8_t segmentType, float angleDegrees,
                                          uint8_t width, uint16_t *rawIndices,
                                          uint8_t maxIndices) const
{
  const LEDSegment *seg = getSegment(segmentType);
  if (!seg || !rawIndices || maxIndices == 0)
  {
    return 0;
  }

  // Normalize angle to 0-360 range
  while (angleDegrees < 0.0f)
    angleDegrees += 360.0f;
  while (angleDegrees >= 360.0f)
    angleDegrees -= 360.0f;

  // Convert angle to position (0.0-1.0)
  float position = angleDegrees / 360.0f;

  // Calculate center LED index in logical space
  int centerLogicalIndex = (int)(position * seg->count) % seg->count;

  // Calculate LED indices with width (spread around center)
  uint8_t numWritten = 0;
  int halfWidth = width / 2;

  for (int offset = -halfWidth; offset <= halfWidth && numWritten < maxIndices; offset++)
  {
    int logicalIndex = (centerLogicalIndex + offset + seg->count) % seg->count;

    // Get raw index from logical index
    int16_t rawIndex = getRawLEDIndex(segmentType, logicalIndex);
    if (rawIndex >= 0)
    {
      rawIndices[numWritten++] = (uint16_t)rawIndex;
    }
  }

  return numWritten;
}

/**
 * @brief Get raw LED indices at a specific angle across multiple segments
 *
 * This is useful for synchronized patterns that apply the same angular effect
 * across multiple rings (e.g., chase at 90 degrees on clock, eye_4, and eye_2).
 */
uint8_t SegmentManager::getRawLEDsAtAngleMulti(const uint8_t *segmentTypes,
                                               uint8_t numSegments, float angleDegrees,
                                               uint8_t width, uint16_t *rawIndices,
                                               uint8_t maxIndices) const
{
  if (!segmentTypes || !rawIndices || maxIndices == 0 || numSegments == 0)
  {
    return 0;
  }

  uint8_t totalWritten = 0;

  // Get LEDs from each segment
  for (uint8_t i = 0; i < numSegments; i++)
  {
    uint8_t remainingSpace = maxIndices - totalWritten;
    if (remainingSpace == 0)
    {
      break; // Output buffer full
    }

    uint8_t numWritten = getRawLEDsAtAngle(segmentTypes[i], angleDegrees, width,
                                           &rawIndices[totalWritten], remainingSpace);
    totalWritten += numWritten;
  }

  return totalWritten;
}
