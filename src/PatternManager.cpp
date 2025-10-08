#include "PatternManager.h"
#include "WLEDPatterns.h"

// ============================================================================
// PatternManager Implementation
// ============================================================================

PatternManager::PatternManager(CRGB *leds, int numLeds, SegmentManager *segManager)
    : leds(leds), numLeds(numLeds), segmentManager(segManager), patternCount(0), currentPatternIndex(0),
      autoSwitch(false), autoSwitchInterval(30000), lastAutoSwitch(0),
      globalBrightness(255), globalSpeed(DEFAULT_GLOBAL_SPEED),
      inTransition(false), transitionStart(0), transitionDuration(1000),
      fromPatternIndex(0), toPatternIndex(0)
{

  // Initialize pattern array
  for (int i = 0; i < MAX_PATTERNS; i++)
  {
    patterns[i] = nullptr;
  }
}

PatternManager::~PatternManager()
{
  // Clean up allocated patterns
  for (int i = 0; i < patternCount; i++)
  {
    delete patterns[i];
  }
}

void PatternManager::initialize()
{
  Serial.println("Initializing pattern manager...");

  // Initialize palette manager first
  paletteManager.initialize();

  // Create and add default patterns
  addPattern(new SolidPattern(leds, numLeds, CRGB::White));
  addPattern(new RainbowPattern(leds, numLeds));
  addPattern(new ChasePattern(leds, numLeds, CRGB::Red, 15));
  // addPattern(new PulsePattern(leds, numLeds, CRGB::Blue));
  addPattern(new TwinklePattern(leds, numLeds, 20));
  addPattern(new WavePattern(leds, numLeds, CRGB::Cyan, 10));

  // Add segment-aware patterns if SegmentManager is available
  if (segmentManager != nullptr)
  {
    addPattern(new MultiRingPattern(leds, numLeds, segmentManager, 3));
    // addPattern(new SpiralPattern(leds, numLeds, segmentManager, 2));
    addPattern(new RipplePattern(leds, numLeds, segmentManager, 1000));
    // addPattern(new EyeBreathingPattern(leds, numLeds, segmentManager));
    // addPattern(new SegmentTestPattern(leds, numLeds, segmentManager, SEGMENT_TEST_INTERVAL));
  }

  // Add WLED patterns
  addPattern(new WLEDDancingShadowsPattern(leds, numLeds));
  addPattern(new WLEDColorWavesPattern(leds, numLeds));
  addPattern(new WLEDNoisePattern(leds, numLeds));
  addPattern(new WLEDMeteorPattern(leds, numLeds));
  addPattern(new WLEDGlitterPattern(leds, numLeds));
  addPattern(new WLEDTwoDotsPattern(leds, numLeds));
  addPattern(new WLEDColortwinklesPattern(leds, numLeds));
  addPattern(new WLEDFlowPattern(leds, numLeds));

  // Set initial palette for all patterns
  ColorPalette *defaultPalette = paletteManager.getCurrentPalette();
  for (int i = 0; i < patternCount; i++)
  {
    patterns[i]->setPalette(defaultPalette);
  }

  Serial.print("Loaded ");
  Serial.print(patternCount);
  Serial.println(" patterns");

  // Initialize current pattern
  if (patternCount > 0)
  {
    patterns[currentPatternIndex]->initialize();
    applyGlobalSettings();
  }
}

bool PatternManager::update(unsigned long currentTime)
{
  bool updated = false;

  // Handle auto-switching
  if (autoSwitch && !inTransition && (currentTime - lastAutoSwitch) >= autoSwitchInterval)
  {
    nextPattern(false);
    lastAutoSwitch = currentTime;
  }

  // Handle transitions
  if (inTransition)
  {
    updateTransition(currentTime);
    updated = true;
  }
  else
  {
    // Update current pattern
    Pattern *currentPattern = getCurrentPattern();
    if (currentPattern && currentPattern->getActive())
    {
      updated = currentPattern->update(currentTime);
    }
  }

  return updated;
}

