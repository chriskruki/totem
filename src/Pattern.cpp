#include "Pattern.h"
#include "ColorPalette.h"

// ============================================================================
// Base Pattern Class Implementation
// ============================================================================

Pattern::Pattern(CRGB *leds, int numLeds, unsigned long updateInterval)
    : leds(leds), numLeds(numLeds), updateInterval(updateInterval),
      lastUpdate(0), isActive(true), currentPalette(nullptr),
      brightness(255), speed(DEFAULT_GLOBAL_SPEED), speedNormalizationFactor(1.0f)
{
}

void Pattern::initialize()
{
  lastUpdate = 0;
  isActive = true;
  reset();
}

void Pattern::reset()
{
  // Default implementation - clear all LEDs
  fill_solid(leds, numLeds, CRGB::Black);
}

void Pattern::setPalette(ColorPalette *palette)
{
  currentPalette = palette;
}

void Pattern::setBrightness(uint8_t brightness)
{
  this->brightness = brightness;
}

void Pattern::setSpeed(float speed)
{
  this->speed = constrain(speed, SETTINGS_SPEED_MIN, SETTINGS_SPEED_MAX);
  // Adjust update interval based on speed
  updateInterval = (unsigned long)(50.0f / speed);
}

void Pattern::setActive(bool active)
{
  isActive = active;
  if (!active)
  {
    fill_solid(leds, numLeds, CRGB::Black);
  }
}

// ============================================================================
// Solid Pattern Implementation
// ============================================================================

SolidPattern::SolidPattern(CRGB *leds, int numLeds, CRGB color)
    : Pattern(leds, numLeds, 100), color(color)
{
}

bool SolidPattern::update(unsigned long currentTime)
{
  if (!isActive || (currentTime - lastUpdate) < updateInterval)
  {
    return false;
  }

  lastUpdate = currentTime;

  // Get color from palette or use default color
  CRGB scaledColor;
  if (currentPalette)
  {
    // Use first color from palette for solid pattern
    scaledColor = currentPalette->getColor(0);
  }
  else
  {
    scaledColor = color; // Fallback to hardcoded color
  }

  // Apply brightness scaling
  scaledColor.nscale8(brightness);

  fill_solid(leds, numLeds, scaledColor);
  return true;
}

void SolidPattern::setColor(CRGB newColor)
{
  color = newColor;
}

// ============================================================================
// Rainbow Pattern Implementation
// ============================================================================

RainbowPattern::RainbowPattern(CRGB *leds, int numLeds)
    : Pattern(leds, numLeds, 20), hue(0), deltaHue(255 / numLeds)
{
}

bool RainbowPattern::update(unsigned long currentTime)
{
  if (!isActive || (currentTime - lastUpdate) < (updateInterval / speed))
  {
    return false;
  }

  lastUpdate = currentTime;

  // Apply rainbow using logical indices
  for (int logicalIndex = 0; logicalIndex < numLeds; logicalIndex++)
  {
    // Calculate hue for this logical position
    uint8_t pixelHue = hue + (logicalIndex * deltaHue);

    // Convert logical index to raw index
    uint16_t rawIndex = logicalToRawIndex(logicalIndex);

    // Set color at raw index
    leds[rawIndex] = CHSV(pixelHue, 255, brightness);
  }

  hue += 1;
  return true;
}

// ============================================================================
// Chase Pattern Implementation
// ============================================================================

ChasePattern::ChasePattern(CRGB *leds, int numLeds, CRGB color, uint8_t trailLength)
    : Pattern(leds, numLeds, 50), position(0), direction(1),
      trailLength(trailLength), chaseColor(color)
{
}

bool ChasePattern::update(unsigned long currentTime)
{
  if (!isActive || (currentTime - lastUpdate) < (updateInterval / speed))
  {
    return false;
  }

  lastUpdate = currentTime;

  // Fade all LEDs
  for (int i = 0; i < numLeds; i++)
  {
    leds[i].fadeToBlackBy(60);
  }

  // Draw the chase dot and trail (using logical indices)
  for (int i = 0; i < trailLength; i++)
  {
    // Calculate logical index (0-160, where 0 = 12 o'clock)
    int logicalIndex = position - i;
    if (logicalIndex < 0)
      logicalIndex += numLeds;
    if (logicalIndex >= numLeds)
      logicalIndex -= numLeds;

    // Convert logical index to raw index using LED mapping
    uint16_t rawIndex = logicalToRawIndex(logicalIndex);

    // Get color from palette based on position, or use default color
    CRGB trailColor;
    if (currentPalette)
    {
      // Sample palette based on logical position for color variation
      uint8_t paletteIndex = map(logicalIndex, 0, numLeds - 1, 0, 255);
      trailColor = currentPalette->getColor(paletteIndex);
    }
    else
    {
      trailColor = chaseColor; // Fallback to hardcoded color
    }

    uint8_t brightness_scale = map(i, 0, trailLength - 1, brightness, brightness / 4);
    trailColor.nscale8(brightness_scale);

    // Set LED at raw index (physical position)
    leds[rawIndex] = trailColor;
  }

  // Move position (in logical space)
  position += direction;
  if (position >= numLeds)
  {
    position = 0;
  }
  else if (position < 0)
  {
    position = numLeds - 1;
  }

  return true;
}

void ChasePattern::setChaseColor(CRGB color)
{
  chaseColor = color;
}

void ChasePattern::setTrailLength(uint8_t length)
{
  trailLength = constrain(length, 1, numLeds / 2);
}

// ============================================================================
// Synchronized Chase Pattern Implementation
// ============================================================================

/**
 * @brief Constructor for SynchronizedChasePattern
 *
 * Creates a chase pattern that synchronizes across multiple segments by angular position.
 * Example: Chase at 12 o'clock appears at the top of all selected rings simultaneously.
 */
SynchronizedChasePattern::SynchronizedChasePattern(CRGB *leds, int numLeds,
                                                   SegmentManager *segManager,
                                                   const uint8_t *segments,
                                                   uint8_t numSegments,
                                                   uint8_t trailWidth)
    : Pattern(leds, numLeds, 40), segmentManager(segManager), currentAngle(0.0f),
      angularSpeed(5.0f), trailWidth(trailWidth), numTargetSegments(0), palette(nullptr)
{
  setTargetSegments(segments, numSegments);
}

