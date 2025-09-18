#include "LEDDriver.h"
#include <Arduino.h>

LEDDriver::LEDDriver() : brightness(DEFAULT_BRIGHTNESS),
                         lastUpdate(0),
                         needsUpdate(false),
                         currentMode(MODE_MAIN),
                         currentR(STATIC_COLOR_R),
                         currentG(STATIC_COLOR_G),
                         currentB(STATIC_COLOR_B),
                         blinkState(false),
                         lastBlinkTime(0),
                         patternManager(nullptr),
                         globalBrightness(DEFAULT_GLOBAL_BRIGHTNESS),
                         globalSpeed(DEFAULT_GLOBAL_SPEED),
                         selectedPatternIndex(6), // Wave pattern (index 6)
                         selectedPaletteIndex(1), // Ocean palette (index 1)
                         settingsPhase(PHASE_1_QUADRANTS),
                         currentQuadrant(-1),
                         previewedItem(-1),
                         itemPreviewed(false),
                         holdStartTime(0),
                         isHolding(false),
                         flashState(false),
                         lastFlashTime(0),
                         stickyPointerPosition(-1),
                         hasStickyPointer(false),
                         pointerPosition(0),
                         lastPointerMove(0),
                         inCalibrationMode(false),
                         calibrationStartTime(0),
                         lastCalibrationBlink(0),
                         calibrationBlinkState(false),
                         xMin(JOYSTICK_MIN),
                         xMax(JOYSTICK_MAX),
                         yMin(JOYSTICK_MIN),
                         yMax(JOYSTICK_MAX),
                         clickCount(0),
                         lastClickTime(0),
                         firstClickTime(0)
{
  // Initialize joystick state
  joystickState.x = JOYSTICK_CENTER;
  joystickState.y = JOYSTICK_CENTER;
  joystickState.buttonPressed = false;
  joystickState.lastButtonState = false;
  joystickState.lastButtonChange = 0;
  joystickState.lastRead = 0;
}

LEDDriver::~LEDDriver()
{
  // Clean up pattern manager
  delete patternManager;
}

bool LEDDriver::initialize()
{
  Serial.println("Initializing LED Driver...");

  // Initialize FastLED
  FastLED.addLeds<LED_TYPE, DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS);
  FastLED.setBrightness(brightness);

  // Set FastLED power limit based on our safety constants
  if (ENABLE_POWER_LIMITING)
  {
    uint16_t safeCurrentMA = (uint16_t)(MAX_CURRENT_MA * (SAFETY_MARGIN_PERCENT / 100.0));
    FastLED.setMaxPowerInVoltsAndMilliamps(VOLTAGE_5V, safeCurrentMA);
    Serial.print("FastLED power limit set to ");
    Serial.print(VOLTAGE_5V);
    Serial.print("V, ");
    Serial.print(safeCurrentMA);
    Serial.println("mA");
  }

  // Clear all LEDs initially
  clear();
  show();

  // Initialize joystick pins (for future use)
  pinMode(JOYSTICK_BUTTON_PIN, INPUT_PULLUP);

  // Initialize pattern manager
  patternManager = new PatternManager(leds, NUM_LEDS);
  patternManager->initialize();

  Serial.print("LED Driver initialized with ");
  Serial.print(NUM_LEDS);
  Serial.println(" LEDs");

  return true;
}

void LEDDriver::update()
{
  unsigned long currentTime = millis();

  // Read joystick at specified interval (placeholder for future)
  if (currentTime - joystickState.lastRead >= JOYSTICK_READ_INTERVAL)
  {
    readJoystick();
    joystickState.lastRead = currentTime;
  }

  // Update LEDs at specified interval
  if (currentTime - lastUpdate >= LED_UPDATE_INTERVAL)
  {
    if (needsUpdate)
    {
      FastLED.show();
      needsUpdate = false;
    }
    lastUpdate = currentTime;
  }
}

void LEDDriver::setSolidColor(uint8_t r, uint8_t g, uint8_t b)
{
  setSolidColor(CRGB(r, g, b));
}

void LEDDriver::setSolidColor(CRGB color)
{
  for (int i = 0; i < NUM_LEDS; i++)
  {
    leds[i] = color;
  }
  needsUpdate = true;
}

void LEDDriver::setLED(int index, uint8_t r, uint8_t g, uint8_t b)
{
  setLED(index, CRGB(r, g, b));
}

void LEDDriver::setLED(int index, CRGB color)
{
  if (index >= 0 && index < NUM_LEDS)
  {
    leds[index] = color;
    needsUpdate = true;
  }
  else
  {
    Serial.print("Warning: LED index ");
    Serial.print(index);
    Serial.println(" is out of range");
  }
}

void LEDDriver::clear()
{
  for (int i = 0; i < NUM_LEDS; i++)
  {
    leds[i] = CRGB::Black;
  }
  needsUpdate = true;
}

void LEDDriver::setBrightness(uint8_t newBrightness)
{
  brightness = constrain(newBrightness, 0, MAX_BRIGHTNESS);
  FastLED.setBrightness(brightness);
  needsUpdate = true;

  // Only print brightness changes when explicitly requested
  // Serial.print("Brightness set to: ");
  // Serial.println(brightness);
}

uint8_t LEDDriver::getBrightness() const
{
  return brightness;
}

void LEDDriver::show()
{
  // Apply power limiting before displaying
  applyPowerLimiting();

  FastLED.show();
  needsUpdate = false;
}

void LEDDriver::readJoystick()
{
  // Read analog values from joystick
  int rawX = analogRead(JOYSTICK_X_PIN);
  int rawY = analogRead(JOYSTICK_Y_PIN);
  bool buttonState = !digitalRead(JOYSTICK_BUTTON_PIN); // Inverted due to pullup

  // Apply deadzone to prevent jitter
  if (abs(rawX - JOYSTICK_CENTER) < JOYSTICK_DEADZONE)
  {
    rawX = JOYSTICK_CENTER;
  }
  if (abs(rawY - JOYSTICK_CENTER) < JOYSTICK_DEADZONE)
  {
    rawY = JOYSTICK_CENTER;
  }

  // Update joystick state
  joystickState.x = rawX;
  joystickState.y = rawY;

  // Handle button press with debouncing
  unsigned long currentTime = millis();
  if (buttonState != joystickState.lastButtonState)
  {
    if (currentTime - joystickState.lastButtonChange > BUTTON_DEBOUNCE_MS)
    {
      joystickState.buttonPressed = buttonState;
      joystickState.lastButtonState = buttonState;
      joystickState.lastButtonChange = currentTime;

      // Check for double-click to enter settings mode
      if (buttonState && detectDoubleClick(buttonState, currentTime))
      {
        currentMode = MODE_SETTINGS;
        settingsPhase = PHASE_1_QUADRANTS;
        currentQuadrant = -1;
        previewedItem = -1;
        itemPreviewed = false;
        isHolding = false;
        Serial.println("Mode changed to: Settings Mode (Double-Click)");
      }
      // Normal mode switching on single button press
      else if (buttonState && !inCalibrationMode)
      {
        // If in settings mode, single click returns to main mode
        if (currentMode == MODE_SETTINGS)
        {
          currentMode = MODE_MAIN;
          Serial.println("Mode changed to: Main Mode - Exited Settings Mode");
        }
        else
        {
          // Normal cycling between Main -> Pointer -> Pattern -> Main
          if (currentMode == MODE_MAIN)
          {
            currentMode = MODE_POINTER;
            Serial.println("Mode changed to: Pointer Mode");
          }
          else if (currentMode == MODE_POINTER)
          {
            currentMode = MODE_PATTERN;
            Serial.println("Mode changed to: Pattern Browse Mode");
          }
          else if (currentMode == MODE_PATTERN)
          {
            currentMode = MODE_MAIN;
            Serial.println("Mode changed to: Main Mode");
          }
          else
          {
            // Fallback - should not happen, but go to main mode
            currentMode = MODE_MAIN;
            Serial.println("Mode changed to: Main Mode (fallback)");
          }
        }
      }
    }
  }

  // Process the joystick input based on current mode
  processJoystickInput();
}