bool PatternManager::addPattern(Pattern *pattern)
{
  if (patternCount >= MAX_PATTERNS || !pattern)
  {
    return false;
  }

  patterns[patternCount] = pattern;
  patternCount++;
  return true;
}

Pattern *PatternManager::getPattern(int index)
{
  if (index < 0 || index >= patternCount)
  {
    return nullptr;
  }
  return patterns[index];
}

Pattern *PatternManager::getPattern(const String &name)
{
  for (int i = 0; i < patternCount; i++)
  {
    if (patterns[i]->getName().equalsIgnoreCase(name))
    {
      return patterns[i];
    }
  }
  return nullptr;
}

Pattern *PatternManager::getCurrentPattern()
{
  if (currentPatternIndex < 0 || currentPatternIndex >= patternCount)
  {
    return nullptr;
  }
  return patterns[currentPatternIndex];
}

bool PatternManager::setCurrentPattern(int index, bool useTransition)
{
  if (index < 0 || index >= patternCount)
  {
    return false;
  }

  if (useTransition && !inTransition)
  {
    startTransition(index);
  }
  else
  {
    // Deactivate current pattern
    if (getCurrentPattern())
    {
      getCurrentPattern()->setActive(false);
    }

    currentPatternIndex = index;

    // Activate new pattern
    Pattern *newPattern = getCurrentPattern();
    if (newPattern)
    {
      newPattern->initialize();
      newPattern->setActive(true);
      applyGlobalSettings();
    }
  }

  return true;
}

bool PatternManager::setCurrentPattern(const String &name, bool useTransition)
{
  for (int i = 0; i < patternCount; i++)
  {
    if (patterns[i]->getName().equalsIgnoreCase(name))
    {
      return setCurrentPattern(i, useTransition);
    }
  }
  return false;
}

String PatternManager::getPatternName(int index)
{
  if (index < 0 || index >= patternCount)
  {
    return "";
  }
  return patterns[index]->getName();
}

void PatternManager::nextPattern(bool useTransition)
{
  int nextIndex = currentPatternIndex + 1;
  if (nextIndex >= patternCount)
  {
    nextIndex = 0;
  }
  setCurrentPattern(nextIndex, false); // Always use instant switching
}

void PatternManager::previousPattern(bool useTransition)
{
  int prevIndex = currentPatternIndex - 1;
  if (prevIndex < 0)
  {
    prevIndex = patternCount - 1;
  }
  setCurrentPattern(prevIndex, false); // Always use instant switching
}

bool PatternManager::setCurrentPalette(int paletteIndex)
{
  if (!paletteManager.setCurrentPalette(paletteIndex))
  {
    return false;
  }

  // Update current pattern's palette
  Pattern *currentPattern = getCurrentPattern();
  if (currentPattern)
  {
    currentPattern->setPalette(paletteManager.getCurrentPalette());
  }

  return true;
}

bool PatternManager::setCurrentPalette(const String &paletteName)
{
  if (!paletteManager.setCurrentPalette(paletteName))
  {
    return false;
  }

  // Update current pattern's palette
  Pattern *currentPattern = getCurrentPattern();
  if (currentPattern)
  {
    currentPattern->setPalette(paletteManager.getCurrentPalette());
  }

  return true;
}

void PatternManager::nextPalette()
{
  paletteManager.nextPalette();

  // Update current pattern's palette
  Pattern *currentPattern = getCurrentPattern();
  if (currentPattern)
  {
    currentPattern->setPalette(paletteManager.getCurrentPalette());
  }
}

void PatternManager::previousPalette()
{
  paletteManager.previousPalette();

  // Update current pattern's palette
  Pattern *currentPattern = getCurrentPattern();
  if (currentPattern)
  {
    currentPattern->setPalette(paletteManager.getCurrentPalette());
  }
}

void PatternManager::setGlobalBrightness(uint8_t brightness)
{
  globalBrightness = brightness;
  applyGlobalSettings();
}