bool SynchronizedChasePattern::update(unsigned long currentTime)
{
  if (!isActive || !segmentManager || (currentTime - lastUpdate) < updateInterval)
  {
    return false;
  }

  lastUpdate = currentTime;

  // Clear all LEDs
  fill_solid(leds, numLeds, CRGB::Black);

  // Apply effective speed (respects speedNormalizationFactor)
  float effectiveAngularSpeed = angularSpeed * getEffectiveSpeed();

  // Update angle position
  currentAngle += effectiveAngularSpeed;
  if (currentAngle >= 360.0f)
  {
    currentAngle -= 360.0f;
  }

  // Buffer to hold raw LED indices (max possible: ~20 LEDs across all segments)
  uint16_t rawIndices[30];

  // Get raw LED indices at current angle across all target segments
  uint8_t numLEDs = segmentManager->getRawLEDsAtAngleMulti(
      targetSegments, numTargetSegments, currentAngle, trailWidth, rawIndices, 30);

  // Light up all LEDs at this angle
  for (uint8_t i = 0; i < numLEDs; i++)
  {
    uint16_t rawIndex = rawIndices[i];
    if (rawIndex < (uint16_t)numLeds)
    {
      // Get color from palette based on angle, or use white
      CRGB color;
      if (currentPalette)
      {
        uint8_t paletteIndex = (uint8_t)((currentAngle / 360.0f) * 255.0f);
        color = currentPalette->getColor(paletteIndex);
      }
      else
      {
        color = CRGB::White;
      }

      // Apply brightness
      color.nscale8(brightness);

      leds[rawIndex] = color;
    }
  }

  // Add fading trail effect
  uint8_t numTrailSteps = 3; // Number of trailing positions
  for (uint8_t t = 1; t <= numTrailSteps; t++)
  {
    float trailAngle = currentAngle - (effectiveAngularSpeed * t);
    while (trailAngle < 0.0f)
      trailAngle += 360.0f;

    numLEDs = segmentManager->getRawLEDsAtAngleMulti(
        targetSegments, numTargetSegments, trailAngle, trailWidth, rawIndices, 30);

    uint8_t trailBrightness = brightness / (t + 1); // Fade based on distance

    for (uint8_t i = 0; i < numLEDs; i++)
    {
      uint16_t rawIndex = rawIndices[i];
      if (rawIndex < (uint16_t)numLeds)
      {
        CRGB color;
        if (currentPalette)
        {
          uint8_t paletteIndex = (uint8_t)((trailAngle / 360.0f) * 255.0f);
          color = currentPalette->getColor(paletteIndex);
        }
        else
        {
          color = CRGB::White;
        }

        color.nscale8(trailBrightness);

        // Blend with existing color (for overlapping trails)
        leds[rawIndex] += color;
      }
    }
  }

  return true;
}

void SynchronizedChasePattern::setTargetSegments(const uint8_t *segments, uint8_t numSegments)
{
  // Copy segment types to internal array (max 6 segments)
  numTargetSegments = min(numSegments, (uint8_t)6);
  for (uint8_t i = 0; i < numTargetSegments; i++)
  {
    targetSegments[i] = segments[i];
  }
}

// ============================================================================
// Pulse Pattern Implementation
// ============================================================================

PulsePattern::PulsePattern(CRGB *leds, int numLeds, CRGB color)
    : Pattern(leds, numLeds, 10), pulseValue(0), pulseDirection(1), pulseColor(color)
{
}

bool PulsePattern::update(unsigned long currentTime)
{
  if (!isActive || (currentTime - lastUpdate) < (updateInterval / speed))
  {
    return false;
  }

  lastUpdate = currentTime;

  // Update pulse value
  pulseValue += pulseDirection * 2;
  if (pulseValue >= 255)
  {
    pulseValue = 255;
    pulseDirection = -1;
  }
  else if (pulseValue <= 0)
  {
    pulseValue = 0;
    pulseDirection = 1;
  }

  // Apply pulse to all LEDs
  CRGB scaledColor;
  if (currentPalette)
  {
    // Sample palette based on pulse value for color variation
    uint8_t paletteIndex = map(pulseValue, 0, 255, 0, 255);
    scaledColor = currentPalette->getColor(paletteIndex);
  }
  else
  {
    scaledColor = pulseColor; // Fallback to hardcoded color
  }

  uint8_t finalBrightness = map(pulseValue, 0, 255, 0, brightness);
  scaledColor.nscale8(finalBrightness);

  fill_solid(leds, numLeds, scaledColor);
  return true;
}

void PulsePattern::setPulseColor(CRGB color)
{
  pulseColor = color;
}

// ============================================================================
// Twinkle Pattern Implementation
// ============================================================================

TwinklePattern::TwinklePattern(CRGB *leds, int numLeds, uint8_t density)
    : Pattern(leds, numLeds, 50), density(density)
{
  twinkleState = new uint8_t[numLeds];
  twinkleTime = new unsigned long[numLeds];

  // Initialize twinkle arrays
  for (int i = 0; i < numLeds; i++)
  {
    twinkleState[i] = 0;
    twinkleTime[i] = 0;
  }
}

TwinklePattern::~TwinklePattern()
{
  delete[] twinkleState;
  delete[] twinkleTime;
}

bool TwinklePattern::update(unsigned long currentTime)
{
  if (!isActive || (currentTime - lastUpdate) < (updateInterval / speed))
  {
    return false;
  }

  lastUpdate = currentTime;

  // Fade all LEDs
  for (int i = 0; i < numLeds; i++)
  {
    leds[i].fadeToBlackBy(10);
  }

  // Update existing twinkles
  for (int i = 0; i < numLeds; i++)
  {
    if (twinkleState[i] > 0)
    {
      if (currentTime - twinkleTime[i] > 100)
      {
        twinkleState[i]--;
        twinkleTime[i] = currentTime;

        CRGB twinkleColor = CRGB::White;
        if (currentPalette)
        {
          twinkleColor = currentPalette->getColor(random8());
        }
        twinkleColor.nscale8(map(twinkleState[i], 0, 10, 0, brightness));
        leds[i] = twinkleColor;
      }
    }
  }

  // Start new twinkles
  if (random8(100) < density)
  {
    int randomLed = random16(numLeds);
    if (twinkleState[randomLed] == 0)
    {
      twinkleState[randomLed] = 10;
      twinkleTime[randomLed] = currentTime;
    }
  }

  return true;
}

void TwinklePattern::setDensity(uint8_t density)
{
  this->density = constrain(density, 1, 100);
}

// ============================================================================
// Fire Pattern Implementation
// ============================================================================

FirePattern::FirePattern(CRGB *leds, int numLeds)
    : Pattern(leds, numLeds, 30), cooling(55), sparking(120)
{
  heat = new uint8_t[numLeds];

  // Initialize heat array
  for (int i = 0; i < numLeds; i++)
  {
    heat[i] = 0;
  }
}