void LEDDriver::processJoystickInput()
{
  // Handle calibration mode first
  if (inCalibrationMode)
  {
    processCalibrationMode();
    return;
  }

  switch (currentMode)
  {
  case MODE_MAIN:
    // Mode 0: Main mode - display selected pattern and palette
    processMainMode();
    break;

  case MODE_SETTINGS:
    // Mode 1: Settings mode - quadrant-based interface
    processSettingsMode();
    break;

  case MODE_POINTER:
    // Mode 2: Pointer mode - red background with chase pointer
    processPointerMode();
    break;

  case MODE_PATTERN:
    // Mode 3: Pattern browse mode - joystick controls pattern/palette
    processPatternMode();
    break;
  }
}

void LEDDriver::processMainMode()
{
  // Mode 0: Main mode - display selected pattern and palette
  unsigned long currentTime = millis();

  if (patternManager)
  {
    // Set the current pattern and palette based on selected indices
    patternManager->setCurrentPattern(selectedPatternIndex, false);
    patternManager->setCurrentPalette(selectedPaletteIndex);

    // Apply global settings
    patternManager->setGlobalBrightness(globalBrightness);
    patternManager->setGlobalSpeed(globalSpeed);

    // Update pattern manager - this will update the LEDs
    if (patternManager->update(currentTime))
    {
      show(); // Show updated LEDs
    }
  }
}

void LEDDriver::processPointerMode()
{
  // Mode 2: Pointer mode - circular pointer mode with background
  // Map joystick X/Y to circular LED position

  int xDiff = joystickState.x - JOYSTICK_CENTER;
  int yDiff = joystickState.y - JOYSTICK_CENTER;

  // Check if joystick is in deadzone
  int magnitude = sqrt(xDiff * xDiff + yDiff * yDiff);

  // Clear all LEDs and set background color
  // We do this every time to ensure clean display
  for (int i = 0; i < NUM_LEDS; i++)
  {
    setLED(i, POINTER_BG_COLOR_HTML, POINTER_BG_BRIGHTNESS);
  }

  if (magnitude > JOYSTICK_DEADZONE)
  {
    // Calculate angle in radians (atan2 returns -π to π)
    // Note: We need to flip Y coordinate to match expected behavior
    // Standard joystick: Up = higher Y values, but we want Up = 12 o'clock
    float angle = atan2(yDiff, xDiff); // Flip Y to correct coordinate system

    // Convert to our clock layout where:
    // - LED 0 is at 6 o'clock (bottom)
    // - LEDs go counter-clockwise
    // - We want: Up=12, Right=3, Down=6, Left=9

    // atan2(-yDiff, xDiff) gives: Right=0°, Up=90°, Left=180°/-180°, Down=-90°
    // We want: Down=0° (LED 0), Left=90°, Up=180°, Right=270°
    // So we need to rotate by +90°
    angle = angle + M_PI / 2; // Rotate +90° to put Down at 0°

    // Normalize to 0-2π range
    if (angle < 0)
    {
      angle += 2 * M_PI;
    }

    // Convert angle to LED position (0 to NUM_LEDS-1)
    // Starting at LED 0 (6 o'clock), going counter-clockwise
    int centerLED = (int)((angle / (2 * M_PI)) * NUM_LEDS) % NUM_LEDS;

    // Calculate dynamic width based on joystick magnitude
    // Map magnitude from deadzone edge to max range → width from min to max
    // Use maximum single-axis distance so straight directions get full width
    int maxRadius = JOYSTICK_MAX - JOYSTICK_CENTER; // e.g., 4095 - 1790 = 2305
    int maxMagnitude = maxRadius;                   // Use single-axis max, not diagonal
    int effectiveMagnitude = constrain(magnitude - JOYSTICK_DEADZONE, 0, maxMagnitude);
    int maxEffectiveMagnitude = maxMagnitude - JOYSTICK_DEADZONE;

    // Scale magnitude to width range
    int width = map(effectiveMagnitude, 0, maxEffectiveMagnitude, POINTER_WIDTH_MIN, POINTER_WIDTH_MAX);
    width = constrain(width, POINTER_WIDTH_MIN, POINTER_WIDTH_MAX);

    // Create the pointer with dynamic width
    createPointer(centerLED, width);
  }

  show();
}

void LEDDriver::createPointer(int centerLED, int width)
{
  // Light up 'width' number of LEDs centered around the target position
  // For even widths, we'll slightly favor one side to ensure proper centering
  int halfWidth = width / 2;

  for (int i = 0; i < width; i++)
  {
    // Calculate offset from center, ensuring proper centering for both odd and even widths
    int offset = i - halfWidth;

    // Handle LED index wrapping properly
    int ledIndex = centerLED + offset;
    while (ledIndex < 0)
      ledIndex += NUM_LEDS;
    while (ledIndex >= NUM_LEDS)
      ledIndex -= NUM_LEDS;

    // Set LED color with slight dimming for outer LEDs
    // Center LED(s) get full brightness, outer LEDs get 2/3 brightness
    uint8_t brightness;
    if (width == 1 || i == halfWidth || (width % 2 == 0 && i == halfWidth - 1))
    {
      brightness = POINTER_BRIGHTNESS; // Center LED(s) full brightness
    }
    else
    {
      brightness = (POINTER_BRIGHTNESS * 2 / 3); // Outer LEDs dimmed
    }

    setLED(ledIndex, POINTER_COLOR_HTML, brightness);
  }
}

void LEDDriver::setSolidColor(CRGB::HTMLColorCode colorName)
{
  setSolidColor(CRGB(colorName));
}

void LEDDriver::setSolidColor(uint8_t r, uint8_t g, uint8_t b, uint8_t brightness)
{
  // Scale colors by brightness
  uint8_t scaledR = (r * brightness) / 255;
  uint8_t scaledG = (g * brightness) / 255;
  uint8_t scaledB = (b * brightness) / 255;
  setSolidColor(scaledR, scaledG, scaledB);
}