void PatternManager::setGlobalSpeed(float speed)
{
  globalSpeed = constrain(speed, SETTINGS_SPEED_MIN, SETTINGS_SPEED_MAX);
  applyGlobalSettings();
}

void PatternManager::setAutoSwitch(bool enable, unsigned long intervalMs)
{
  autoSwitch = enable;
  autoSwitchInterval = intervalMs;
  lastAutoSwitch = millis();
}

void PatternManager::startTransition(int toIndex, unsigned long durationMs)
{
  if (toIndex < 0 || toIndex >= patternCount || toIndex == currentPatternIndex)
  {
    return;
  }

  inTransition = true;
  transitionStart = millis();
  transitionDuration = durationMs;
  fromPatternIndex = currentPatternIndex;
  toPatternIndex = toIndex;

  // Initialize target pattern
  patterns[toPatternIndex]->initialize();
  patterns[toPatternIndex]->setActive(true);
  applyGlobalSettings();
}

void PatternManager::updateTransition(unsigned long currentTime)
{
  if (!inTransition)
    return;

  unsigned long elapsed = currentTime - transitionStart;

  if (elapsed >= transitionDuration)
  {
    // Transition complete
    patterns[fromPatternIndex]->setActive(false);
    currentPatternIndex = toPatternIndex;
    inTransition = false;

    Serial.print("Transitioned to pattern: ");
    Serial.println(getCurrentPattern()->getName());
  }
  else
  {
    // Update both patterns during transition
    float progress = (float)elapsed / (float)transitionDuration;

    // Update from pattern with decreasing brightness
    uint8_t fromBrightness = (uint8_t)((1.0f - progress) * globalBrightness);
    patterns[fromPatternIndex]->setBrightness(fromBrightness);
    patterns[fromPatternIndex]->update(currentTime);

    // Update to pattern with increasing brightness
    uint8_t toBrightness = (uint8_t)(progress * globalBrightness);
    patterns[toPatternIndex]->setBrightness(toBrightness);
    patterns[toPatternIndex]->update(currentTime);

    // Blend the results (simple additive blending)
    // Note: This is a basic implementation - could be enhanced with more sophisticated blending
  }
}

void PatternManager::applyGlobalSettings()
{
  Pattern *currentPattern = getCurrentPattern();
  if (currentPattern)
  {
    currentPattern->setBrightness(globalBrightness);
    currentPattern->setSpeed(globalSpeed);
  }
}

void PatternManager::printPatterns()
{
  Serial.println("=== Available Patterns ===");
  for (int i = 0; i < patternCount; i++)
  {
    Serial.print(i);
    Serial.print(": ");
    Serial.print(patterns[i]->getName());
    Serial.print(" - ");
    Serial.println(patterns[i]->getDescription());
  }
  Serial.print("Current pattern: ");
  Serial.println(getCurrentPattern() ? getCurrentPattern()->getName() : "None");
}

void PatternManager::printStatus()
{
  Serial.println("=== Pattern Manager Status ===");
  Serial.print("Current Pattern: ");
  Serial.print(getCurrentPattern() ? getCurrentPattern()->getName() : "None");
  Serial.print(" (");
  Serial.print(currentPatternIndex);
  Serial.print("/");
  Serial.print(patternCount - 1);
  Serial.println(")");

  Serial.print("Current Palette: ");
  ColorPalette *currentPalette = paletteManager.getCurrentPalette();
  Serial.println(currentPalette ? currentPalette->getName() : "None");

  Serial.print("Global Brightness: ");
  Serial.println(globalBrightness);

  Serial.print("Global Speed: ");
  Serial.println(globalSpeed);

  Serial.print("Auto Switch: ");
  Serial.print(autoSwitch ? "ON" : "OFF");
  if (autoSwitch)
  {
    Serial.print(" (");
    Serial.print(autoSwitchInterval / 1000);
    Serial.print("s)");
  }
  Serial.println();

  Serial.print("In Transition: ");
  Serial.println(inTransition ? "YES" : "NO");
}