FirePattern::~FirePattern()
{
  delete[] heat;
}

bool FirePattern::update(unsigned long currentTime)
{
  if (!isActive || (currentTime - lastUpdate) < (updateInterval / speed))
  {
    return false;
  }

  lastUpdate = currentTime;

  // Step 1: Cool down every cell a little
  for (int i = 0; i < numLeds; i++)
  {
    heat[i] = qsub8(heat[i], random8(0, ((cooling * 10) / numLeds) + 2));
  }

  // Step 2: Heat from each cell drifts 'up' and diffuses a little
  for (int k = numLeds - 1; k >= 2; k--)
  {
    heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2]) / 3;
  }

  // Step 3: Randomly ignite new 'sparks' of heat near the bottom
  if (random8() < sparking)
  {
    int y = random8(7);
    heat[y] = qadd8(heat[y], random8(160, 255));
  }

  // Step 4: Map from heat cells to LED colors
  for (int j = 0; j < numLeds; j++)
  {
    CRGB color = HeatColor(heat[j]);
    color.nscale8(brightness);
    leds[j] = color;
  }

  return true;
}

void FirePattern::setCooling(uint8_t cooling)
{
  this->cooling = cooling;
}

void FirePattern::setSparking(uint8_t sparking)
{
  this->sparking = sparking;
}

// ============================================================================
// Wave Pattern Implementation
// ============================================================================

WavePattern::WavePattern(CRGB *leds, int numLeds, CRGB color, uint8_t waveLength)
    : Pattern(leds, numLeds, 30), wavePosition(0), waveLength(waveLength), waveColor(color)
{
}

bool WavePattern::update(unsigned long currentTime)
{
  if (!isActive || (currentTime - lastUpdate) < (updateInterval / speed))
  {
    return false;
  }

  lastUpdate = currentTime;

  // Clear all LEDs
  fill_solid(leds, numLeds, CRGB::Black);

  // Draw sine wave using logical indices
  for (int logicalIndex = 0; logicalIndex < numLeds; logicalIndex++)
  {
    float angle = (float)(logicalIndex + wavePosition) * 2.0 * PI / waveLength;
    uint8_t intensity = (uint8_t)((sin(angle) + 1.0) * 127.5);

    // Get color from palette based on logical position, or use default color
    CRGB scaledColor;
    if (currentPalette)
    {
      // Sample palette based on logical position for color variation
      uint8_t paletteIndex = map(logicalIndex, 0, numLeds - 1, 0, 255);
      scaledColor = currentPalette->getColor(paletteIndex);
    }
    else
    {
      scaledColor = waveColor; // Fallback to hardcoded color
    }

    scaledColor.nscale8(map(intensity, 0, 255, 0, brightness));

    // Convert logical index to raw index and set LED
    uint16_t rawIndex = logicalToRawIndex(logicalIndex);
    leds[rawIndex] = scaledColor;
  }

  wavePosition++;
  if (wavePosition >= waveLength)
  {
    wavePosition = 0;
  }

  return true;
}

void WavePattern::setWaveColor(CRGB color)
{
  waveColor = color;
}

void WavePattern::setWaveLength(uint8_t length)
{
  waveLength = constrain(length, 4, numLeds);
}

// ============================================================================
// Multi-Ring Pattern Implementation
// ============================================================================

MultiRingPattern::MultiRingPattern(CRGB *leds, int numLeds, SegmentManager *segManager, uint8_t width)
    : Pattern(leds, numLeds, 50), segmentManager(segManager), currentPosition(0.0f), patternWidth(width)
{
}

bool MultiRingPattern::update(unsigned long currentTime)
{
  if (!isActive || (currentTime - lastUpdate) < updateInterval)
  {
    return false;
  }

  lastUpdate = currentTime;

  // Clear all LEDs
  fill_solid(leds, numLeds, CRGB::Black);

  if (!segmentManager || !currentPalette)
  {
    return true;
  }

  // Update position based on speed
  currentPosition += speed * 0.01f;
  if (currentPosition >= 1.0f)
  {
    currentPosition -= 1.0f;
  }

  // Sample color from palette
  CRGB color = currentPalette->getColorSmooth(currentPosition);

  // Apply pattern to all segments at the same relative position
  for (uint8_t segment = 0; segment < segmentManager->getSegmentCount(); segment++)
  {
    segmentManager->setSegmentPositionColor(leds, segment, currentPosition, color, patternWidth);
  }

  return true;
}

void MultiRingPattern::setPatternWidth(uint8_t width)
{
  patternWidth = constrain(width, 1, 10);
}

// ============================================================================
// Spiral Pattern Implementation
// ============================================================================

SpiralPattern::SpiralPattern(CRGB *leds, int numLeds, SegmentManager *segManager, uint8_t width)
    : Pattern(leds, numLeds, 80), segmentManager(segManager), spiralPosition(0.0f),
      spiralWidth(width), expandingOut(true), currentRing(0)
{
}

bool SpiralPattern::update(unsigned long currentTime)
{
  if (!isActive || (currentTime - lastUpdate) < updateInterval)
  {
    return false;
  }

  lastUpdate = currentTime;

  // Clear all LEDs
  fill_solid(leds, numLeds, CRGB::Black);

  if (!segmentManager || !currentPalette)
  {
    return true;
  }

  // Update spiral position
  spiralPosition += speed * 0.02f;

  if (expandingOut)
  {
    // Expanding from center outward
    if (spiralPosition >= 1.0f)
    {
      spiralPosition = 0.0f;
      currentRing++;
      if (currentRing >= segmentManager->getSegmentCount())
      {
        currentRing = 0;
      }
    }
  }
  else
  {
    // Contracting toward center
    if (spiralPosition >= 1.0f)
    {
      spiralPosition = 0.0f;
      if (currentRing == 0)
      {
        currentRing = segmentManager->getSegmentCount() - 1;
      }
      else
      {
        currentRing--;
      }
    }
  }

  // Sample color from palette based on ring
  float colorPosition = (float)currentRing / (float)segmentManager->getSegmentCount();
  CRGB color = currentPalette->getColorSmooth(colorPosition);

  // Draw spiral effect - light up current ring and fade previous rings
  for (uint8_t ring = 0; ring <= currentRing; ring++)
  {
    uint8_t fade = 255 - (currentRing - ring) * 60;
    CRGB fadedColor = color;
    fadedColor.nscale8(fade);

    float ringPosition = spiralPosition;
    if (ring < currentRing)
    {
      ringPosition = 1.0f; // Previous rings are fully lit
    }

    segmentManager->setSegmentPositionColor(leds, ring, ringPosition, fadedColor, spiralWidth);
  }

  return true;
}

