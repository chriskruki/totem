#include "PolePatternConfig.h"

// ============================================================================
// PolePatternManager Implementation
// ============================================================================

PolePatternManager::PolePatternManager(CRGB *mainLeds, int mainNumLeds, CRGB *poleLeds, int poleNumLeds)
    : mainLeds(mainLeds), mainNumLeds(mainNumLeds), poleLeds(poleLeds), poleNumLeds(poleNumLeds),
      patternCount(0), currentPatternIndex(0)
{
  // Initialize pattern array
  for (int i = 0; i < MAX_POLE_PATTERNS; i++)
  {
    patterns[i] = nullptr;
  }
}

PolePatternManager::~PolePatternManager()
{
  // Clean up allocated patterns
  for (int i = 0; i < patternCount; i++)
  {
    if (patterns[i] != nullptr)
    {
      delete patterns[i];
      patterns[i] = nullptr;
    }
  }
}

void PolePatternManager::initialize()
{
  Serial.println("Initializing pole pattern manager...");

  // Create and add enabled pole patterns
  // Comment out patterns in PolePatternConfig.h to disable them

#ifdef ENABLE_POLE_COLUMN_WAVE
  addPattern(new PoleColumnWavePattern(mainLeds, mainNumLeds, poleLeds, poleNumLeds));
#endif

#ifdef ENABLE_POLE_SPIRAL_CHASE
  addPattern(new PoleSpiralChasePattern(mainLeds, mainNumLeds, poleLeds, poleNumLeds));
#endif

#ifdef ENABLE_POLE_HELIX
  addPattern(new PoleHelixPattern(mainLeds, mainNumLeds, poleLeds, poleNumLeds));
#endif

#ifdef ENABLE_POLE_FIRE
  addPattern(new PoleFirePattern(mainLeds, mainNumLeds, poleLeds, poleNumLeds));
#endif

#ifdef ENABLE_POLE_BOUNCE
  addPattern(new PoleBouncePattern(mainLeds, mainNumLeds, poleLeds, poleNumLeds));
#endif

  Serial.print("Loaded ");
  Serial.print(patternCount);
  Serial.println(" pole patterns");

  // Set first pattern as active
  if (patternCount > 0)
  {
    patterns[currentPatternIndex]->setActive(true);
  }
}

bool PolePatternManager::addPattern(PolePattern *pattern)
{
  if (patternCount >= MAX_POLE_PATTERNS || pattern == nullptr)
  {
    Serial.println("Failed to add pole pattern (full or null)");
    return false;
  }

  patterns[patternCount] = pattern;
  patternCount++;
  return true;
}

PolePattern *PolePatternManager::getPattern(int index)
{
  if (index < 0 || index >= patternCount)
  {
    return nullptr;
  }
  return patterns[index];
}

PolePattern *PolePatternManager::getCurrentPattern()
{
  return getPattern(currentPatternIndex);
}

void PolePatternManager::setCurrentPattern(int index)
{
  if (index < 0 || index >= patternCount)
  {
    return;
  }

  // Deactivate current pattern
  if (patterns[currentPatternIndex] != nullptr)
  {
    patterns[currentPatternIndex]->setActive(false);
  }

  // Set new pattern
  currentPatternIndex = index;

  // Activate new pattern
  if (patterns[currentPatternIndex] != nullptr)
  {
    patterns[currentPatternIndex]->setActive(true);
  }
}

bool PolePatternManager::update(unsigned long currentTime)
{
  PolePattern *currentPattern = getCurrentPattern();
  if (currentPattern != nullptr && currentPattern->getActive())
  {
    return currentPattern->update(currentTime);
  }
  return false;
}

void PolePatternManager::setPalette(ColorPalette *palette)
{
  for (int i = 0; i < patternCount; i++)
  {
    if (patterns[i] != nullptr)
    {
      patterns[i]->setPalette(palette);
    }
  }
}

void PolePatternManager::setBrightness(uint8_t brightness)
{
  for (int i = 0; i < patternCount; i++)
  {
    if (patterns[i] != nullptr)
    {
      patterns[i]->setBrightness(brightness);
    }
  }
}

void PolePatternManager::setSpeed(float speed)
{
  for (int i = 0; i < patternCount; i++)
  {
    if (patterns[i] != nullptr)
    {
      patterns[i]->setSpeed(speed);
    }
  }
}

String PolePatternManager::getPatternName(int index)
{
  PolePattern *pattern = getPattern(index);
  if (pattern != nullptr)
  {
    return pattern->getName();
  }
  return "Unknown";
}