void LEDDriver::setSolidColor(CRGB color, uint8_t brightness)
{
  // Scale the CRGB color by brightness
  CRGB scaledColor = CRGB(
      (color.r * brightness) / 255,
      (color.g * brightness) / 255,
      (color.b * brightness) / 255);
  setSolidColor(scaledColor);
}

void LEDDriver::setLED(int index, uint8_t r, uint8_t g, uint8_t b, uint8_t brightness)
{
  // Scale colors by brightness
  uint8_t scaledR = (r * brightness) / 255;
  uint8_t scaledG = (g * brightness) / 255;
  uint8_t scaledB = (b * brightness) / 255;
  setLED(index, scaledR, scaledG, scaledB);
}

void LEDDriver::setLED(int index, CRGB color, uint8_t brightness)
{
  // Scale the CRGB color by brightness
  CRGB scaledColor = CRGB(
      (color.r * brightness) / 255,
      (color.g * brightness) / 255,
      (color.b * brightness) / 255);
  setLED(index, scaledColor);
}

void LEDDriver::setMode(uint8_t mode)
{
  if (mode < NUM_MODES || mode == MODE_CALIBRATION)
  {
    // Handle calibration mode specially
    if (mode == MODE_CALIBRATION)
    {
      startCalibrationMode();
      return;
    }

    currentMode = mode;
    Serial.print("Mode manually set to: ");
    switch (currentMode)
    {
    case MODE_MAIN:
      Serial.println("Main Mode");
      break;
    case MODE_SETTINGS:
      Serial.println("Settings Mode");
      break;
    case MODE_POINTER:
      Serial.println("Pointer Mode");
      break;
    }
  }
  else
  {
    Serial.print("Invalid mode: ");
    Serial.println(mode);
  }
}

void LEDDriver::getCurrentColor(uint8_t &r, uint8_t &g, uint8_t &b) const
{
  r = currentR;
  g = currentG;
  b = currentB;
}

bool LEDDriver::detectDoubleClick(bool buttonPressed, unsigned long currentTime)
{
  // Only count button presses (not releases)
  if (!buttonPressed)
  {
    return false;
  }

  // Reset if too much time has passed since first click
  if (clickCount > 0 && (currentTime - firstClickTime) > DOUBLE_CLICK_TIMEOUT)
  {
    clickCount = 0;
  }

  // First click
  if (clickCount == 0)
  {
    clickCount = 1;
    firstClickTime = currentTime;
    lastClickTime = currentTime;
    Serial.println("Click 1/2");
    return false;
  }

  // Second click - check timing
  if ((currentTime - lastClickTime) < DOUBLE_CLICK_TIMEOUT)
  {
    clickCount++;
    lastClickTime = currentTime;

    Serial.print("Click ");
    Serial.print(clickCount);
    Serial.println("/2");

    if (clickCount >= 2)
    {
      clickCount = 0; // Reset for next time
      Serial.println("DOUBLE CLICK DETECTED!");
      return true;
    }
  }
  else
  {
    // Too slow, reset
    clickCount = 1;
    firstClickTime = currentTime;
    lastClickTime = currentTime;
    Serial.println("Click 1/2 (reset)");
  }

  return false;
}

void LEDDriver::startCalibrationMode()
{
  inCalibrationMode = true;
  calibrationStartTime = millis();
  lastCalibrationBlink = 0;
  calibrationBlinkState = false;

  // Initialize calibration bounds to current values
  xMin = xMax = joystickState.x;
  yMin = yMax = joystickState.y;

  Serial.println("=== CALIBRATION MODE STARTED ===");
  Serial.println("Move joystick to all extremes.");
  Serial.println("Press button to save, or wait 10s to auto-save.");
  Serial.println("LEDs will blink rapidly during calibration.");

  // Set LEDs to dim white to indicate calibration mode
  setSolidColor(64, 64, 64);
  show();
}

void LEDDriver::exitCalibrationMode()
{
  inCalibrationMode = false;
  saveCalibration();

  Serial.println("=== CALIBRATION MODE COMPLETE ===");
  Serial.print("X range: ");
  Serial.print(xMin);
  Serial.print(" to ");
  Serial.println(xMax);
  Serial.print("Y range: ");
  Serial.print(yMin);
  Serial.print(" to ");
  Serial.println(yMax);

  // Restore normal color
  setSolidColor(currentR, currentG, currentB);
  show();
}

void LEDDriver::processCalibrationMode()
{
  unsigned long currentTime = millis();
  static unsigned long lastValueDisplay = 0;
  const unsigned long VALUE_DISPLAY_INTERVAL = 250; // Display values every 250ms

  // Update calibration bounds with current joystick position
  bool boundsUpdated = false;
  if (joystickState.x < xMin)
  {
    xMin = joystickState.x;
    boundsUpdated = true;
  }
  if (joystickState.x > xMax)
  {
    xMax = joystickState.x;
    boundsUpdated = true;
  }
  if (joystickState.y < yMin)
  {
    yMin = joystickState.y;
    boundsUpdated = true;
  }
  if (joystickState.y > yMax)
  {
    yMax = joystickState.y;
    boundsUpdated = true;
  }

  // Display current joystick values and bounds periodically
  if (currentTime - lastValueDisplay >= VALUE_DISPLAY_INTERVAL)
  {
    Serial.print("Joystick: X=");
    Serial.print(joystickState.x);
    Serial.print(", Y=");
    Serial.print(joystickState.y);
    Serial.print(" | Bounds: X[");
    Serial.print(xMin);
    Serial.print("-");
    Serial.print(xMax);
    Serial.print("] Y[");
    Serial.print(yMin);
    Serial.print("-");
    Serial.print(yMax);
    Serial.print("] Range: X=");
    Serial.print(xMax - xMin);
    Serial.print(", Y=");
    Serial.println(yMax - yMin);

    lastValueDisplay = currentTime;
  }

  // Show immediate feedback when bounds are updated
  if (boundsUpdated)
  {
    Serial.print("*** New bound detected: X[");
    Serial.print(xMin);
    Serial.print("-");
    Serial.print(xMax);
    Serial.print("] Y[");
    Serial.print(yMin);
    Serial.print("-");
    Serial.print(yMax);
    Serial.println("] ***");
  }

  // Blink LEDs to show calibration is active
  if (currentTime - lastCalibrationBlink >= CALIBRATION_BLINK_RATE)
  {
    calibrationBlinkState = !calibrationBlinkState;
    lastCalibrationBlink = currentTime;

    if (calibrationBlinkState)
    {
      setSolidColor(255, 255, 255); // Bright white
    }
    else
    {
      setSolidColor(32, 32, 32); // Dim white
    }
    show();
  }

  // Check for button press to save calibration
  if (joystickState.buttonPressed)
  {
    exitCalibrationMode();
    return;
  }

  // Auto-exit after timeout
  if (currentTime - calibrationStartTime >= CALIBRATION_TIMEOUT)
  {
    Serial.println("Calibration timeout - auto-saving...");
    exitCalibrationMode();
  }
}