void SpiralPattern::setSpiralWidth(uint8_t width)
{
  spiralWidth = constrain(width, 1, 8);
}

// ============================================================================
// Ripple Pattern Implementation
// ============================================================================

RipplePattern::RipplePattern(CRGB *leds, int numLeds, SegmentManager *segManager, unsigned long interval)
    : Pattern(leds, numLeds, 50), segmentManager(segManager), currentRingPosition(0),
      bouncingOutward(true), bounceSpeed(1310), ringIntensity(255), lastUpdate(0)
{
  // Start at the innermost ring (EYE_0)
  // Fixed-point: 0-32768 represents 0.0-5.0 rings (6554 per ring)
  // bounceSpeed: 0.02 * 65535 ≈ 1310
  currentRingPosition = 0;
}

bool RipplePattern::update(unsigned long currentTime)
{
  if (!isActive || (currentTime - lastUpdate) < updateInterval)
  {
    return false;
  }

  lastUpdate = currentTime;

  // Clear all LEDs
  fill_solid(leds, numLeds, CRGB::Black);

  if (!segmentManager || !currentPalette)
  {
    return true;
  }

  // Calculate adjusted speed (fixed-point)
  uint16_t adjustedSpeed = (uint16_t)(bounceSpeed * speed);

  if (bouncingOutward)
  {
    if (currentRingPosition <= (32768 - adjustedSpeed))
    {
      currentRingPosition += adjustedSpeed;
    }
    else
    {
      currentRingPosition = 32768; // Max = 5.0 rings (32768 = 5 * 6554)
      bouncingOutward = false;     // Start bouncing inward
    }
  }
  else
  {
    if (currentRingPosition >= adjustedSpeed)
    {
      currentRingPosition -= adjustedSpeed;
    }
    else
    {
      currentRingPosition = 0;
      bouncingOutward = true; // Start bouncing outward
    }
  }

  // Determine which eye ring to light up (integer division)
  // activeRing: 0 = EYE_0 (center), 1 = EYE_1, 2 = EYE_2, 3 = EYE_3, 4 = EYE_4
  uint8_t activeRing = currentRingPosition / 6554; // 6554 = 32768 / 5 rings

  // Get color from palette based on ring position
  // Convert fixed-point to palette index (0-255)
  uint8_t colorPos = (currentRingPosition * 255) / 32768; // Normalize to 0-255
  CRGB ringColor = currentPalette->getColor(colorPos);
  ringColor.nscale8(brightness);

  // Light up the active eye ring
  // Map activeRing (0-4) to segment types (SEGMENT_EYE_0 to SEGMENT_EYE_4)
  // activeRing 0 → SEGMENT_EYE_0 (5), activeRing 4 → SEGMENT_EYE_4 (1)
  uint8_t segmentType = SEGMENT_EYE_0 - activeRing;

  if (segmentType >= SEGMENT_EYE_4 && segmentType <= SEGMENT_EYE_0)
  {
    segmentManager->fillSegment(leds, segmentType, ringColor);
  }

  return true;
}

void RipplePattern::setBounceSpeed(float speed)
{
  // Convert float speed to fixed-point (0.01-0.1 → 655-6554)
  bounceSpeed = (uint16_t)constrain(speed * 65535, 655, 6554);
}

// ============================================================================
// Eye Breathing Pattern Implementation
// ============================================================================

EyeBreathingPattern::EyeBreathingPattern(CRGB *leds, int numLeds, SegmentManager *segManager)
    : Pattern(leds, numLeds, 30), segmentManager(segManager), breathPhase(0.0f),
      currentEyeRing(0), breathingIn(true)
{
}

bool EyeBreathingPattern::update(unsigned long currentTime)
{
  if (!isActive || (currentTime - lastUpdate) < updateInterval)
  {
    return false;
  }

  lastUpdate = currentTime;

  // Clear all LEDs
  fill_solid(leds, numLeds, CRGB::Black);

  if (!segmentManager || !currentPalette)
  {
    return true;
  }

  // Update breathing phase
  breathPhase += speed * 0.05f;

  if (breathPhase >= 1.0f)
  {
    breathPhase = 0.0f;
    if (breathingIn)
    {
      // Move to next ring
      currentEyeRing++;
      if (currentEyeRing >= NUM_EYE_RINGS)
      {
        currentEyeRing = NUM_EYE_RINGS - 1;
        breathingIn = false;
      }
    }
    else
    {
      // Move to previous ring
      if (currentEyeRing == 0)
      {
        breathingIn = true;
      }
      else
      {
        currentEyeRing--;
      }
    }
  }

  // Calculate breathing intensity using sine wave
  float intensity = (sin(breathPhase * 2 * PI) + 1.0f) / 2.0f;
  uint8_t brightness = (uint8_t)(intensity * 255);

  // Get color from palette
  float colorPos = (float)currentEyeRing / (float)NUM_EYE_RINGS;
  CRGB color = currentPalette->getColorSmooth(colorPos);
  color.nscale8(brightness);

  // Light up current eye ring and fade adjacent rings
  for (uint8_t ring = 0; ring < NUM_EYE_RINGS; ring++)
  {
    if (ring == currentEyeRing)
    {
      segmentManager->fillSegment(leds, ring, color);
    }
    else if (abs((int)ring - (int)currentEyeRing) == 1)
    {
      CRGB fadedColor = color;
      fadedColor.nscale8(100); // 40% brightness for adjacent rings
      segmentManager->fillSegment(leds, ring, fadedColor);
    }
  }

  return true;
}

// ============================================================================
// Segment Test Pattern Implementation
// ============================================================================

SegmentTestPattern::SegmentTestPattern(CRGB *leds, int numLeds, SegmentManager *segManager, unsigned long interval)
    : Pattern(leds, numLeds, 50), segmentManager(segManager), currentSegment(0),
      lastSegmentChange(0), segmentInterval(interval)
{
}