bool PatternManager::handleSerialCommand(const String &command)
{
  // Pattern commands
  if (command.startsWith("pattern "))
  {
    String param = command.substring(8);
    param.trim();

    if (param.length() == 0 || param == "list")
    {
      printPatterns();
      return true;
    }

    // Try to parse as number first
    int patternIndex = param.toInt();
    if (patternIndex >= 0 && patternIndex < patternCount)
    {
      setCurrentPattern(patternIndex, false);
      Serial.print("Set pattern to: ");
      Serial.println(getCurrentPattern()->getName());
      return true;
    }

    // Try to find by name
    if (setCurrentPattern(param, false))
    {
      Serial.print("Set pattern to: ");
      Serial.println(getCurrentPattern()->getName());
      return true;
    }

    Serial.println("Pattern not found");
    return true;
  }

  if (command == "pattern next" || command == "next pattern")
  {
    nextPattern(false);
    Serial.print("Next pattern: ");
    Serial.println(getCurrentPattern()->getName());
    return true;
  }

  if (command == "pattern prev" || command == "prev pattern")
  {
    previousPattern(false);
    Serial.print("Previous pattern: ");
    Serial.println(getCurrentPattern()->getName());
    return true;
  }

  // Palette commands
  if (command.startsWith("palette "))
  {
    String param = command.substring(8);
    param.trim();

    if (param.length() == 0 || param == "list")
    {
      paletteManager.printPalettes();
      return true;
    }

    // Try to parse as number first
    int paletteIndex = param.toInt();
    if (paletteIndex >= 0 && paletteIndex < paletteManager.getPaletteCount())
    {
      setCurrentPalette(paletteIndex);
      Serial.print("Set palette to: ");
      Serial.println(paletteManager.getCurrentPalette()->getName());
      return true;
    }

    // Try to find by name
    if (setCurrentPalette(param))
    {
      Serial.print("Set palette to: ");
      Serial.println(paletteManager.getCurrentPalette()->getName());
      return true;
    }

    Serial.println("Palette not found");
    return true;
  }

  if (command == "palette next" || command == "next palette")
  {
    nextPalette();
    Serial.print("Next palette: ");
    Serial.println(paletteManager.getCurrentPalette()->getName());
    return true;
  }

  if (command == "palette prev" || command == "prev palette")
  {
    previousPalette();
    Serial.print("Previous palette: ");
    Serial.println(paletteManager.getCurrentPalette()->getName());
    return true;
  }

  // Speed commands
  if (command.startsWith("speed "))
  {
    float speed = command.substring(6).toFloat();
    if (speed >= SETTINGS_SPEED_MIN && speed <= SETTINGS_SPEED_MAX)
    {
      setGlobalSpeed(speed);
      Serial.print("Set speed to: ");
      Serial.println(speed);
    }
    else
    {
      Serial.println("Speed must be between " + String(SETTINGS_SPEED_MIN) + " and " + String(SETTINGS_SPEED_MAX));
    }
    return true;
  }

  // Auto switch commands
  if (command.startsWith("auto "))
  {
    String param = command.substring(5);
    param.trim();

    if (param == "on" || param == "enable")
    {
      setAutoSwitch(true);
      Serial.println("Auto-switch enabled");
    }
    else if (param == "off" || param == "disable")
    {
      setAutoSwitch(false);
      Serial.println("Auto-switch disabled");
    }
    else
    {
      int intervalSec = param.toInt();
      if (intervalSec >= 5)
      {
        setAutoSwitch(true, intervalSec * 1000);
        Serial.print("Auto-switch enabled with ");
        Serial.print(intervalSec);
        Serial.println(" second interval");
      }
      else
      {
        Serial.println("Auto interval must be at least 5 seconds");
      }
    }
    return true;
  }

  if (command == "status")
  {
    printStatus();
    return true;
  }

  return false; // Command not handled
}