void LEDDriver::saveCalibration()
{
  // Validate calibration ranges
  int xRange = xMax - xMin;
  int yRange = yMax - yMin;

  if (xRange < MIN_JOYSTICK_RANGE || yRange < MIN_JOYSTICK_RANGE)
  {
    Serial.println("WARNING: Calibration range too small, using full range");
    xMin = JOYSTICK_MIN;
    xMax = JOYSTICK_MAX;
    yMin = JOYSTICK_MIN;
    yMax = JOYSTICK_MAX;
  }

  // Update the joystick center based on calibration
  // Center should be the middle of the calibrated range
  // Note: We're not updating JOYSTICK_CENTER constant, just using it for calculations

  Serial.println("Calibration saved successfully!");
}

void LEDDriver::loadCalibration()
{
  // In a real implementation, this would load from EEPROM/preferences
  // For now, we'll use hardcoded full-range values
  xMin = JOYSTICK_MIN;
  xMax = JOYSTICK_MAX;
  yMin = JOYSTICK_MIN;
  yMax = JOYSTICK_MAX;
}

void LEDDriver::getCalibrationBounds(int &xMinOut, int &xMaxOut, int &yMinOut, int &yMaxOut) const
{
  xMinOut = xMin;
  xMaxOut = xMax;
  yMinOut = yMin;
  yMaxOut = yMax;
}

// Power Management Implementation
float LEDDriver::calculateCurrentDraw()
{
  if (!ENABLE_POWER_LIMITING)
  {
    return 0.0; // Power limiting disabled
  }

  float totalCurrent = 0.0;

  // Calculate current draw based on actual LED colors and brightness
  for (int i = 0; i < NUM_LEDS; i++)
  {
    // Get the actual RGB values for each LED (after FastLED brightness scaling)
    CRGB ledColor = leds[i];

    // Calculate current for each color channel (assuming linear relationship)
    // Each channel can draw up to ~20mA at full intensity
    float ledCurrent = 0.0;
    ledCurrent += (ledColor.r / 255.0) * 20.0; // Red channel
    ledCurrent += (ledColor.g / 255.0) * 20.0; // Green channel
    ledCurrent += (ledColor.b / 255.0) * 20.0; // Blue channel

    // Apply global brightness scaling
    ledCurrent = (ledCurrent * brightness) / 255.0;

    totalCurrent += ledCurrent;
  }

  return totalCurrent;
}

float LEDDriver::calculatePowerConsumption()
{
  if (!ENABLE_POWER_LIMITING)
  {
    return 0.0; // Power limiting disabled
  }

  float current = calculateCurrentDraw();
  return (current / 1000.0) * VOLTAGE_5V; // Convert mA to A, then calculate watts
}

uint8_t LEDDriver::calculateSafeBrightness()
{
  if (!ENABLE_POWER_LIMITING)
  {
    return MAX_BRIGHTNESS; // No limiting
  }

  // Calculate theoretical maximum current if all LEDs were white at current brightness
  float theoreticalMaxCurrent = NUM_LEDS * LED_CURRENT_MA_PER_LED * (brightness / 255.0);

  // Apply safety margin
  float safeMaxCurrent = MAX_CURRENT_MA * (SAFETY_MARGIN_PERCENT / 100.0);

  if (theoreticalMaxCurrent <= safeMaxCurrent)
  {
    return brightness; // Current brightness is safe
  }

  // Calculate safe brightness level
  float safeBrightnessRatio = safeMaxCurrent / (NUM_LEDS * LED_CURRENT_MA_PER_LED);
  uint8_t safeBrightness = (uint8_t)(safeBrightnessRatio * 255.0);

  return constrain(safeBrightness, 1, MAX_BRIGHTNESS); // Ensure at least 1 for visibility
}

bool LEDDriver::isPowerLimitExceeded()
{
  if (!ENABLE_POWER_LIMITING)
  {
    return false; // Power limiting disabled
  }

  float currentDraw = calculateCurrentDraw();
  float powerConsumption = calculatePowerConsumption();
  float safeMaxCurrent = MAX_CURRENT_MA * (SAFETY_MARGIN_PERCENT / 100.0);

  return (currentDraw > safeMaxCurrent) || (powerConsumption > POWER_LIMIT_WATTS);
}

void LEDDriver::applyPowerLimiting()
{
  if (!ENABLE_POWER_LIMITING)
  {
    return; // Power limiting disabled
  }

  if (isPowerLimitExceeded())
  {
    uint8_t safeBrightness = calculateSafeBrightness();

    if (safeBrightness < brightness)
    {
      Serial.print("POWER LIMIT: Reducing brightness from ");
      Serial.print(brightness);
      Serial.print(" to ");
      Serial.println(safeBrightness);

      FastLED.setBrightness(safeBrightness);
      // Note: We don't update the internal brightness variable to preserve user setting
    }
  }
}

// Public power management methods
float LEDDriver::getCurrentPowerConsumption()
{
  return calculatePowerConsumption();
}

float LEDDriver::getCurrentDraw()
{
  return calculateCurrentDraw();
}

bool LEDDriver::isPowerLimited()
{
  if (!ENABLE_POWER_LIMITING)
  {
    return false;
  }

  uint8_t safeBrightness = calculateSafeBrightness();
  return safeBrightness < brightness;
}

// Pattern management methods
PatternManager &LEDDriver::getPatternManager()
{
  return *patternManager;
}

bool LEDDriver::handlePatternCommand(const String &command)
{
  if (patternManager)
  {
    return patternManager->handleSerialCommand(command);
  }
  return false;
}

