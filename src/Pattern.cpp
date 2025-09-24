#include "Pattern.h"
#include "ColorPalette.h"

// ============================================================================
// Base Pattern Class Implementation
// ============================================================================

Pattern::Pattern(CRGB *leds, int numLeds, unsigned long updateInterval)
    : leds(leds), numLeds(numLeds), updateInterval(updateInterval),
      lastUpdate(0), isActive(true), currentPalette(nullptr),
      brightness(255), speed(DEFAULT_GLOBAL_SPEED)
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

  fill_rainbow(leds, numLeds, hue, deltaHue);

  // Apply brightness scaling
  for (int i = 0; i < numLeds; i++)
  {
    leds[i].nscale8(brightness);
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

  // Draw the chase dot and trail
  for (int i = 0; i < trailLength; i++)
  {
    int ledIndex = position - i;
    if (ledIndex < 0)
      ledIndex += numLeds;
    if (ledIndex >= numLeds)
      ledIndex -= numLeds;

    // Get color from palette based on position, or use default color
    CRGB trailColor;
    if (currentPalette)
    {
      // Sample palette based on position for color variation
      uint8_t paletteIndex = map(ledIndex, 0, numLeds - 1, 0, 255);
      trailColor = currentPalette->getColor(paletteIndex);
    }
    else
    {
      trailColor = chaseColor; // Fallback to hardcoded color
    }

    uint8_t brightness_scale = map(i, 0, trailLength - 1, brightness, brightness / 4);
    trailColor.nscale8(brightness_scale);

    leds[ledIndex] = trailColor;
  }

  // Move position
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

  // Draw sine wave
  for (int i = 0; i < numLeds; i++)
  {
    float angle = (float)(i + wavePosition) * 2.0 * PI / waveLength;
    uint8_t intensity = (uint8_t)((sin(angle) + 1.0) * 127.5);

    // Get color from palette based on position, or use default color
    CRGB scaledColor;
    if (currentPalette)
    {
      // Sample palette based on position for color variation
      uint8_t paletteIndex = map(i, 0, numLeds - 1, 0, 255);
      scaledColor = currentPalette->getColor(paletteIndex);
    }
    else
    {
      scaledColor = waveColor; // Fallback to hardcoded color
    }

    scaledColor.nscale8(map(intensity, 0, 255, 0, brightness));
    leds[i] = scaledColor;
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
    : Pattern(leds, numLeds, 50), segmentManager(segManager), lastRippleTime(0), rippleInterval(interval)
{

  // Initialize ripples
  for (uint8_t i = 0; i < MAX_RIPPLES; i++)
  {
    ripples[i].radius = 0.0f;
    ripples[i].intensity = 0;
    ripples[i].active = false;
  }
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

  // Start new ripple if it's time
  if (currentTime - lastRippleTime > rippleInterval / speed)
  {
    for (uint8_t i = 0; i < MAX_RIPPLES; i++)
    {
      if (!ripples[i].active)
      {
        ripples[i].radius = 0.0f;
        ripples[i].intensity = 255;
        ripples[i].active = true;
        lastRippleTime = currentTime;
        break;
      }
    }
  }

  // Update and draw ripples
  for (uint8_t i = 0; i < MAX_RIPPLES; i++)
  {
    if (ripples[i].active)
    {
      // Expand ripple
      ripples[i].radius += speed * 0.05f;
      ripples[i].intensity = max(0, (int)ripples[i].intensity - (int)(speed * 8));

      // Deactivate if fully faded
      if (ripples[i].intensity <= 0 || ripples[i].radius > NUM_EYE_RINGS)
      {
        ripples[i].active = false;
        continue;
      }

      // Draw ripple on appropriate rings
      uint8_t ringIndex = (uint8_t)ripples[i].radius;
      if (ringIndex < segmentManager->getSegmentCount())
      {
        float colorPos = (float)i / (float)MAX_RIPPLES;
        CRGB color = currentPalette->getColorSmooth(colorPos);
        color.nscale8(ripples[i].intensity);

        // Fill the ring with ripple color
        segmentManager->fillSegment(leds, ringIndex, color);
      }
    }
  }

  return true;
}

void RipplePattern::setRippleInterval(unsigned long interval)
{
  rippleInterval = constrain(interval, 200, 5000);
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
    : Pattern(leds, numLeds, updateInterval), poleLeds(poleLeds), poleNumLeds(poleNumLeds)
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
          // Create rainbow colors based on column and height
          uint8_t hue = (col * 20 + height * 5 + (uint8_t)(currentTime / 50)) % 255;
          uint8_t sat = 255;
          uint8_t val = (uint8_t)(intensity * brightness);

          CHSV hsvColor(hue, sat, val);
          CRGB rgbColor;
          hsv2rgb_rainbow(hsvColor, rgbColor);

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

    // Calculate hue based on position and time
    uint8_t hue = (hueShift + ledIndex * 2) % 255;
    uint8_t sat = 255;
    uint8_t val = (uint8_t)(fade * brightness);

    CHSV hsvColor(hue, sat, val);
    CRGB rgbColor;
    hsv2rgb_rainbow(hsvColor, rgbColor);

    poleLeds[ledIndex] = rgbColor;
  }

  // Advance hue shift for color cycling
  hueShift += 2;

  lastUpdate = currentTime;
  return true;
}

// PoleHelixPattern implementation
PoleHelixPattern::PoleHelixPattern(CRGB *leds, int numLeds, CRGB *poleLeds, int poleNumLeds)
    : PolePattern(leds, numLeds, poleLeds, poleNumLeds, 40),
      helixPhase(0.0f),
      numHelixes(3),
      helixSpeed(0.1f)
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

  // Update helix phase
  helixPhase += speed * helixSpeed;
  if (helixPhase >= 2.0f * PI)
  {
    helixPhase -= 2.0f * PI;
  }

  // Create multiple helixes
  for (uint8_t helix = 0; helix < numHelixes; helix++)
  {
    float helixOffset = (2.0f * PI * helix) / numHelixes;

    for (uint16_t i = 0; i < poleNumLeds; i++)
    {
      uint8_t height = getPoleHeight(i);
      uint8_t column = getPoleColumn(i);

      // Calculate helix position
      float helixPos = helixPhase + helixOffset + (height * 0.3f);
      float expectedColumn = (sin(helixPos) + 1.0f) * (POLE_SPIRAL_REPEAT / 2.0f);

      // Check if this LED is close to the helix path
      float columnDistance = abs(column - expectedColumn);
      if (columnDistance > POLE_SPIRAL_REPEAT / 2)
      {
        columnDistance = POLE_SPIRAL_REPEAT - columnDistance; // Wrap around
      }

      if (columnDistance <= 1.5f)
      {
        // Calculate intensity based on distance from helix center
        float intensity = 1.0f - (columnDistance / 1.5f);

        // Calculate color based on helix and height
        uint8_t hue = (helix * 85 + height * 5 + (uint8_t)(currentTime / 100)) % 255;
        uint8_t sat = 255;
        uint8_t val = (uint8_t)(intensity * brightness);

        CHSV hsvColor(hue, sat, val);
        CRGB rgbColor;
        hsv2rgb_rainbow(hsvColor, rgbColor);

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

  // Step 4: Map heat to LED colors
  for (int j = 0; j < poleNumLeds; j++)
  {
    CRGB color = HeatColor(heat[j]);

    // Scale brightness
    color.nscale8(brightness);

    poleLeds[j] = color;
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
      explosionHue(0)
{
  initializeSparkles();
}

void FireworkAction::trigger(unsigned long currentTime)
{
  ActionPattern::trigger(currentTime);
  currentPhase = PHASE_LAUNCH;
  phaseStartTime = currentTime;
  launchPosition = 0.0f;
  explosionRadius = 0.0f;
  explosionHue = random8();
  initializeSparkles();
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
      currentPhase = PHASE_SPARKLES;
      phaseStartTime = currentTime;
      addRandomSparkles(MAX_SPARKLES); // Fill with initial sparkles
    }
    break;

  case PHASE_SPARKLES:
    updateSparklePhase(currentTime);
    if (phaseElapsed >= SPARKLE_DURATION)
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
    int clockStart = CLOCK_START;
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
      float intensity = 1.0f - (ringAge / 2.0f); // Fade over 2 ring distances
      intensity = max(0.0f, min(1.0f, intensity));

      if (intensity > 0.0f)
      {
        // Calculate ring color (rainbow effect)
        uint8_t ringHue = (explosionHue + ring * 40) % 255;
        uint8_t ringBrightness = (uint8_t)(intensity * brightness);

        CHSV hsvColor(ringHue, 255, ringBrightness);
        CRGB rgbColor;
        hsv2rgb_rainbow(hsvColor, rgbColor);

        // Light up the appropriate ring
        if (ring < 5)
        {
          // Eye rings (EYE_4 to EYE_0)
          int segmentType = SEGMENT_EYE_4 + ring;
          int startLED, count;

          switch (segmentType)
          {
          case SEGMENT_EYE_4:
            startLED = EYE_4_START;
            count = EYE_4_COUNT;
            break;
          case SEGMENT_EYE_3:
            startLED = EYE_3_START;
            count = EYE_3_COUNT;
            break;
          case SEGMENT_EYE_2:
            startLED = EYE_2_START;
            count = EYE_2_COUNT;
            break;
          case SEGMENT_EYE_1:
            startLED = EYE_1_START;
            count = EYE_1_COUNT;
            break;
          case SEGMENT_EYE_0:
            startLED = EYE_0_START;
            count = EYE_0_COUNT;
            break;
          default:
            continue;
          }

          for (int i = 0; i < count; i++)
          {
            int ledIndex = startLED + i;
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
            int ledIndex = CLOCK_START + i;
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

void FireworkAction::updateSparklePhase(unsigned long currentTime)
{
  // Update existing sparkles
  for (int i = 0; i < MAX_SPARKLES; i++)
  {
    if (sparkles[i].active)
    {
      // Fade sparkle
      if (sparkles[i].intensity > sparkles[i].decayRate)
      {
        sparkles[i].intensity -= sparkles[i].decayRate;
      }
      else
      {
        sparkles[i].active = false;
        continue;
      }

      // Set LED color
      if (sparkles[i].ledIndex < numLeds)
      {
        CHSV hsvColor(sparkles[i].hue, 200, sparkles[i].intensity);
        CRGB rgbColor;
        hsv2rgb_rainbow(hsvColor, rgbColor);
        leds[sparkles[i].ledIndex] = rgbColor;
      }
    }
  }

  // Occasionally add new sparkles
  if (random8() < 30)
  { // 30/255 chance each frame
    addRandomSparkles(1);
  }
}

void FireworkAction::initializeSparkles()
{
  for (int i = 0; i < MAX_SPARKLES; i++)
  {
    sparkles[i].active = false;
    sparkles[i].intensity = 0;
    sparkles[i].ledIndex = 0;
    sparkles[i].hue = 0;
    sparkles[i].decayRate = 1;
  }
}

void FireworkAction::addRandomSparkles(int count)
{
  for (int i = 0; i < count; i++)
  {
    // Find inactive sparkle slot
    for (int j = 0; j < MAX_SPARKLES; j++)
    {
      if (!sparkles[j].active)
      {
        sparkles[j].active = true;
        sparkles[j].ledIndex = getRandomEyeOrClockLED();
        sparkles[j].intensity = random8(150, 255); // Bright sparkles
        sparkles[j].hue = random8();               // Random color
        sparkles[j].decayRate = random8(1, 4);     // Random fade rate
        break;
      }
    }
  }
}

float FireworkAction::getDistanceFromCenter(uint16_t ledIndex)
{
  // Calculate distance from center (EYE_0) for explosion effect
  if (ledIndex >= EYE_0_START && ledIndex < EYE_0_START + EYE_0_COUNT)
  {
    return 0.0f; // Center
  }
  else if (ledIndex >= EYE_1_START && ledIndex < EYE_1_START + EYE_1_COUNT)
  {
    return 1.0f;
  }
  else if (ledIndex >= EYE_2_START && ledIndex < EYE_2_START + EYE_2_COUNT)
  {
    return 2.0f;
  }
  else if (ledIndex >= EYE_3_START && ledIndex < EYE_3_START + EYE_3_COUNT)
  {
    return 3.0f;
  }
  else if (ledIndex >= EYE_4_START && ledIndex < EYE_4_START + EYE_4_COUNT)
  {
    return 4.0f;
  }
  else if (ledIndex >= CLOCK_START && ledIndex < CLOCK_START + CLOCK_COUNT)
  {
    return 5.0f; // Clock ring (outermost)
  }
  return 6.0f; // Unknown/outside
}

uint16_t FireworkAction::getRandomEyeOrClockLED()
{
  // Randomly select from eye rings or clock ring
  int ringChoice = random8(6); // 0-5 for the 6 rings

  switch (ringChoice)
  {
  case 0:
    return EYE_0_START; // Center (only 1 LED)
  case 1:
    return EYE_1_START + random8(EYE_1_COUNT);
  case 2:
    return EYE_2_START + random8(EYE_2_COUNT);
  case 3:
    return EYE_3_START + random8(EYE_3_COUNT);
  case 4:
    return EYE_4_START + random8(EYE_4_COUNT);
  case 5:
    return CLOCK_START + random8(CLOCK_COUNT);
  default:
    return EYE_0_START;
  }
}
