#include "WLEDPatterns.h"

// ============================================================================
// WLED Dancing Shadows Pattern
// ============================================================================

WLEDDancingShadowsPattern::WLEDDancingShadowsPattern(CRGB *leds, int numLeds)
    : Pattern(leds, numLeds, 30), counter(0)
{
  speedNormalizationFactor = 15.0f; // WLED patterns need higher speed multiplier
}

bool WLEDDancingShadowsPattern::update(unsigned long currentTime)
{
  if (!isActive || (currentTime - lastUpdate) < updateInterval)
  {
    return false;
  }
  lastUpdate = currentTime;

  // Create dancing shadows effect using sine waves
  counter += (uint16_t)(getEffectiveSpeed() * 10);

  for (int i = 0; i < numLeds; i++)
  {
    // Multiple sine waves create shadow patterns
    uint16_t wave1 = sin16((i * 70) + counter) / 256;
    uint16_t wave2 = sin16((i * 40) + (counter / 2)) / 256;
    uint16_t wave3 = sin16((i * 25) + (counter / 3)) / 256;

    // Combine waves
    uint8_t brightness = (wave1 + wave2 + wave3) / 3;

    // Get color from palette
    uint8_t paletteIndex = (i * 255 / numLeds) + (counter / 256);
    CRGB color = currentPalette ? currentPalette->getColor(paletteIndex) : CRGB::White;

    // Apply brightness from wave
    leds[i] = color;
    leds[i].nscale8(brightness);
  }

  return true;
}

// ============================================================================
// WLED Color Waves Pattern
// ============================================================================

WLEDColorWavesPattern::WLEDColorWavesPattern(CRGB *leds, int numLeds)
    : Pattern(leds, numLeds, 20), counter(0), waveSpeed(128)
{
  speedNormalizationFactor = 5.0f; // WLED patterns need higher speed multiplier
}

bool WLEDColorWavesPattern::update(unsigned long currentTime)
{
  if (!isActive || (currentTime - lastUpdate) < updateInterval)
  {
    return false;
  }
  lastUpdate = currentTime;

  counter += (uint16_t)(getEffectiveSpeed() * waveSpeed / 10);

  for (int i = 0; i < numLeds; i++)
  {
    // Create multiple wave frequencies
    uint16_t wave1 = sin16(counter + (i * 256));
    uint16_t wave2 = sin16((counter / 2) + (i * 128));
    uint16_t wave3 = sin16((counter / 3) + (i * 512));

    // Combine waves to create palette index
    uint8_t paletteIndex = ((wave1 + wave2 + wave3) / 3) / 256;

    // Get color from palette with smooth blending
    CRGB color = currentPalette ? currentPalette->getColor(paletteIndex) : CRGB(paletteIndex, 255 - paletteIndex, 128);

    leds[i] = color;
  }

  return true;
}

// ============================================================================
// WLED Noise Pattern (Noise 2 - Multi-octave algorithm)
// ============================================================================

WLEDNoisePattern::WLEDNoisePattern(CRGB *leds, int numLeds)
    : Pattern(leds, numLeds, 25), scale(30), noiseX(0), noiseY(0), noiseZ(0)
{
  speedNormalizationFactor = 6.0f; // WLED patterns need higher speed multiplier
}

bool WLEDNoisePattern::update(unsigned long currentTime)
{
  if (!isActive || (currentTime - lastUpdate) < updateInterval)
  {
    return false;
  }
  lastUpdate = currentTime;

  // Noise 2 algorithm: Multi-octave noise with slower Z movement
  // Based on WLED FX.cpp mode_noise implementation
  // Reference: https://github.com/wled/WLED/blob/main/wled00/FX.cpp

  noiseX += (uint16_t)(getEffectiveSpeed() * 150); // X axis movement
  noiseY += (uint16_t)(getEffectiveSpeed() * 130); // Y axis movement
  noiseZ += (uint16_t)(getEffectiveSpeed() * 90);  // Slower Z for Noise 2

  // Use fillnoise8 for the entire strip at once (more efficient)
  uint8_t noiseData[numLeds];

  // Fill noise array with multi-octave noise
  for (int i = 0; i < numLeds; i++)
  {
    // Primary noise layer
    uint16_t xPos = noiseX + (i * scale);
    uint8_t noise1 = inoise16(xPos, noiseY, noiseZ) >> 8;

    // Secondary noise layer with different scale (adds complexity)
    uint8_t noise2 = inoise16(xPos, noiseY + 5000, noiseZ + 3000) >> 8;

    // Combine noise layers for Noise 2 effect
    // Use qadd8 for saturating addition
    noiseData[i] = qadd8(scale8(noise1, 200), scale8(noise2, 100));
  }

  // Map noise values to palette colors
  for (int i = 0; i < numLeds; i++)
  {
    // Convert logical index to raw index for correct LED mapping
    uint16_t rawIndex = logicalToRawIndex(i);

    // Map noise to palette
    CRGB color = currentPalette ? currentPalette->getColor(noiseData[i]) : CHSV(noiseData[i], 255, 255);

    leds[rawIndex] = color;
  }

  return true;
}