void LEDDriver::processPatternMode()
{
  // Mode 3: Pattern browse mode - joystick controls pattern and palette selection
  unsigned long currentTime = millis();

  if (patternManager)
  {
    // Handle joystick input for pattern control
    int xDiff = joystickState.x - JOYSTICK_CENTER;
    int yDiff = joystickState.y - JOYSTICK_CENTER;

    // Use joystick X-axis to cycle patterns
    static int lastPatternChange = 0;
    static unsigned long lastPatternChangeTime = 0;
    const unsigned long PATTERN_CHANGE_INTERVAL = 300; // Minimum time between pattern changes

    if (abs(xDiff) > JOYSTICK_DEADZONE &&
        (currentTime - lastPatternChangeTime) > PATTERN_CHANGE_INTERVAL)
    {
      if (xDiff > 0 && lastPatternChange <= 0) // Right - next pattern
      {
        selectedPatternIndex = (selectedPatternIndex + 1) % patternManager->getPatternCount();
        patternManager->setCurrentPattern(selectedPatternIndex, false);
        Serial.print("Next pattern: ");
        Serial.println(patternManager->getCurrentPattern()->getName());
        lastPatternChange = 1;
        lastPatternChangeTime = currentTime;
      }
      else if (xDiff < 0 && lastPatternChange >= 0) // Left - previous pattern
      {
        selectedPatternIndex = (selectedPatternIndex - 1 + patternManager->getPatternCount()) % patternManager->getPatternCount();
        patternManager->setCurrentPattern(selectedPatternIndex, false);
        Serial.print("Previous pattern: ");
        Serial.println(patternManager->getCurrentPattern()->getName());
        lastPatternChange = -1;
        lastPatternChangeTime = currentTime;
      }
    }
    else if (abs(xDiff) <= JOYSTICK_DEADZONE)
    {
      lastPatternChange = 0; // Reset when joystick returns to center
    }

    // Use joystick Y-axis to cycle color palettes
    static int lastPaletteChange = 0;
    static unsigned long lastPaletteChangeTime = 0;
    const unsigned long PALETTE_CHANGE_INTERVAL = 300;

    if (abs(yDiff) > JOYSTICK_DEADZONE &&
        (currentTime - lastPaletteChangeTime) > PALETTE_CHANGE_INTERVAL)
    {
      if (yDiff > 0 && lastPaletteChange <= 0) // Up - next palette
      {
        selectedPaletteIndex = (selectedPaletteIndex + 1) % patternManager->getPaletteManager().getPaletteCount();
        patternManager->setCurrentPalette(selectedPaletteIndex);
        Serial.print("Next palette: ");
        Serial.println(patternManager->getPaletteManager().getCurrentPalette()->getName());
        lastPaletteChange = 1;
        lastPaletteChangeTime = currentTime;
      }
      else if (yDiff < 0 && lastPaletteChange >= 0) // Down - previous palette
      {
        selectedPaletteIndex = (selectedPaletteIndex - 1 + patternManager->getPaletteManager().getPaletteCount()) % patternManager->getPaletteManager().getPaletteCount();
        patternManager->setCurrentPalette(selectedPaletteIndex);
        Serial.print("Previous palette: ");
        Serial.println(patternManager->getPaletteManager().getCurrentPalette()->getName());
        lastPaletteChange = -1;
        lastPaletteChangeTime = currentTime;
      }
    }
    else if (abs(yDiff) <= JOYSTICK_DEADZONE)
    {
      lastPaletteChange = 0; // Reset when joystick returns to center
    }

    // Apply global settings
    patternManager->setGlobalBrightness(globalBrightness);
    patternManager->setGlobalSpeed(globalSpeed);

    // Update pattern manager - this will update the LEDs
    if (patternManager->update(currentTime))
    {
      show(); // Show updated LEDs
    }
  }
}

// Settings mode implementation
void LEDDriver::processSettingsMode()
{
  unsigned long currentTime = millis();

  // Handle joystick input for quadrant detection and holding logic
  int xDiff = joystickState.x - JOYSTICK_CENTER;
  int yDiff = joystickState.y - JOYSTICK_CENTER;
  int magnitude = sqrt(xDiff * xDiff + yDiff * yDiff);

  // Determine current quadrant based on joystick position
  int newQuadrant = -1;
  if (magnitude > JOYSTICK_DEADZONE)
  {
    newQuadrant = determineQuadrant(xDiff, yDiff);
  }

  // Handle quadrant changes and holding logic
  if (newQuadrant != currentQuadrant)
  {
    // Quadrant changed - stop any current holding
    if (isHolding)
    {
      stopHolding();
    }
    currentQuadrant = newQuadrant;

    // Start holding if in a valid quadrant
    if (currentQuadrant >= 0)
    {
      startHolding(currentQuadrant);
    }
  }

  // Process holding logic if currently holding
  if (isHolding && currentQuadrant >= 0)
  {
    unsigned long holdDuration = currentTime - holdStartTime;

    // Start flashing at warning time
    if (holdDuration >= SETTINGS_HOLD_WARNING_MS && holdDuration < SETTINGS_HOLD_SELECT_MS)
    {
      // Flash the pointer to indicate approaching selection
      if (currentTime - lastFlashTime >= SETTINGS_FLASH_INTERVAL)
      {
        flashState = !flashState;
        lastFlashTime = currentTime;
      }
    }

    // Selection complete
    if (holdDuration >= SETTINGS_HOLD_SELECT_MS)
    {
      selectCurrentItem();
      return; // selectCurrentItem handles mode transition
    }
  }

  // Render the appropriate phase
  if (settingsPhase == PHASE_1_QUADRANTS)
  {
    processSettingsPhase1();
  }
  else
  {
    processSettingsPhase2();
  }

  show();
}