bool SegmentTestPattern::update(unsigned long currentTime)
{
  if (!isActive || (currentTime - lastUpdate) < updateInterval)
  {
    return false;
  }

  lastUpdate = currentTime;

  // Clear all LEDs
  fill_solid(leds, numLeds, CRGB::Black);

  if (!segmentManager)
  {
    return true;
  }

  // Change segment every segmentInterval milliseconds
  if (currentTime - lastSegmentChange > segmentInterval / speed)
  {
    currentSegment++;
    if (currentSegment >= segmentManager->getSegmentCount())
    {
      currentSegment = 0;
    }
    lastSegmentChange = currentTime;

#if ENABLE_SEGMENT_DEBUG
    const char *segmentName = segmentManager->getSegmentName(currentSegment);
    Serial.print("Testing segment: ");
    Serial.print(currentSegment);
    Serial.print(" (");
    Serial.print(segmentName);
    Serial.println(")");
#endif
  }

  // Light up current segment in a distinctive color
  CRGB testColors[] = {
      CRGB::Red,    // EYE_4 - Outermost ring
      CRGB::Orange, // EYE_3
      CRGB::Yellow, // EYE_2
      CRGB::Green,  // EYE_1
      CRGB::Blue,   // EYE_0 - Center LED
      CRGB::Purple  // CLOCK - Outer ring
  };

  CRGB segmentColor = testColors[currentSegment % 6];
  segmentManager->fillSegment(leds, currentSegment, segmentColor);

  return true;
}

void SegmentTestPattern::setSegmentInterval(unsigned long interval)
{
  segmentInterval = constrain(interval, 500, 10000);
}

// ============================================================================
// POLE PATTERN IMPLEMENTATIONS
// ============================================================================

// Base PolePattern class implementation
PolePattern::PolePattern(CRGB *leds, int numLeds, CRGB *poleLeds, int poleNumLeds, unsigned long updateInterval)
    : Pattern(leds, numLeds, updateInterval), poleLeds(poleLeds), poleNumLeds(poleNumLeds), palette(nullptr)
{
}

uint8_t PolePattern::getPoleColumn(uint16_t index) const
{
  return index % POLE_SPIRAL_REPEAT;
}

uint8_t PolePattern::getPoleHeight(uint16_t index) const
{
  return index / POLE_SPIRAL_REPEAT;
}

int PolePattern::getPoleIndex(uint8_t column, uint8_t height) const
{
  if (column >= POLE_SPIRAL_REPEAT || height >= POLE_HEIGHT_LEVELS)
  {
    return -1; // Invalid position
  }

  int index = height * POLE_SPIRAL_REPEAT + column;
  return (index < poleNumLeds) ? index : -1;
}

CRGB PolePattern::getPaletteColor(float position) const
{
  if (palette)
  {
    return palette->getColorSmooth(position);
  }
  else
  {
    // Fallback to rainbow if no palette set
    uint8_t hue = (uint8_t)(position * 255.0f);
    return CHSV(hue, 255, 255);
  }
}

// PoleColumnWavePattern implementation
PoleColumnWavePattern::PoleColumnWavePattern(CRGB *leds, int numLeds, CRGB *poleLeds, int poleNumLeds)
    : PolePattern(leds, numLeds, poleLeds, poleNumLeds, 50),
      wavePosition(0.0f),
      waveWidth(3),
      reverseDirection(false)
{
}

bool PoleColumnWavePattern::update(unsigned long currentTime)
{
  if (!isActive || (currentTime - lastUpdate) < updateInterval)
  {
    return false;
  }

  // Clear pole LEDs
  fill_solid(poleLeds, poleNumLeds, CRGB::Black);

  // Update wave position
  wavePosition += speed * 0.5f;
  if (wavePosition >= POLE_SPIRAL_REPEAT)
  {
    wavePosition = 0.0f;
    reverseDirection = !reverseDirection; // Change direction at each cycle
  }

  // Create column wave effect
  for (uint8_t col = 0; col < POLE_SPIRAL_REPEAT; col++)
  {
    // Calculate distance from wave center
    float distanceFromWave = abs((float)col - wavePosition);
    if (distanceFromWave > POLE_SPIRAL_REPEAT / 2)
    {
      distanceFromWave = POLE_SPIRAL_REPEAT - distanceFromWave; // Wrap around
    }

    // Calculate wave intensity
    float intensity = 0.0f;
    if (distanceFromWave <= waveWidth)
    {
      intensity = 1.0f - (distanceFromWave / waveWidth);
    }

    if (intensity > 0.0f)
    {
      // Light up all LEDs in this column
      for (uint8_t height = 0; height < POLE_HEIGHT_LEVELS; height++)
      {
        int ledIndex = getPoleIndex(col, height);
        if (ledIndex >= 0)
        {
          // Use palette colors based on column and height position
          float palettePosition = ((float)col / POLE_SPIRAL_REPEAT + (float)height / POLE_HEIGHT_LEVELS + (currentTime / 5000.0f)) / 2.0f;
          palettePosition = fmod(palettePosition, 1.0f); // Wrap to 0.0-1.0

          CRGB rgbColor = getPaletteColor(palettePosition);
          rgbColor.nscale8((uint8_t)(intensity * brightness));

          poleLeds[ledIndex] = rgbColor;
        }
      }
    }
  }

  lastUpdate = currentTime;
  return true;
}

// PoleSpiralChasePattern implementation
PoleSpiralChasePattern::PoleSpiralChasePattern(CRGB *leds, int numLeds, CRGB *poleLeds, int poleNumLeds)
    : PolePattern(leds, numLeds, poleLeds, poleNumLeds, 30),
      chasePosition(0.0f),
      chaseLength(20),
      hueShift(0)
{
}

bool PoleSpiralChasePattern::update(unsigned long currentTime)
{
  if (!isActive || (currentTime - lastUpdate) < updateInterval)
  {
    return false;
  }

  // Clear pole LEDs
  fill_solid(poleLeds, poleNumLeds, CRGB::Black);

  // Update chase position
  chasePosition += speed * 2.0f;
  if (chasePosition >= poleNumLeds)
  {
    chasePosition = 0.0f;
  }

  // Create chase effect
  for (int i = 0; i < chaseLength; i++)
  {
    int ledIndex = (int)(chasePosition - i);
    if (ledIndex < 0)
      ledIndex += poleNumLeds; // Wrap around
    if (ledIndex >= poleNumLeds)
      ledIndex -= poleNumLeds;

    // Calculate fade based on position in chase tail
    float fade = 1.0f - ((float)i / (float)chaseLength);

    // Use palette color based on position
    float palettePosition = ((float)ledIndex / (float)poleNumLeds + hueShift / 255.0f);
    palettePosition = fmod(palettePosition, 1.0f); // Wrap to 0.0-1.0

    CRGB rgbColor = getPaletteColor(palettePosition);
    rgbColor.nscale8((uint8_t)(fade * brightness));

    poleLeds[ledIndex] = rgbColor;
  }

  // Advance hue shift for color cycling
  hueShift += 2;

  lastUpdate = currentTime;
  return true;
}