// ============================================================================
// WLED Meteor Pattern
// ============================================================================

WLEDMeteorPattern::WLEDMeteorPattern(CRGB *leds, int numLeds)
    : Pattern(leds, numLeds, 40), meteorPosition(0), meteorSize(5),
      trailDecay(64), randomDecay(true), direction(1)
{
  speedNormalizationFactor = 4.0f; // Meteor needs higher speed - was very slow
}

bool WLEDMeteorPattern::update(unsigned long currentTime)
{
  if (!isActive || (currentTime - lastUpdate) < updateInterval)
  {
    return false;
  }
  lastUpdate = currentTime;

  // Fade all LEDs (create trail)
  for (int i = 0; i < numLeds; i++)
  {
    if (randomDecay && random8() > 200)
    {
      leds[i].fadeToBlackBy(trailDecay / 2);
    }
    else
    {
      leds[i].fadeToBlackBy(trailDecay);
    }
  }

  // Draw meteor head
  for (int j = 0; j < meteorSize; j++)
  {
    int ledIndex = meteorPosition + (j * direction);
    if (ledIndex >= 0 && ledIndex < numLeds)
    {
      // Get color from palette based on position in meteor
      uint8_t paletteIndex = (j * 255 / meteorSize);
      CRGB color = currentPalette ? currentPalette->getColor(paletteIndex) : CRGB::White;

      // Fade out at edges
      uint8_t brightness = 255 - (j * 255 / meteorSize);
      color.nscale8(brightness);

      leds[ledIndex] = color;
    }
  }

  // Move meteor (apply speed)
  meteorPosition += direction * (int)(getEffectiveSpeed());

  // Bounce at edges
  if (meteorPosition >= numLeds)
  {
    meteorPosition = 0;
  }
  else if (meteorPosition < 0)
  {
    meteorPosition = numLeds - 1;
  }

  return true;
}

// ============================================================================
// WLED Glitter Pattern
// ============================================================================

WLEDGlitterPattern::WLEDGlitterPattern(CRGB *leds, int numLeds)
    : Pattern(leds, numLeds, 20), glitterDensity(80), glitterBrightness(255)
{
  speedNormalizationFactor = 4.0f; // Glitter speed affects fade rate
}

bool WLEDGlitterPattern::update(unsigned long currentTime)
{
  if (!isActive || (currentTime - lastUpdate) < updateInterval)
  {
    return false;
  }
  lastUpdate = currentTime;

  // Fill with palette colors first
  for (int i = 0; i < numLeds; i++)
  {
    uint8_t paletteIndex = (i * 255 / numLeds);
    CRGB color = currentPalette ? currentPalette->getColor(paletteIndex) : CRGB(paletteIndex, 128, 255 - paletteIndex);
    leds[i] = color;
    leds[i].fadeToBlackBy(128); // Dim the base
  }

  // Add random glitter
  int glitterCount = (numLeds * glitterDensity) / 255;
  for (int i = 0; i < glitterCount; i++)
  {
    if (random8() < 200)
    {
      int pos = random16(numLeds);
      leds[pos] = CRGB::White;
      leds[pos].nscale8(glitterBrightness);
    }
  }

  // Fade all LEDs slightly for sparkle effect (speed affects fade rate)
  fadeToBlackBy(leds, numLeds, (uint8_t)(10 + getEffectiveSpeed() * 5));

  return true;
}

// ============================================================================
// WLED Two Dots Pattern
// ============================================================================

WLEDTwoDotsPattern::WLEDTwoDotsPattern(CRGB *leds, int numLeds)
    : Pattern(leds, numLeds, 25), dot1Position(0), dot2Position(numLeds / 2),
      dot1Speed(1.0f), dot2Speed(1.2f), dot1Direction(true), dot2Direction(false),
      fadeRate(64)
{
  speedNormalizationFactor = 7.0f; // Two Dots needs higher speed
}

bool WLEDTwoDotsPattern::update(unsigned long currentTime)
{
  if (!isActive || (currentTime - lastUpdate) < updateInterval)
  {
    return false;
  }
  lastUpdate = currentTime;

  // Fade all LEDs
  fadeToBlackBy(leds, numLeds, fadeRate);

  // Update dot 1
  if (dot1Direction)
  {
    dot1Position += dot1Speed * getEffectiveSpeed();
    if (dot1Position >= numLeds - 1)
    {
      dot1Position = numLeds - 1;
      dot1Direction = false;
    }
  }
  else
  {
    dot1Position -= dot1Speed * getEffectiveSpeed();
    if (dot1Position <= 0)
    {
      dot1Position = 0;
      dot1Direction = true;
    }
  }

  // Update dot 2
  if (dot2Direction)
  {
    dot2Position += dot2Speed * getEffectiveSpeed();
    if (dot2Position >= numLeds - 1)
    {
      dot2Position = numLeds - 1;
      dot2Direction = false;
    }
  }
  else
  {
    dot2Position -= dot2Speed * getEffectiveSpeed();
    if (dot2Position <= 0)
    {
      dot2Position = 0;
      dot2Direction = true;
    }
  }

  // Draw dots with colors from palette
  int pos1 = (int)dot1Position;
  int pos2 = (int)dot2Position;

  if (pos1 >= 0 && pos1 < numLeds)
  {
    CRGB color1 = currentPalette ? currentPalette->getColor(64) : CRGB::Red;
    leds[pos1] = color1;
  }

  if (pos2 >= 0 && pos2 < numLeds)
  {
    CRGB color2 = currentPalette ? currentPalette->getColor(192) : CRGB::Blue;
    leds[pos2] = color2;
  }

  return true;
}