// Settings mode helper methods (stubs for now)
void LEDDriver::processSettingsPhase1()
{
  // Clear all LEDs first
  for (int i = 0; i < NUM_LEDS; i++)
  {
    leds[i] = CRGB::Black;
  }

  // Render quadrant previews
  renderQuadrantPreviews();

  // Render pointer if in a quadrant
  if (currentQuadrant >= 0)
  {
    renderQuadrantPointer();
  }
}
void LEDDriver::processSettingsPhase2()
{
  // Clear all LEDs first
  for (int i = 0; i < NUM_LEDS; i++)
  {
    leds[i] = CRGB::Black;
  }

  // Handle joystick input for clock position selection
  int xDiff = joystickState.x - JOYSTICK_CENTER;
  int yDiff = joystickState.y - JOYSTICK_CENTER;
  int magnitude = sqrt(xDiff * xDiff + yDiff * yDiff);

  int newPosition = -1;
  if (magnitude > JOYSTICK_DEADZONE)
  {
    newPosition = determineClockPosition(xDiff, yDiff);

    // Restrict to available items for pattern/palette phases
    int maxValidPosition = 11; // Default for brightness/speed (12 positions)
    if (settingsPhase == PHASE_2_PATTERN && patternManager)
    {
      maxValidPosition = min(patternManager->getPatternCount(), SETTINGS_MAX_ITEMS) - 1;
    }
    else if (settingsPhase == PHASE_2_PALETTE && patternManager)
    {
      maxValidPosition = min(patternManager->getPaletteManager().getPaletteCount(), SETTINGS_MAX_ITEMS) - 1;
    }

    // Only allow valid positions
    if (newPosition > maxValidPosition)
    {
      newPosition = -1; // Invalid position, treat as deadzone
    }
  }

  // Handle position changes and holding logic
  if (newPosition != previewedItem)
  {
    if (isHolding)
    {
      stopHolding();
    }
    previewedItem = newPosition;
    itemPreviewed = (newPosition >= 0);

    if (newPosition >= 0)
    {
      startHolding(newPosition); // Reuse holding logic for position
    }
    else
    {
      // When returning to deadzone, don't clear sticky pointer immediately
      // Let it remain for visual feedback
    }
  }

  // Process holding logic for selection
  if (isHolding && previewedItem >= 0)
  {
    unsigned long currentTime = millis();
    unsigned long holdDuration = currentTime - holdStartTime;

    // Start flashing at warning time
    if (holdDuration >= SETTINGS_HOLD_WARNING_MS && holdDuration < SETTINGS_HOLD_SELECT_MS)
    {
      if (currentTime - lastFlashTime >= SETTINGS_FLASH_INTERVAL)
      {
        flashState = !flashState;
        lastFlashTime = currentTime;
      }
    }

    // Selection complete
    if (holdDuration >= SETTINGS_HOLD_SELECT_MS)
    {
      selectCurrentItem();
      return; // selectCurrentItem handles phase/mode transition
    }
  }

  // Render the appropriate Phase 2 interface
  switch (settingsPhase)
  {
  case PHASE_2_BRIGHTNESS:
    renderBrightnessPhase2();
    break;
  case PHASE_2_SPEED:
    renderSpeedPhase2();
    break;
  case PHASE_2_PATTERN:
    renderPatternPhase2();
    break;
  case PHASE_2_PALETTE:
    renderPalettePhase2();
    break;
  default:
    break;
  }
}
void LEDDriver::renderQuadrantPreviews()
{
  unsigned long currentTime = millis();

  // Calculate quadrant boundaries
  int ledsPerQuadrant = NUM_LEDS / 4;

  // Quadrant 0: Brightness (6-3 o'clock) - Red fading effect
  for (int i = 0; i < ledsPerQuadrant; i++)
  {
    int ledIndex = i;
    // Create fading effect using sine wave
    float phase = (float)i / ledsPerQuadrant * 2.0 * PI + (currentTime / 1000.0);
    uint8_t brightness = (uint8_t)((sin(phase) + 1.0) * 127.5);
    setLED(ledIndex, SETTINGS_BRIGHTNESS_COLOR, brightness / 3); // Dimmed for preview
  }

  // Quadrant 1: Speed (3-12 o'clock) - Chase effect
  for (int i = 0; i < ledsPerQuadrant; i++)
  {
    int ledIndex = ledsPerQuadrant + i;
    // Create chase effect
    int chasePos = ((currentTime / 200) % ledsPerQuadrant);
    if (i == chasePos || i == (chasePos + 1) % ledsPerQuadrant || i == (chasePos + 2) % ledsPerQuadrant)
    {
      setLED(ledIndex, SETTINGS_SPEED_COLOR, 100);
    }
    else
    {
      setLED(ledIndex, SETTINGS_SPEED_COLOR, 10);
    }
  }

  // Quadrant 2: Pattern (12-9 o'clock) - Wave with forest green palette
  for (int i = 0; i < ledsPerQuadrant; i++)
  {
    int ledIndex = 2 * ledsPerQuadrant + i;
    // Create wave effect with forest colors
    float angle = (float)i / ledsPerQuadrant * 2.0 * PI + (currentTime / 500.0);
    uint8_t intensity = (uint8_t)((sin(angle) + 1.0) * 127.5);
    CRGB forestColor = CRGB(0, intensity / 2, 0); // Green wave
    setLED(ledIndex, forestColor);
  }

  // Quadrant 3: Palette (9-6 o'clock) - Rainbow effect
  for (int i = 0; i < ledsPerQuadrant; i++)
  {
    int ledIndex = 3 * ledsPerQuadrant + i;
    // Create rainbow effect
    uint8_t hue = map(i, 0, ledsPerQuadrant - 1, 0, 255);
    CRGB rainbowColor = CHSV(hue, 255, 100); // Rainbow with moderate brightness
    setLED(ledIndex, rainbowColor);
  }
}
void LEDDriver::renderBrightnessPhase2()
{
  // Show 12 brightness levels around the clock
  for (int i = 0; i < 12; i++)
  {
    int ledIndex = (i * NUM_LEDS) / 12; // Map to LED positions
    uint8_t brightness = map(i, 0, 11, SETTINGS_BRIGHTNESS_MIN, SETTINGS_BRIGHTNESS_MAX);

    // Show brightness preview
    setLED(ledIndex, SETTINGS_BRIGHTNESS_COLOR, brightness);
  }

  // Always show 12 ticks for brightness (12 levels from min to max)
  renderSelectionTicks(12);

  // Always render pointer (active, sticky, or current setting)
  renderPhase2Pointer();
}
void LEDDriver::renderSpeedPhase2()
{
  // Preview speed with Chase pattern and Ocean blue palette
  if (patternManager)
  {
    // Find Chase pattern index (should be index 2 based on initialization order)
    int chasePatternIndex = -1;
    for (int i = 0; i < patternManager->getPatternCount(); i++)
    {
      if (patternManager->getPatternName(i).equalsIgnoreCase("Chase"))
      {
        chasePatternIndex = i;
        break;
      }
    }

    // Use Chase pattern if found, otherwise fallback to current pattern
    if (chasePatternIndex >= 0)
    {
      patternManager->setCurrentPattern(chasePatternIndex, false);
    }

    // Set Ocean palette for blue preview (index 1)
    ColorPalette *oceanPalette = patternManager->getPaletteManager().getPalette(1); // Ocean palette
    if (oceanPalette)
    {
      Pattern *currentPattern = patternManager->getCurrentPattern();
      if (currentPattern)
      {
        currentPattern->setPalette(oceanPalette);
      }
    }

    // Use sticky selection speed if available, otherwise use consistent preview speed
    float previewSpeed;
    if (hasStickyPointer && stickyPointerPosition >= 0)
    {
      // Use the sticky selection's speed
      previewSpeed = map(stickyPointerPosition, 0, 11, (int)(SETTINGS_SPEED_MIN * 10), (int)(SETTINGS_SPEED_MAX * 10)) / 10.0f;
    }
    else
    {
      // Use a moderate speed (1.5x) that shows the chase effect clearly
      previewSpeed = 1.5f;
    }
    patternManager->setGlobalSpeed(previewSpeed);

    // Apply current global brightness for consistent display
    patternManager->setGlobalBrightness(globalBrightness);

    // Update pattern to show speed effect
    patternManager->update(millis());
  }

  // Always show 12 ticks for speed (12 levels from min to max)
  renderSelectionTicks(12);

  // Always render pointer (active, sticky, or current setting)
  renderPhase2Pointer();
}

void LEDDriver::renderPatternPhase2()
{
  // Show pattern preview: active selection, sticky selection, or current pattern
  if (patternManager)
  {
    int displayPatternIndex = selectedPatternIndex; // Default to current pattern

    if (itemPreviewed && previewedItem >= 0)
    {
      // Active selection - show previewed pattern
      int totalPatterns = min(patternManager->getPatternCount(), SETTINGS_MAX_ITEMS);
      int maxPosition = totalPatterns - 1;
      displayPatternIndex = map(previewedItem, 0, maxPosition, 0, totalPatterns - 1);
      displayPatternIndex = constrain(displayPatternIndex, 0, totalPatterns - 1);
    }
    else if (hasStickyPointer && stickyPointerPosition >= 0)
    {
      // Sticky selection - show last previewed pattern
      int totalPatterns = min(patternManager->getPatternCount(), SETTINGS_MAX_ITEMS);
      int maxPosition = totalPatterns - 1;
      displayPatternIndex = map(stickyPointerPosition, 0, maxPosition, 0, totalPatterns - 1);
      displayPatternIndex = constrain(displayPatternIndex, 0, totalPatterns - 1);
    }
    // else: show current pattern (already set as default)

    patternManager->setCurrentPattern(displayPatternIndex, false);

    // Apply current global settings to ensure proper display
    patternManager->setGlobalBrightness(globalBrightness);
    patternManager->setGlobalSpeed(globalSpeed);

    patternManager->update(millis());
  }

  // Only show ticks for available patterns
  int availablePatterns = patternManager ? min(patternManager->getPatternCount(), SETTINGS_MAX_ITEMS) : 12;
  renderSelectionTicks(availablePatterns);

  // Always render pointer (active, sticky, or current setting)
  renderPhase2Pointer();
}