// PoleHelixPattern implementation (OPTIMIZED)
PoleHelixPattern::PoleHelixPattern(CRGB *leds, int numLeds, CRGB *poleLeds, int poleNumLeds)
    : PolePattern(leds, numLeds, poleLeds, poleNumLeds, 40),
      helixPhase(0), // Fixed-point: 0-65535 represents 0-2π
      numHelixes(3),
      helixSpeed(6554) // 0.1 * 65535 ≈ 6554 (10% speed)
{
}

bool PoleHelixPattern::update(unsigned long currentTime)
{
  if (!isActive || (currentTime - lastUpdate) < updateInterval)
  {
    return false;
  }

  // Clear pole LEDs
  fill_solid(poleLeds, poleNumLeds, CRGB::Black);

  // Update helix phase (fixed-point, auto-wraps at 65536)
  helixPhase += (uint16_t)(speed * helixSpeed);
  // No need to wrap - uint16_t overflow IS the wrap!

  // Create multiple helixes
  for (uint8_t helix = 0; helix < numHelixes; helix++)
  {
    // Helix offset in fixed-point (divide full range by numHelixes)
    uint16_t helixOffset = (65535 / numHelixes) * helix;

    for (uint16_t i = 0; i < poleNumLeds; i++)
    {
      uint8_t height = getPoleHeight(i);
      uint8_t column = getPoleColumn(i);

      // Calculate helix position (fixed-point)
      // height * 0.3 ≈ height * 328 (in 16-bit fixed-point where 1024 = 0.3 * 65535/20)
      uint16_t helixPos16 = helixPhase + helixOffset + (height * 328);

      // Use FastLED's sin16() - 10-20x faster than sin()!
      int16_t sinValue = sin16(helixPos16); // Returns -32768 to +32767

      // Map to column range: (sin + 32768) * POLE_SPIRAL_REPEAT / 65536
      uint8_t expectedColumn = ((uint32_t)(sinValue + 32768) * POLE_SPIRAL_REPEAT) >> 16;

      // Check if this LED is close to the helix path (integer math)
      int8_t columnDistance = abs((int8_t)column - (int8_t)expectedColumn);
      if (columnDistance > (int8_t)(POLE_SPIRAL_REPEAT / 2))
      {
        columnDistance = POLE_SPIRAL_REPEAT - columnDistance; // Wrap around
      }

      if (columnDistance <= 1) // Threshold of 1.5 → 1 (close enough)
      {
        // Calculate intensity based on distance (0-255 range)
        uint8_t intensity = 255 - (columnDistance * 170); // 170 = 255/1.5

        // Palette position calculation (no fmod needed - uint8_t wraps!)
        uint8_t palettePos = (helix * 85) + (height * 2) + (currentTime >> 12);
        // 85 = 255/3 (divides palette into thirds for 3 helixes)
        // >> 12 approximates / 4096 (close to / 10000)

        CRGB rgbColor = getPaletteColor(palettePos / 255.0f);

        // Use scale8() for fast brightness scaling
        rgbColor.nscale8(scale8(intensity, brightness));

        poleLeds[i] = rgbColor;
      }
    }
  }

  lastUpdate = currentTime;
  return true;
}

// PoleFirePattern implementation
PoleFirePattern::PoleFirePattern(CRGB *leds, int numLeds, CRGB *poleLeds, int poleNumLeds)
    : PolePattern(leds, numLeds, poleLeds, poleNumLeds, 30),
      cooling(55),
      sparking(120)
{
  // Initialize heat array
  for (int i = 0; i < POLE_NUM_LEDS; i++)
  {
    heat[i] = 0;
  }
}

bool PoleFirePattern::update(unsigned long currentTime)
{
  if (!isActive || (currentTime - lastUpdate) < updateInterval)
  {
    return false;
  }

  // Step 1: Cool down every cell a little
  for (int i = 0; i < poleNumLeds; i++)
  {
    heat[i] = qsub8(heat[i], random8(0, ((cooling * 10) / poleNumLeds) + 2));
  }

  // Step 2: Heat from each cell drifts 'up' and diffuses a little
  for (int k = poleNumLeds - 1; k >= 2; k--)
  {
    heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2]) / 3;
  }

  // Step 3: Randomly ignite new 'sparks' at the bottom
  if (random8() < sparking)
  {
    int y = random8(7);
    heat[y] = qadd8(heat[y], random8(160, 255));
  }

  // Step 4: Map heat to LED colors using palette
  for (int j = 0; j < poleNumLeds; j++)
  {
    // Map heat to palette position (0.0 = cool, 1.0 = hot)
    float palettePosition = (float)heat[j] / 255.0f;
    CRGB color = getPaletteColor(palettePosition);

    // Scale brightness
    color.nscale8(brightness);

    poleLeds[j] = color;
  }

  lastUpdate = currentTime;
  return true;
}

// PoleBouncePattern implementation (OPTIMIZED)
PoleBouncePattern::PoleBouncePattern(CRGB *leds, int numLeds, CRGB *poleLeds, int poleNumLeds)
    : PolePattern(leds, numLeds, poleLeds, poleNumLeds, 40),
      wave1Position(0),      // Fixed-point: 0 = start
      wave2Position(32768),  // Fixed-point: 32768 = middle (0.5 * 65535)
      wave1Direction(true),  // Wave 1 starts going up
      wave2Direction(false), // Wave 2 starts going down
      waveLength(30),
      hueOffset(128),     // 180 degrees apart in hue
      waveSpeed(655),     // 0.01 * 65535 ≈ 655
      ledPositionStep(0), // Will be calculated
      waveLengthFixed(0)  // Will be calculated
{
  // Pre-compute constants (one-time calculation)
  if (poleNumLeds > 1)
  {
    ledPositionStep = 65535 / (poleNumLeds - 1); // ONE division for entire pattern lifetime!
    waveLengthFixed = ((uint32_t)waveLength * 65535) / poleNumLeds;
  }
}

