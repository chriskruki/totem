#include "SegmentManager.h"
#include <Arduino.h>

SegmentManager::SegmentManager()
{
  initializeSegments();
}

void SegmentManager::initializeSegments()
{
  // Initialize all segments based on config.h definitions
  segments[SEGMENT_EYE_4] = {EYE_4_START, EYE_4_COUNT, EYE_4_END, SEGMENT_EYE_4, "EYE_4"};
  segments[SEGMENT_EYE_3] = {EYE_3_START, EYE_3_COUNT, EYE_3_END, SEGMENT_EYE_3, "EYE_3"};
  segments[SEGMENT_EYE_2] = {EYE_2_START, EYE_2_COUNT, EYE_2_END, SEGMENT_EYE_2, "EYE_2"};
  segments[SEGMENT_EYE_1] = {EYE_1_START, EYE_1_COUNT, EYE_1_END, SEGMENT_EYE_1, "EYE_1"};
  segments[SEGMENT_EYE_0] = {EYE_0_START, EYE_0_COUNT, EYE_0_END, SEGMENT_EYE_0, "EYE_0"};
  segments[SEGMENT_CLOCK] = {CLOCK_START, CLOCK_COUNT, CLOCK_END, SEGMENT_CLOCK, "CLOCK"};
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
  for (uint8_t i = 0; i < NUM_TOTAL_RINGS; i++)
  {
    if (ledIndex >= segments[i].startIndex && ledIndex <= segments[i].endIndex)
    {
      return &segments[i];
    }
  }
  return nullptr;
}

int16_t SegmentManager::getSegmentLEDByPosition(uint8_t segmentType, float position) const
{
  const LEDSegment *segment = getSegment(segmentType);
  if (!segment || position < 0.0f || position > 1.0f)
  {
    return -1;
  }

  // Convert position to LED index within segment
  int16_t ledIndex = (int16_t)(position * segment->count);

  // Handle wrap-around (position 1.0 should map to LED 0)
  if (ledIndex >= segment->count)
  {
    ledIndex = 0;
  }

  return ledIndex;
}

float SegmentManager::getPositionBySegmentLED(uint8_t segmentType, uint16_t ledIndex) const
{
  const LEDSegment *segment = getSegment(segmentType);
  if (!segment || ledIndex >= segment->count)
  {
    return -1.0f;
  }

  return (float)ledIndex / (float)segment->count;
}

int16_t SegmentManager::getAbsoluteLEDIndex(uint8_t segmentType, uint16_t relativeIndex) const
{
  const LEDSegment *segment = getSegment(segmentType);
  if (!segment || relativeIndex >= segment->count)
  {
    return -1;
  }

  return segment->startIndex + relativeIndex;
}

void SegmentManager::setSegmentPositionColor(CRGB *leds, uint8_t segmentType, float position,
                                             CRGB color, uint8_t width) const
{
  const LEDSegment *segment = getSegment(segmentType);
  if (!segment || !leds)
  {
    return;
  }

  int16_t centerLED = getSegmentLEDByPosition(segmentType, position);
  if (centerLED < 0)
  {
    return;
  }

  // Calculate half-width for centering
  int8_t halfWidth = width / 2;

  // Light up LEDs around the center position
  for (int8_t i = -halfWidth; i <= halfWidth; i++)
  {
    int16_t ledIndex = centerLED + i;

    // Handle wrap-around for circular segments
    if (ledIndex < 0)
    {
      ledIndex += segment->count;
    }
    else if (ledIndex >= segment->count)
    {
      ledIndex -= segment->count;
    }

    // Convert to absolute LED index
    int16_t absoluteIndex = segment->startIndex + ledIndex;

    if (absoluteIndex >= 0 && absoluteIndex < NUM_LEDS)
    {
      leds[absoluteIndex] = color;
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

  for (uint16_t i = segment->startIndex; i <= segment->endIndex; i++)
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

  for (uint16_t i = segment->startIndex; i <= segment->endIndex; i++)
  {
    leds[i] = color;
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

  // Create gradient across the segment
  for (uint16_t i = 0; i < segment->count; i++)
  {
    float ratio = (float)i / (float)(segment->count - 1);
    CRGB blendedColor = blend(startColor, endColor, (uint8_t)(ratio * 255));
    leds[segment->startIndex + i] = blendedColor;
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
  for (uint8_t i = 0; i < NUM_TOTAL_RINGS; i++)
  {
    const LEDSegment *seg = &segments[i];
    Serial.print("Segment ");
    Serial.print(i);
    Serial.print(" (");
    Serial.print(seg->name);
    Serial.print("): LEDs ");
    Serial.print(seg->startIndex);
    Serial.print("-");
    Serial.print(seg->endIndex);
    Serial.print(" (");
    Serial.print(seg->count);
    Serial.println(" LEDs)");
  }
  Serial.print("Total LEDs: ");
  Serial.println(NUM_LEDS);
  Serial.println("===============================");
}