void LEDDriver::renderPalettePhase2()
{
  // Show palette preview: active selection, sticky selection, or current palette
  if (patternManager)
  {
    int displayPaletteIndex = selectedPaletteIndex; // Default to current palette

    if (itemPreviewed && previewedItem >= 0)
    {
      // Active selection - show previewed palette
      int totalPalettes = min(patternManager->getPaletteManager().getPaletteCount(), SETTINGS_MAX_ITEMS);
      int maxPosition = totalPalettes - 1;
      displayPaletteIndex = map(previewedItem, 0, maxPosition, 0, totalPalettes - 1);
      displayPaletteIndex = constrain(displayPaletteIndex, 0, totalPalettes - 1);
    }
    else if (hasStickyPointer && stickyPointerPosition >= 0)
    {
      // Sticky selection - show last previewed palette
      int totalPalettes = min(patternManager->getPaletteManager().getPaletteCount(), SETTINGS_MAX_ITEMS);
      int maxPosition = totalPalettes - 1;
      displayPaletteIndex = map(stickyPointerPosition, 0, maxPosition, 0, totalPalettes - 1);
      displayPaletteIndex = constrain(displayPaletteIndex, 0, totalPalettes - 1);
    }
    // else: show current palette (already set as default)

    // Use current pattern with the display palette
    patternManager->setCurrentPattern(selectedPatternIndex, false);
    patternManager->setCurrentPalette(displayPaletteIndex);

    // Apply current global settings to ensure proper display
    patternManager->setGlobalBrightness(globalBrightness);
    patternManager->setGlobalSpeed(globalSpeed);

    patternManager->update(millis());
  }

  // Only show ticks for available palettes
  int availablePalettes = patternManager ? min(patternManager->getPaletteManager().getPaletteCount(), SETTINGS_MAX_ITEMS) : 12;
  renderSelectionTicks(availablePalettes);

  // Always render pointer (active, sticky, or current setting)
  renderPhase2Pointer();
}

void LEDDriver::renderSelectionTicks(int numItems)
{
  // Render red tick marks at selection positions
  for (int i = 0; i < numItems && i < SETTINGS_MAX_ITEMS; i++)
  {
    int ledIndex = (i * NUM_LEDS) / numItems;

    // Add red tick marks (small, bright red indicators)
    for (int j = 0; j < POINTER_WIDTH_MAX; j++)
    {
      int tickIndex = (ledIndex + j - POINTER_WIDTH_MAX / 2 + NUM_LEDS) % NUM_LEDS;
      CRGB currentColor = leds[tickIndex];

      // Blend red tick with existing color
      currentColor.r = max(currentColor.r, (uint8_t)100);
      setLED(tickIndex, currentColor);
    }
  }
}

void LEDDriver::renderQuadrantPointer()
{
  // Calculate quadrant boundaries
  int ledsPerQuadrant = NUM_LEDS / 4;
  int quadrantStart = currentQuadrant * ledsPerQuadrant;
  int quadrantEnd = quadrantStart + ledsPerQuadrant - 1;

  // Calculate center of current quadrant
  int quadrantCenter = quadrantStart + ledsPerQuadrant / 2;

  // Determine pointer brightness based on flashing state
  uint8_t pointerBrightness = SETTINGS_POINTER_FLASH_MAX;
  if (isHolding && flashState)
  {
    pointerBrightness = SETTINGS_POINTER_FLASH_MIN;
  }

  // Create pointer in the center of the quadrant
  int pointerWidth = isHolding ? POINTER_WIDTH_MAX : POINTER_WIDTH_MIN;

  for (int i = 0; i < pointerWidth; i++)
  {
    int offset = i - pointerWidth / 2;
    int ledIndex = quadrantCenter + offset;

    // Wrap around within the quadrant
    while (ledIndex < quadrantStart)
      ledIndex += ledsPerQuadrant;
    while (ledIndex > quadrantEnd)
      ledIndex -= ledsPerQuadrant;

    // Override the quadrant preview with pointer color
    setLED(ledIndex, POINTER_COLOR_HTML, pointerBrightness);
  }
}

void LEDDriver::renderPhase2Pointer()
{
  // Determine what to display: active selection, sticky pointer, or current setting
  int displayPosition = -1;
  int displayWidth = POINTER_WIDTH_MIN;
  uint8_t displayBrightness = SETTINGS_POINTER_FLASH_MAX;
  bool isCurrentSetting = false;

  if (previewedItem >= 0 && previewedItem < 12)
  {
    // Active pointer - joystick is pointing at a position
    displayPosition = previewedItem;
    displayWidth = isHolding ? POINTER_WIDTH_MAX : POINTER_WIDTH_MIN;

    // Update sticky position
    stickyPointerPosition = previewedItem;
    hasStickyPointer = true;

    // Apply flashing if holding
    if (isHolding && flashState)
    {
      displayBrightness = SETTINGS_POINTER_FLASH_MIN;
    }
  }
  else if (hasStickyPointer && stickyPointerPosition >= 0)
  {
    // Sticky pointer - show last position at minimum size
    displayPosition = stickyPointerPosition;
    displayWidth = POINTER_WIDTH_MIN;
    displayBrightness = SETTINGS_POINTER_FLASH_MAX / 2; // Dimmed for sticky
  }
  else
  {
    // Show current setting position by default
    displayPosition = getCurrentSettingPosition();
    displayWidth = POINTER_WIDTH_MIN;
    displayBrightness = SETTINGS_POINTER_FLASH_MAX / 3; // Even more dimmed for current setting
    isCurrentSetting = true;
  }

  // Render the pointer
  if (displayPosition >= 0)
  {
    int ledIndex = (displayPosition * NUM_LEDS) / 12;

    for (int i = 0; i < displayWidth; i++)
    {
      int offset = i - displayWidth / 2;
      int targetIndex = (ledIndex + offset + NUM_LEDS) % NUM_LEDS;

      // Override with pointer color
      setLED(targetIndex, POINTER_COLOR_HTML, displayBrightness);
    }
  }
}