bool PoleBouncePattern::update(unsigned long currentTime)
{
  if (!isActive || (currentTime - lastUpdate) < updateInterval)
  {
    return false;
  }

  // Clear pole LEDs
  fill_solid(poleLeds, poleNumLeds, CRGB::Black);

  // Calculate wave speed (fixed-point)
  uint16_t currentWaveSpeed = (uint16_t)(speed * waveSpeed);

  // Update wave 1 (integer math)
  if (wave1Direction) // Going up
  {
    if (wave1Position <= (65535 - currentWaveSpeed))
    {
      wave1Position += currentWaveSpeed;
    }
    else
    {
      wave1Position = 65535;
      wave1Direction = false; // Bounce down
    }
  }
  else // Going down
  {
    if (wave1Position >= currentWaveSpeed)
    {
      wave1Position -= currentWaveSpeed;
    }
    else
    {
      wave1Position = 0;
      wave1Direction = true; // Bounce up
    }
  }

  // Update wave 2 (opposite direction)
  if (wave2Direction) // Going up
  {
    if (wave2Position <= (65535 - currentWaveSpeed))
    {
      wave2Position += currentWaveSpeed;
    }
    else
    {
      wave2Position = 65535;
      wave2Direction = false; // Bounce down
    }
  }
  else // Going down
  {
    if (wave2Position >= currentWaveSpeed)
    {
      wave2Position -= currentWaveSpeed;
    }
    else
    {
      wave2Position = 0;
      wave2Direction = true; // Bounce up
    }
  }

  // Render both waves (OPTIMIZED - no float math, no divisions!)
  uint16_t ledPosition = 0;
  uint8_t timeOffset = currentTime >> 12; // Approximates / 4096 (close to / 5000)

  for (int i = 0; i < poleNumLeds; i++)
  {
    CRGB finalColor = CRGB::Black;

    // Calculate wave 1 contribution (integer math)
    int32_t wave1Distance = abs((int32_t)ledPosition - (int32_t)wave1Position);
    if (wave1Distance <= waveLengthFixed)
    {
      // Integer intensity calculation (0-255)
      uint8_t wave1Intensity = 255 - ((wave1Distance * 255) / waveLengthFixed);

      // Palette position (wraps automatically with uint8_t - no fmod!)
      uint8_t wave1PalettePos = (ledPosition >> 8) + timeOffset;

      CRGB wave1Color = getPaletteColor(wave1PalettePos / 255.0f);
      wave1Color.nscale8(scale8(wave1Intensity, brightness));
      finalColor += wave1Color;
    }

    // Calculate wave 2 contribution (same optimization)
    int32_t wave2Distance = abs((int32_t)ledPosition - (int32_t)wave2Position);
    if (wave2Distance <= waveLengthFixed)
    {
      uint8_t wave2Intensity = 255 - ((wave2Distance * 255) / waveLengthFixed);
      uint8_t wave2PalettePos = (ledPosition >> 8) + hueOffset + timeOffset;

      CRGB wave2Color = getPaletteColor(wave2PalettePos / 255.0f);
      wave2Color.nscale8(scale8(wave2Intensity, brightness));
      finalColor += wave2Color;
    }

    poleLeds[i] = finalColor;
    ledPosition += ledPositionStep; // Integer increment - 30x faster than division!
  }

  lastUpdate = currentTime;
  return true;
}

// ============================================================================
// ACTION PATTERN IMPLEMENTATION
// ============================================================================

ActionPattern::ActionPattern(CRGB *leds, int numLeds, CRGB *poleLeds, int poleNumLeds, unsigned long updateInterval)
    : leds(leds), numLeds(numLeds), poleLeds(poleLeds), poleNumLeds(poleNumLeds),
      startTime(0), lastUpdate(0), updateInterval(updateInterval),
      isActive(false), isComplete(false), brightness(255), speed(1.0f)
{
}

void ActionPattern::trigger(unsigned long currentTime)
{
  startTime = currentTime;
  lastUpdate = currentTime;
  isActive = true;
  isComplete = false;
}

// ============================================================================
// FIREWORK ACTION IMPLEMENTATION
// ============================================================================

FireworkAction::FireworkAction(CRGB *leds, int numLeds, CRGB *poleLeds, int poleNumLeds)
    : ActionPattern(leds, numLeds, poleLeds, poleNumLeds, 16), // 60+ FPS for smooth animation
      currentPhase(PHASE_LAUNCH),
      phaseStartTime(0),
      launchPosition(0.0f),
      explosionRadius(0.0f),
      explosionHue(0),
      fadeIntensity(1.0f)
{
}

void FireworkAction::trigger(unsigned long currentTime)
{
  ActionPattern::trigger(currentTime);
  currentPhase = PHASE_LAUNCH;
  phaseStartTime = currentTime;
  launchPosition = 0.0f;
  explosionRadius = 0.0f;
  explosionHue = random8();
  fadeIntensity = 1.0f;
}

bool FireworkAction::update(unsigned long currentTime)
{
  if (!isActive || (currentTime - lastUpdate) < updateInterval)
  {
    return false;
  }

  unsigned long phaseElapsed = currentTime - phaseStartTime;

  // Update current phase
  switch (currentPhase)
  {
  case PHASE_LAUNCH:
    updateLaunchPhase(currentTime);
    if (phaseElapsed >= LAUNCH_DURATION)
    {
      currentPhase = PHASE_EXPLOSION;
      phaseStartTime = currentTime;
      explosionRadius = 0.0f;
    }
    break;

  case PHASE_EXPLOSION:
    updateExplosionPhase(currentTime);
    if (phaseElapsed >= EXPLOSION_DURATION)
    {
      currentPhase = PHASE_FADEOUT;
      phaseStartTime = currentTime;
      fadeIntensity = 1.0f; // Start at full intensity
    }
    break;

  case PHASE_FADEOUT:
    updateFadeoutPhase(currentTime);
    if (phaseElapsed >= FADEOUT_DURATION)
    {
      // Action is complete
      isActive = false;
      isComplete = true;
      return false;
    }
    break;
  }

  lastUpdate = currentTime;
  return true;
}

void FireworkAction::updateLaunchPhase(unsigned long currentTime)
{
  // Update launch position (0.0 = bottom, 1.0 = top of pole)
  unsigned long phaseElapsed = currentTime - phaseStartTime;
  launchPosition = (float)phaseElapsed / (float)LAUNCH_DURATION;

  // Use actual pole LEDs if available
  if (poleLeds && poleNumLeds > 0)
  {
    // Clear pole LEDs
    fill_solid(poleLeds, poleNumLeds, CRGB::Black);

    // Create white trail moving up the pole
    int trailCenter = (int)(launchPosition * poleNumLeds);
    int trailLength = 12; // Length of the trail

    for (int i = 0; i < trailLength; i++)
    {
      int ledIndex = trailCenter - i;
      if (ledIndex >= 0 && ledIndex < poleNumLeds)
      {
        // Brighter at head, dimmer at tail
        uint8_t intensity = 255 - (i * 20);
        poleLeds[ledIndex] = CRGB(intensity, intensity, intensity); // White trail
      }
    }
  }
  else
  {
    // Fallback: simulate on clock ring
    int clockStart = CLOCK_RAW_START;
    int clockCount = CLOCK_COUNT;

    // Map launch position to clock LEDs (bottom to top simulation)
    int trailCenter = (int)(launchPosition * clockCount);
    int trailLength = 8; // Length of the trail

    for (int i = 0; i < trailLength; i++)
    {
      int ledIndex = trailCenter - i;
      if (ledIndex >= 0 && ledIndex < clockCount)
      {
        int absoluteIndex = clockStart + ledIndex;
        if (absoluteIndex < numLeds)
        {
          // Brighter at head, dimmer at tail
          uint8_t intensity = 255 - (i * 30);
          leds[absoluteIndex] = CRGB(intensity, intensity, intensity); // White trail
        }
      }
    }
  }
}