// ============================================================================
// WLED Colortwinkles Pattern
// ============================================================================

WLEDColortwinklesPattern::WLEDColortwinklesPattern(CRGB *leds, int numLeds)
    : Pattern(leds, numLeds, 30), twinkleCount(0), spawnProbability(30)
{
  speedNormalizationFactor = 5.0f; // Colortwinkles speed affects spawn/fade
  // Initialize twinkle array
  for (int i = 0; i < MAX_TWINKLES; i++)
  {
    twinkles[i].ledIndex = 0;
    twinkles[i].brightness = 0;
    twinkles[i].colorIndex = 0;
    twinkles[i].fadeDirection = 0;
    twinkles[i].fadeSpeed = 0;
  }
}

void WLEDColortwinklesPattern::spawnTwinkle()
{
  if (twinkleCount >= MAX_TWINKLES)
  {
    return;
  }

  // Find empty slot
  for (int i = 0; i < MAX_TWINKLES; i++)
  {
    if (twinkles[i].fadeDirection == 0)
    {
      twinkles[i].ledIndex = random16(numLeds);
      twinkles[i].brightness = 0;
      twinkles[i].colorIndex = random8();
      twinkles[i].fadeDirection = 1; // Fading in
      twinkles[i].fadeSpeed = random8(5, 20);
      twinkleCount++;
      break;
    }
  }
}

void WLEDColortwinklesPattern::updateTwinkle(Twinkle &t)
{
  if (t.fadeDirection == 0)
  {
    return; // Inactive twinkle
  }

  // Calculate effective fade speed
  uint8_t effectiveFadeSpeed = (uint8_t)(t.fadeSpeed * getEffectiveSpeed());

  // Update brightness
  if (t.fadeDirection == 1)
  {
    // Fading in
    t.brightness += effectiveFadeSpeed;
    if (t.brightness >= 255)
    {
      t.brightness = 255;
      t.fadeDirection = -1; // Start fading out
    }
  }
  else
  {
    // Fading out
    if (t.brightness >= effectiveFadeSpeed)
    {
      t.brightness -= effectiveFadeSpeed;
    }
    else
    {
      t.brightness = 0;
      t.fadeDirection = 0; // Deactivate
      twinkleCount--;
    }
  }

  // Apply to LED
  if (t.ledIndex < (uint16_t)numLeds)
  {
    CRGB color = currentPalette ? currentPalette->getColor(t.colorIndex) : CHSV(t.colorIndex, 255, 255);
    color.nscale8(t.brightness);
    leds[t.ledIndex] = color;
  }
}

bool WLEDColortwinklesPattern::update(unsigned long currentTime)
{
  if (!isActive || (currentTime - lastUpdate) < updateInterval)
  {
    return false;
  }
  lastUpdate = currentTime;

  // Clear all LEDs
  fill_solid(leds, numLeds, CRGB::Black);

  // Try to spawn new twinkle (speed affects spawn probability)
  uint8_t effectiveSpawnProb = (uint8_t)(spawnProbability * getEffectiveSpeed());
  if (random8() < effectiveSpawnProb)
  {
    spawnTwinkle();
  }

  // Update all active twinkles
  for (int i = 0; i < MAX_TWINKLES; i++)
  {
    updateTwinkle(twinkles[i]);
  }

  return true;
}

// ============================================================================
// WLED Flow Pattern
// ============================================================================

WLEDFlowPattern::WLEDFlowPattern(CRGB *leds, int numLeds)
    : Pattern(leds, numLeds, 20), flowOffset(0), flowSpeed(128), blurAmount(128)
{
  speedNormalizationFactor = 6.0f; // Flow needs higher speed
}

bool WLEDFlowPattern::update(unsigned long currentTime)
{
  if (!isActive || (currentTime - lastUpdate) < updateInterval)
  {
    return false;
  }
  lastUpdate = currentTime;

  flowOffset += (uint16_t)(getEffectiveSpeed() * flowSpeed / 10);

  // Fill strip with flowing palette colors
  for (int i = 0; i < numLeds; i++)
  {
    // Calculate palette position with flow offset
    uint8_t paletteIndex = ((i * 256 / numLeds) + (flowOffset / 256)) & 0xFF;

    CRGB color = currentPalette ? currentPalette->getColor(paletteIndex) : CHSV(paletteIndex, 255, 255);

    leds[i] = color;
  }

  // Apply blur for smoother flow
  if (blurAmount > 0)
  {
    blur1d(leds, numLeds, blurAmount);
  }

  return true;
}