int LEDDriver::getCurrentSettingPosition()
{
  // Return the clock position (0-11) that corresponds to the current setting value
  switch (settingsPhase)
  {
  case PHASE_2_BRIGHTNESS:
    // Map current brightness to clock position (0-11)
    return map(globalBrightness, SETTINGS_BRIGHTNESS_MIN, SETTINGS_BRIGHTNESS_MAX, 0, 11);

  case PHASE_2_SPEED:
    // Map current speed to clock position (0-11)
    return map((int)(globalSpeed * 10), (int)(SETTINGS_SPEED_MIN * 10), (int)(SETTINGS_SPEED_MAX * 10), 0, 11);

  case PHASE_2_PATTERN:
    if (patternManager)
    {
      int totalPatterns = min(patternManager->getPatternCount(), SETTINGS_MAX_ITEMS);
      if (totalPatterns > 0)
      {
        return map(selectedPatternIndex, 0, totalPatterns - 1, 0, totalPatterns - 1);
      }
    }
    return 0;

  case PHASE_2_PALETTE:
    if (patternManager)
    {
      int totalPalettes = min(patternManager->getPaletteManager().getPaletteCount(), SETTINGS_MAX_ITEMS);
      if (totalPalettes > 0)
      {
        return map(selectedPaletteIndex, 0, totalPalettes - 1, 0, totalPalettes - 1);
      }
    }
    return 0;

  default:
    return 0;
  }
}

int LEDDriver::determineQuadrant(int x, int y)
{
  // Determine quadrant based on joystick position
  // Quadrant 0: Brightness (bottom-right, 6-3 o'clock)
  // Quadrant 1: Speed (top-right, 3-12 o'clock)
  // Quadrant 2: Pattern (top-left, 12-9 o'clock)
  // Quadrant 3: Palette (bottom-left, 9-6 o'clock)

  if (x >= 0 && y <= 0)
    return 0; // Bottom-right (Brightness)
  if (x >= 0 && y > 0)
    return 1; // Top-right (Speed)
  if (x < 0 && y > 0)
    return 2; // Top-left (Pattern)
  if (x < 0 && y <= 0)
    return 3; // Bottom-left (Palette)

  return -1; // Should not reach here
}
int LEDDriver::determineClockPosition(int x, int y)
{
  // Convert joystick position to clock position (0-11, representing 12, 1, 2, ... 11 o'clock)
  // We want: 12 o'clock (Up) = 0, 3 o'clock (Right) = 3, 6 o'clock (Down) = 6, 9 o'clock (Left) = 9
  // Note: This uses standard coordinate system, different from LED pointer mode

  float angle = atan2(y, x); // Standard coordinate system (Up = positive Y)

  // atan2(y, x) gives: Right=0°, Up=90°, Left=180°/-180°, Down=-90°
  // We want: Up=0° (12 o'clock), so we need to rotate by -90° (or +270°)
  angle = angle - M_PI / 2; // Rotate -90° to put 12 o'clock at 0°

  // Normalize to 0-2π range
  if (angle < 0)
  {
    angle += 2 * M_PI;
  }

  // Convert to 12 discrete positions (0 = 12 o'clock, 1 = 1 o'clock, etc.)
  // Add 0.5 for rounding to nearest position
  int clockPosition = (int)((angle / (2 * M_PI)) * 12 + 0.5) % 12;

  return clockPosition;
}
void LEDDriver::startHolding(int quadrant)
{
  isHolding = true;
  holdStartTime = millis();
  flashState = false;
  lastFlashTime = 0;
  // Serial.print("Started holding quadrant ");
  // Serial.println(quadrant);
}

void LEDDriver::stopHolding()
{
  isHolding = false;
  holdStartTime = 0;
  flashState = false;
  lastFlashTime = 0;
}

void LEDDriver::selectCurrentItem()
{
  // This method handles both Phase 1 and Phase 2 selections
  if (settingsPhase == PHASE_1_QUADRANTS)
  {
    // Phase 1: Quadrant selection - transition to Phase 2
    Serial.print("PHASE 1: Selected quadrant ");
    Serial.println(currentQuadrant);

    switch (currentQuadrant)
    {
    case 0: // Brightness
      settingsPhase = PHASE_2_BRIGHTNESS;
      Serial.println("Entering Brightness Selection Phase");
      break;
    case 1: // Speed
      settingsPhase = PHASE_2_SPEED;
      Serial.println("Entering Speed Selection Phase");
      break;
    case 2: // Pattern
      settingsPhase = PHASE_2_PATTERN;
      Serial.println("Entering Pattern Selection Phase");
      break;
    case 3: // Palette
      settingsPhase = PHASE_2_PALETTE;
      Serial.println("Entering Palette Selection Phase");
      break;
    }

    // Reset holding state for Phase 2
    stopHolding();
    currentQuadrant = -1;
    previewedItem = -1;
    itemPreviewed = false;
    stickyPointerPosition = -1;
    hasStickyPointer = false;
  }
  else
  {
    // Phase 2: Clock position selection - apply the setting
    applySelectedSetting();
  }
}

void LEDDriver::applySelectedSetting()
{
  Serial.print("PHASE 2: Applying setting at clock position ");
  Serial.println(previewedItem);

  switch (settingsPhase)
  {
  case PHASE_2_BRIGHTNESS:
    // Map clock position (0-11) to brightness range
    globalBrightness = map(previewedItem, 0, 11, SETTINGS_BRIGHTNESS_MIN, SETTINGS_BRIGHTNESS_MAX);
    Serial.print("Set global brightness to ");
    Serial.println(globalBrightness);
    break;

  case PHASE_2_SPEED:
    // Map clock position (0-11) to speed range
    globalSpeed = map(previewedItem, 0, 11, (int)(SETTINGS_SPEED_MIN * 10), (int)(SETTINGS_SPEED_MAX * 10)) / 10.0f;
    Serial.print("Set global speed to ");
    Serial.println(globalSpeed);
    break;

  case PHASE_2_PATTERN:
    // Map clock position to pattern index (only available patterns)
    if (patternManager)
    {
      int totalPatterns = min(patternManager->getPatternCount(), SETTINGS_MAX_ITEMS);
      int maxPosition = totalPatterns - 1; // Only map to available positions
      selectedPatternIndex = map(previewedItem, 0, maxPosition, 0, totalPatterns - 1);
      selectedPatternIndex = constrain(selectedPatternIndex, 0, totalPatterns - 1);
      Serial.print("Set pattern index to ");
      Serial.println(selectedPatternIndex);
    }
    break;

  case PHASE_2_PALETTE:
    // Map clock position to palette index (only available palettes)
    if (patternManager)
    {
      int totalPalettes = min(patternManager->getPaletteManager().getPaletteCount(), SETTINGS_MAX_ITEMS);
      int maxPosition = totalPalettes - 1; // Only map to available positions
      selectedPaletteIndex = map(previewedItem, 0, maxPosition, 0, totalPalettes - 1);
      selectedPaletteIndex = constrain(selectedPaletteIndex, 0, totalPalettes - 1);
      Serial.print("Set palette index to ");
      Serial.println(selectedPaletteIndex);
    }
    break;

  default:
    break;
  }

  // Return to main mode
  currentMode = MODE_MAIN;
  settingsPhase = PHASE_1_QUADRANTS;
  stopHolding();
  currentQuadrant = -1;
  previewedItem = -1;
  itemPreviewed = false;
  stickyPointerPosition = -1;
  hasStickyPointer = false;

  Serial.println("Returning to Main Mode");
}