void FireworkAction::updateExplosionPhase(unsigned long currentTime)
{
  unsigned long phaseElapsed = currentTime - phaseStartTime;
  float progress = (float)phaseElapsed / (float)EXPLOSION_DURATION;

  // Clear main LEDs first to ensure explosion is visible
  if (leds && numLeds > 0)
  {
    fill_solid(leds, numLeds, CRGB::Black);
  }

  // Explosion radiates from center (EYE_0) outward
  explosionRadius = progress * 6.0f; // 6 rings total (5 eye + 1 clock)

  // Rainbow cascade effect
  explosionHue = (explosionHue + 2) % 255; // Slowly shift hue

  // Light up rings based on explosion radius
  for (int ring = 0; ring < 6; ring++)
  {
    float ringDistance = (float)ring;

    // Check if this ring should be lit
    if (explosionRadius >= ringDistance)
    {
      // Calculate intensity based on how recently this ring was hit
      float ringAge = explosionRadius - ringDistance;
      float intensity = 1.0f - (ringAge / 3.0f); // Fade over 3 ring distances (slower fade)
      intensity = max(0.0f, min(1.0f, intensity));

      if (intensity > 0.0f)
      {
        // Calculate ring color (rainbow effect)
        uint8_t ringHue = (explosionHue + ring * 40) % 255;
        // Ensure minimum brightness for visibility, scale with intensity
        uint8_t ringBrightness = (uint8_t)(max(100.0f, intensity * 255.0f)); // Minimum 100 brightness

        CHSV hsvColor(ringHue, 255, ringBrightness);
        CRGB rgbColor;
        hsv2rgb_rainbow(hsvColor, rgbColor);

        // Light up the appropriate ring
        if (ring < 5)
        {
          // Eye rings (EYE_0 to EYE_4) - center to outer
          int segmentType = SEGMENT_EYE_0 - ring; // Subtract to go from center (4) outward to (0)
          int rawStartLED, count;

          switch (segmentType)
          {
          case SEGMENT_EYE_0: // ring 0 - center
            rawStartLED = EYE_0_RAW_START;
            count = EYE_0_COUNT;
            break;
          case SEGMENT_EYE_1: // ring 1
            rawStartLED = EYE_1_RAW_START;
            count = EYE_1_COUNT;
            break;
          case SEGMENT_EYE_2: // ring 2
            rawStartLED = EYE_2_RAW_START;
            count = EYE_2_COUNT;
            break;
          case SEGMENT_EYE_3: // ring 3
            rawStartLED = EYE_3_RAW_START;
            count = EYE_3_COUNT;
            break;
          case SEGMENT_EYE_4: // ring 4 - outermost
            rawStartLED = EYE_4_RAW_START;
            count = EYE_4_COUNT;
            break;
          default:
            continue;
          }

          for (int i = 0; i < count; i++)
          {
            int ledIndex = rawStartLED + i;
            if (ledIndex < numLeds)
            {
              leds[ledIndex] = rgbColor;
            }
          }
        }
        else
        {
          // Clock ring
          for (int i = 0; i < CLOCK_COUNT; i++)
          {
            int ledIndex = CLOCK_RAW_START + i;
            if (ledIndex < numLeds)
            {
              leds[ledIndex] = rgbColor;
            }
          }
        }
      }
    }
  }
}

void FireworkAction::updateFadeoutPhase(unsigned long currentTime)
{
  unsigned long phaseElapsed = currentTime - phaseStartTime;

  // Calculate fade progress (1.0 to 0.0 over FADEOUT_DURATION)
  float fadeProgress = 1.0f - ((float)phaseElapsed / (float)FADEOUT_DURATION);
  fadeProgress = max(0.0f, min(1.0f, fadeProgress));

  // Clear main LEDs first
  if (leds && numLeds > 0)
  {
    fill_solid(leds, numLeds, CRGB::Black);
  }

  // Render fading explosion rings
  for (int ring = 0; ring < 6; ring++)
  {
    // Calculate ring color (same as explosion but fading)
    uint8_t ringHue = (explosionHue + ring * 40) % 255;
    uint8_t ringBrightness = (uint8_t)(fadeProgress * brightness);

    if (ringBrightness > 0)
    {
      CHSV hsvColor(ringHue, 255, ringBrightness);
      CRGB rgbColor;
      hsv2rgb_rainbow(hsvColor, rgbColor);

      // Light up the appropriate ring
      if (ring < 5)
      {
        // Eye rings (EYE_0 to EYE_4) - center to outer
        int segmentType = SEGMENT_EYE_0 - ring;
        int rawStartLED, count;

        switch (segmentType)
        {
        case SEGMENT_EYE_0:
          rawStartLED = EYE_0_RAW_START;
          count = EYE_0_COUNT;
          break;
        case SEGMENT_EYE_1:
          rawStartLED = EYE_1_RAW_START;
          count = EYE_1_COUNT;
          break;
        case SEGMENT_EYE_2:
          rawStartLED = EYE_2_RAW_START;
          count = EYE_2_COUNT;
          break;
        case SEGMENT_EYE_3:
          rawStartLED = EYE_3_RAW_START;
          count = EYE_3_COUNT;
          break;
        case SEGMENT_EYE_4:
          rawStartLED = EYE_4_RAW_START;
          count = EYE_4_COUNT;
          break;
        default:
          continue;
        }

        for (int i = 0; i < count; i++)
        {
          int ledIndex = rawStartLED + i;
          if (ledIndex < numLeds)
          {
            leds[ledIndex] = rgbColor;
          }
        }
      }
      else
      {
        // Clock ring
        for (int i = 0; i < CLOCK_COUNT; i++)
        {
          int ledIndex = CLOCK_RAW_START + i;
          if (ledIndex < numLeds)
          {
            leds[ledIndex] = rgbColor;
          }
        }
      }
    }
  }
}
