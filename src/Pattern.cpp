#include "Pattern.h"
#include "ColorPalette.h"

// ============================================================================
// Base Pattern Class Implementation
// ============================================================================

Pattern::Pattern(CRGB *leds, int numLeds, unsigned long updateInterval)
    : leds(leds), numLeds(numLeds), updateInterval(updateInterval),
      lastUpdate(0), isActive(true), currentPalette(nullptr),
      brightness(255), speed(1.0f)
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
  this->speed = constrain(speed, 0.1f, 10.0f);
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
