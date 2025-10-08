#include "LEDDriver.h"
#include <Arduino.h>

LEDDriver::LEDDriver() : brightness(DEFAULT_BRIGHTNESS),
                         lastUpdate(0),
                         needsUpdate(false),
                         currentMode(MAIN_MODE_EXPLORER), // Start in Explorer mode
                         currentR(STATIC_COLOR_R),
                         currentG(STATIC_COLOR_G),
                         currentB(STATIC_COLOR_B),
                         blinkState(false),
                         lastBlinkTime(0),
                         patternManager(nullptr),
                         polePatternManager(nullptr),
                         globalBrightness(DEFAULT_GLOBAL_BRIGHTNESS),
                         globalSpeed(DEFAULT_GLOBAL_SPEED),
                         selectedPatternIndex(4), // Wave pattern (index 4)
                         selectedPaletteIndex(4), // Party palette (index 4)
                         poleBrightness(DEFAULT_POLE_BRIGHTNESS),
                         poleSpeed(DEFAULT_POLE_SPEED),
                         selectedPolePatternIndex(4), // Bounce pattern (default)
                         selectedPolePaletteIndex(0), // First pole palette
                         selectedJoltPaletteIndex(0), // First palette for jolt mode
                         currentMainMode(MAIN_MODE_EXPLORER),
                         currentSubMode(EXPLORER_SUBMODE_CLOCK_PATTERN),
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
                         buttonHeldDown(false),
                         buttonPressStartTime(0),
                         holdActionTriggered(false),
                         eyeRenderer(nullptr),
                         activeFireworkCount(0),
                         inFireworkMode(false),
                         fireworkModeStartTime(0),
                         lastJoystickUpState(false)
{
  // Initialize firework array
  for (int i = 0; i < MAX_ACTIVE_FIREWORKS; i++)
  {
    activeFireworks[i] = nullptr;
  }

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

  // Clean up pole pattern manager
  delete polePatternManager;

  // Clean up eye renderer
  delete eyeRenderer;
}

bool LEDDriver::initialize()
{
  Serial.println("Initializing LED Driver...");

  // Initialize FastLED for main LEDs (Eye + Clock)
  FastLED.addLeds<LED_TYPE, DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS);

  // Initialize FastLED for pole LEDs
  FastLED.addLeds<POLE_LED_TYPE, POLE_DATA_PIN, POLE_COLOR_ORDER>(poleLeds, POLE_NUM_LEDS);

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
  patternManager = new PatternManager(leds, NUM_LEDS, &segmentManager);
  patternManager->initialize();

  // Initialize pole pattern manager
  polePatternManager = new PolePatternManager(leds, NUM_LEDS, poleLeds, POLE_NUM_LEDS);
  polePatternManager->initialize();

  // Set default palette for pole patterns
  ColorPalette *defaultPalette = patternManager->getPaletteManager().getPalette(selectedPolePaletteIndex);
  if (defaultPalette)
  {
    polePatternManager->setPalette(defaultPalette);
  }

  // Initialize eye renderer for pointer mode
  eyeRenderer = new EyeRenderer(leds, &segmentManager);

  // Set eye colors - blue/green iris like in your image
  eyeRenderer->setEyeColors(CRGB::Cyan, CRGB(5, 5, 5)); // Cyan iris, very dim background

  Serial.print("LED Driver initialized with ");
  Serial.print(NUM_LEDS);
  Serial.println(" LEDs");

#if ENABLE_SEGMENT_DEBUG
  // Print segment configuration for debugging
  segmentManager.printSegmentInfo();
#endif

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
    // Update pole patterns
    updatePole();

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

  // Update button state only when it changes (with debouncing)
  if (buttonState != joystickState.lastButtonState)
  {
    if (currentTime - joystickState.lastButtonChange > BUTTON_DEBOUNCE_MS)
    {
      joystickState.buttonPressed = buttonState;
      joystickState.lastButtonState = buttonState;
      joystickState.lastButtonChange = currentTime;
    }
  }

  // Check for button hold continuously (even when state hasn't changed)
  if (detectButtonHold(buttonState, currentTime))
  {
    // Check if we're in firework mode - hold exits AND cycles mode
    if (inFireworkMode)
    {
      exitFireworkMode();
      // Continue to process normal mode switching to leave firework sub-mode
    }

    // Button hold: cycle main mode
    cycleHoldAction();
  }

  // Single-click: cycle sub-mode (button released after short press)
  // Detect on button release when hold wasn't triggered
  static bool wasPressed = false;
  if (joystickState.buttonPressed)
  {
    wasPressed = true;
  }
  else if (wasPressed && !holdActionTriggered && !inCalibrationMode)
  {
    // If in settings mode, single click returns to previous mode
    if (currentMode == SPECIAL_MODE_SETTINGS)
    {
      currentMode = currentMainMode; // Return to main mode
      Serial.print("Exited Settings Mode, returned to: ");
      Serial.println(getCurrentModeDescription());
    }
    else
    {
      // Check if we're in firework mode - single click exits AND cycles mode
      if (inFireworkMode)
      {
        exitFireworkMode();
        // Continue to process normal mode switching to leave firework sub-mode
      }

      // Normal single-click: cycle sub-mode
      cycleSingleClick();
    }

    wasPressed = false; // Reset for next cycle
  }

  // Process the joystick input based on current mode
  processJoystickInput();
}

/**
 * @brief Analyzes joystick position and returns direction and intensity information
 *
 * This method provides a unified way to detect joystick movement with support for:
 * - Primary direction detection (UP, DOWN, LEFT, RIGHT, or NONE)
 * - Intensity detection (SOFT push vs HARD push)
 * - Magnitude and normalized values for granular control
 *
 * Example usage:
 *
 *
 *    JoystickDirectionInfo dir = getJoystickDirection();
 *
 *   // Check for soft left push
 *   if (dir.direction == DIR_LEFT && dir.intensity == INTENSITY_SOFT) {
 *     // Handle soft left action
 *   }
 *
 *   // Check for hard right push
 *   if (dir.direction == DIR_RIGHT && dir.intensity == INTENSITY_HARD) {
 *     // Handle hard right action (e.g., skip multiple items)
 *   }
 *
 *   // Use normalized value for smooth transitions
 *   if (dir.direction == DIR_UP) {
 *     float value = dir.normalizedValue; // 0.0 to 1.0
 *     // Map to speed, brightness, etc.
 *   }
 *
 * @return JoystickDirectionInfo structure containing direction, intensity, magnitude, and normalized value
 */
LEDDriver::JoystickDirectionInfo LEDDriver::getJoystickDirection()
{
  JoystickDirectionInfo info;
  info.direction = DIR_NONE;
  info.intensity = INTENSITY_NONE;
  info.magnitude = 0;
  info.normalizedValue = 0.0f;

  // Calculate deltas from center
  int deltaX = joystickState.x - JOYSTICK_CENTER;
  int deltaY = joystickState.y - JOYSTICK_CENTER;

  // Determine primary direction based on which axis has greater magnitude
  int absDeltaX = abs(deltaX);
  int absDeltaY = abs(deltaY);

  // Check if we're in deadzone
  if (absDeltaX < JOYSTICK_DEADZONE && absDeltaY < JOYSTICK_DEADZONE)
  {
    return info; // Return with DIR_NONE and INTENSITY_NONE
  }

  // Determine primary direction
  if (absDeltaX > absDeltaY)
  {
    // Horizontal movement is dominant
    if (deltaX > 0)
    {
      info.direction = DIR_RIGHT;
      info.magnitude = deltaX;
    }
    else
    {
      info.direction = DIR_LEFT;
      info.magnitude = absDeltaX;
    }
  }
  else
  {
    // Vertical movement is dominant
    if (deltaY > 0)
    {
      info.direction = DIR_UP;
      info.magnitude = deltaY;
    }
    else
    {
      info.direction = DIR_DOWN;
      info.magnitude = absDeltaY;
    }
  }

  // Calculate normalized value (0.0 to 1.0 from deadzone to edge)
  int maxRange = JOYSTICK_MAX - JOYSTICK_CENTER;     // Distance from center to edge
  int effectiveRange = maxRange - JOYSTICK_DEADZONE; // Range excluding deadzone
  int magnitudeFromDeadzone = info.magnitude - JOYSTICK_DEADZONE;

  if (magnitudeFromDeadzone > 0)
  {
    info.normalizedValue = constrain((float)magnitudeFromDeadzone / (float)effectiveRange, 0.0f, 1.0f);
  }

  // Determine intensity (soft vs hard push)
  // Hard push: within JOYSTICK_HARD_PUSH_MARGIN of the edge
  int maxDistance = JOYSTICK_MAX - JOYSTICK_CENTER; // Maximum possible distance from center
  int distanceFromEdge = maxDistance - info.magnitude;

  if (distanceFromEdge <= JOYSTICK_HARD_PUSH_MARGIN)
  {
    info.intensity = INTENSITY_HARD;
  }
  else if (info.magnitude > JOYSTICK_DEADZONE)
  {
    info.intensity = INTENSITY_SOFT;
  }

  return info;
}

void LEDDriver::processJoystickInput()
{
  // Handle special modes first
  if (inCalibrationMode)
  {
    processCalibrationMode();
    return;
  }

  // Check if we're in a special settings mode
  if (currentMode == SPECIAL_MODE_SETTINGS)
  {
    processSettingsMode();
    return;
  }

  // Process current mode based on main mode and sub-mode
  processCurrentMode();
}

void LEDDriver::processCurrentMode()
{
  switch (currentMainMode)
  {
  case MAIN_MODE_EXPLORER:
    // Explorer Mode: Color/Pattern exploration
    processExplorerMode();
    break;

  case MAIN_MODE_INTERACTION:
    // Interaction Mode: Interactive effects
    processInteractionMode();
    break;

  default:
    // Fallback to Explorer mode
    Serial.println("Unknown main mode, falling back to Explorer");
    currentMainMode = MAIN_MODE_EXPLORER;
    currentSubMode = EXPLORER_SUBMODE_CLOCK_PATTERN;
    processExplorerMode();
    break;
  }
}

void LEDDriver::processExplorerMode()
{
  switch (currentSubMode)
  {
  case EXPLORER_SUBMODE_CLOCK_PATTERN:
    // Clock Pattern Explorer - joystick controls clock patterns/palettes
    processClockPatternExplorer();
    break;

  case EXPLORER_SUBMODE_CLOCK_SETTINGS:
    // Clock Settings - joystick controls clock brightness/speed
    processClockSettings();
    break;

  case EXPLORER_SUBMODE_POLE_PATTERN:
    // Pole Pattern Explorer - joystick controls pole patterns/palettes
    processPolePatternExplorer();
    break;

  case EXPLORER_SUBMODE_POLE_SETTINGS:
    // Pole Settings - joystick controls pole brightness/speed
    processPoleSettings();
    break;

  default:
    // Fallback to clock pattern explorer
    Serial.println("Unknown explorer sub-mode, falling back to Clock Pattern");
    currentSubMode = EXPLORER_SUBMODE_CLOCK_PATTERN;
    processClockPatternExplorer();
    break;
  }
}

void LEDDriver::processInteractionMode()
{
  switch (currentSubMode)
  {
  case INTERACTION_SUBMODE_EYEBALL:
    // Eyeball Mode - eye tracking when joystick active
    processEyeballMode();
    break;

  case INTERACTION_SUBMODE_FIREWORK:
    // Firework Mode - joystick up launches fireworks
    processFireworkMode();
    break;

  case INTERACTION_SUBMODE_JOLT:
    // Jolt Mode - magnitude-based rainbow expansion
    processJoltMode();
    break;

  case INTERACTION_SUBMODE_SPEED_CTRL:
    // Speed Control Mode - adjust global speed with joystick
    processSpeedControlMode();
    break;

  default:
    // Fallback to eyeball mode
    Serial.println("Unknown interaction sub-mode, falling back to Eyeball");
    currentSubMode = INTERACTION_SUBMODE_EYEBALL;
    processEyeballMode();
    break;
  }
}

void LEDDriver::processMainModeOld()
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

void LEDDriver::processEyeModeOld()
{
  // Mode 2: Eye mode - shows patterns on clock, eye tracking when joystick active
  unsigned long currentTime = millis();

  if (patternManager)
  {
    // Set the current pattern and palette based on selected indices
    patternManager->setCurrentPattern(selectedPatternIndex, false);
    patternManager->setCurrentPalette(selectedPaletteIndex);

    // Apply global settings
    patternManager->setGlobalBrightness(globalBrightness);
    patternManager->setGlobalSpeed(globalSpeed);

    // Update pattern manager - this will update the LEDs with patterns
    if (patternManager->update(currentTime))
    {
      // Check if joystick is outside deadzone
      bool joystickActive = (abs(joystickState.x - JOYSTICK_CENTER) > JOYSTICK_DEADZONE ||
                             abs(joystickState.y - JOYSTICK_CENTER) > JOYSTICK_DEADZONE);

      if (joystickActive && eyeRenderer)
      {
        // Clear eye segments with black background
        for (int i = 0; i < 5; i++)
        {
          segmentManager.fillSegment(leds, i + SEGMENT_EYE_4, CRGB::Black);
        }

        // Update eye position based on joystick
        eyeRenderer->updateEyePosition(joystickState.x, joystickState.y);

        // Render eye on the black background
        eyeRenderer->renderEye();
      }

      show(); // Show updated LEDs
    }
  }
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
  if (mode < NUM_MAIN_MODES || mode == SPECIAL_MODE_CALIBRATION)
  {
    // Handle calibration mode specially
    if (mode == SPECIAL_MODE_CALIBRATION)
    {
      startCalibrationMode();
      return;
    }

    currentMode = mode;
    // Update the main mode system
    currentMainMode = mode;
    currentSubMode = 0; // Reset to first sub-mode

    Serial.print("Mode manually set to: ");
    Serial.print(getCurrentModeDescription());
    Serial.print(" - Sub-Mode: ");
    Serial.println(getCurrentSubModeDescription());
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

bool LEDDriver::detectButtonHold(bool buttonPressed, unsigned long currentTime)
{
  // Button just pressed - start tracking
  if (buttonPressed && !buttonHeldDown)
  {
    buttonHeldDown = true;
    buttonPressStartTime = currentTime;
    holdActionTriggered = false;
    Serial.println("Button pressed - hold timer started");
    return false;
  }

  // Button is being held down
  if (buttonPressed && buttonHeldDown && !holdActionTriggered)
  {
    unsigned long holdDuration = currentTime - buttonPressStartTime;

    // Check if hold duration threshold reached
    if (holdDuration >= BUTTON_HOLD_DURATION)
    {
      holdActionTriggered = true;
      Serial.print("BUTTON HOLD DETECTED! (");
      Serial.print(holdDuration);
      Serial.println("ms)");
      return true;
    }
  }

  // Button released
  if (!buttonPressed && buttonHeldDown)
  {
    unsigned long holdDuration = currentTime - buttonPressStartTime;

    // Reset hold tracking
    buttonHeldDown = false;
    holdActionTriggered = false;

    // If released after threshold but before action triggered, treat as single click
    if (holdDuration >= BUTTON_HOLD_THRESHOLD && holdDuration < BUTTON_HOLD_DURATION)
    {
      Serial.print("Button released (");
      Serial.print(holdDuration);
      Serial.println("ms) - treated as single click");
    }
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

void LEDDriver::processBrightnessSpeedModeOld()
{
  // Mode 4: Combined brightness/speed mode - up/down brightness, left/right speed
  unsigned long currentTime = millis();

  // Handle joystick Y-axis for brightness control
  int yDiff = joystickState.y - JOYSTICK_CENTER;
  int xDiff = joystickState.x - JOYSTICK_CENTER;

  static int lastBrightnessChange = 0;
  static unsigned long lastBrightnessChangeTime = 0;
  static int lastSpeedChange = 0;
  static unsigned long lastSpeedChangeTime = 0;
  const unsigned long CHANGE_INTERVAL = 200;

  // Handle brightness control (Y-axis)
  if (abs(yDiff) > JOYSTICK_DEADZONE &&
      (currentTime - lastBrightnessChangeTime) > CHANGE_INTERVAL)
  {
    if (yDiff > 0 && lastBrightnessChange <= 0) // Up - increase brightness
    {
      if (globalBrightness < SETTINGS_BRIGHTNESS_MAX)
      {
        globalBrightness = min(SETTINGS_BRIGHTNESS_MAX, globalBrightness + (SETTINGS_BRIGHTNESS_MAX / BRIGHTNESS_LEVELS));
        Serial.print("Brightness increased to: ");
        Serial.println(globalBrightness);
        lastBrightnessChange = 1;
        lastBrightnessChangeTime = currentTime;
      }
    }
    else if (yDiff < 0 && lastBrightnessChange >= 0) // Down - decrease brightness
    {
      if (globalBrightness > SETTINGS_BRIGHTNESS_MIN)
      {
        globalBrightness = max(SETTINGS_BRIGHTNESS_MIN, globalBrightness - (SETTINGS_BRIGHTNESS_MAX / BRIGHTNESS_LEVELS));
        Serial.print("Brightness decreased to: ");
        Serial.println(globalBrightness);
        lastBrightnessChange = -1;
        lastBrightnessChangeTime = currentTime;
      }
    }
  }
  else if (abs(yDiff) <= JOYSTICK_DEADZONE)
  {
    lastBrightnessChange = 0; // Reset when joystick returns to center
  }

  // Handle speed control (X-axis)
  if (abs(xDiff) > JOYSTICK_DEADZONE &&
      (currentTime - lastSpeedChangeTime) > CHANGE_INTERVAL)
  {
    if (xDiff > 0 && lastSpeedChange <= 0) // Right - increase speed
    {
      if (globalSpeed < SETTINGS_SPEED_MAX)
      {
        globalSpeed = min(SETTINGS_SPEED_MAX, globalSpeed + ((SETTINGS_SPEED_MAX - SETTINGS_SPEED_MIN) / SPEED_LEVELS));
        Serial.print("Speed increased to: ");
        Serial.println(globalSpeed);
        lastSpeedChange = 1;
        lastSpeedChangeTime = currentTime;
      }
    }
    else if (xDiff < 0 && lastSpeedChange >= 0) // Left - decrease speed
    {
      if (globalSpeed > SETTINGS_SPEED_MIN)
      {
        globalSpeed = max(SETTINGS_SPEED_MIN, globalSpeed - ((SETTINGS_SPEED_MAX - SETTINGS_SPEED_MIN) / SPEED_LEVELS));
        Serial.print("Speed decreased to: ");
        Serial.println(globalSpeed);
        lastSpeedChange = -1;
        lastSpeedChangeTime = currentTime;
      }
    }
  }
  else if (abs(xDiff) <= JOYSTICK_DEADZONE)
  {
    lastSpeedChange = 0; // Reset when joystick returns to center
  }

  if (patternManager)
  {
    // Set the current pattern and palette based on selected indices (global settings)
    patternManager->setCurrentPattern(selectedPatternIndex, false);
    patternManager->setCurrentPalette(selectedPaletteIndex);

    // Apply current global settings
    patternManager->setGlobalBrightness(globalBrightness);
    patternManager->setGlobalSpeed(globalSpeed);

    // Update pattern manager - this will update the LEDs with the current pattern/palette
    patternManager->update(currentTime);
  }

  // Render combined preview showing both brightness and speed indicators on top of pattern
  uint8_t brightnessLevel = map(globalBrightness, SETTINGS_BRIGHTNESS_MIN, SETTINGS_BRIGHTNESS_MAX, 1, BRIGHTNESS_LEVELS);
  uint8_t speedLevel = map(globalSpeed * 10, SETTINGS_SPEED_MIN * 10, SETTINGS_SPEED_MAX * 10, 1, SPEED_LEVELS);
  renderCombinedPreview(brightnessLevel, speedLevel);

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

void LEDDriver::processPatternModeOld()
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

  // Return to pattern mode
  currentMode = currentMainMode; // Return to current main mode
  settingsPhase = PHASE_1_QUADRANTS;
  stopHolding();
  currentQuadrant = -1;
  previewedItem = -1;
  itemPreviewed = false;
  stickyPointerPosition = -1;
  hasStickyPointer = false;

  Serial.println("Returning to Pattern/Palette Explore Mode");
}

SegmentManager &LEDDriver::getSegmentManager()
{
  return segmentManager;
}

void LEDDriver::renderBrightnessPreview(uint8_t level)
{
  // Render brightness level on vertical column LEDs in eye rings
  // Level 1 = only bottom LED (6 o'clock), Level 9 = all LEDs to top (12 o'clock)

  CRGB previewColor = CRGB::White;

  for (uint8_t i = 0; i < min((int)level, BRIGHTNESS_PREVIEW_LEDS); i++)
  {
    // Access pre-computed raw LED index directly from mapping array
    uint16_t rawIndex = BRIGHTNESS_LED_POSITIONS[i];

    if (rawIndex < NUM_LEDS)
    {
      leds[rawIndex] = previewColor;
    }
  }
}

void LEDDriver::renderSpeedPreview(uint8_t level)
{
  // Render speed level on horizontal line LEDs in eye rings
  // Level 1 = only left LED (9 o'clock), Level 9 = all LEDs to right (3 o'clock)

  CRGB previewColor = CRGB::Blue;

  for (uint8_t i = 0; i < min((int)level, SPEED_PREVIEW_LEDS); i++)
  {
    // Access pre-computed raw LED index directly from mapping array
    uint16_t rawIndex = SPEED_LED_POSITIONS[i];

    if (rawIndex < NUM_LEDS)
    {
      leds[rawIndex] = previewColor;
    }
  }
}

void LEDDriver::renderCombinedPreview(uint8_t brightnessLevel, uint8_t speedLevel)
{
  // Render both brightness (vertical) and speed (horizontal) previews simultaneously
  // Brightness uses white LEDs, speed uses blue LEDs
  // Center LED (EYE_0) will be mixed if both are active

  CRGB brightnessColor = CRGB::White;
  CRGB speedColor = CRGB::Blue;
  CRGB mixedColor = CRGB(128, 128, 255); // Purple-ish mix for center overlap

  // Render brightness preview (vertical)
  for (uint8_t i = 0; i < min((int)brightnessLevel, BRIGHTNESS_PREVIEW_LEDS); i++)
  {
    // Access pre-computed raw LED index directly from mapping array
    uint16_t rawIndex = BRIGHTNESS_LED_POSITIONS[i];

    if (rawIndex < NUM_LEDS)
    {
      leds[rawIndex] = brightnessColor;
    }
  }

  // Render speed preview (horizontal)
  for (uint8_t i = 0; i < min((int)speedLevel, SPEED_PREVIEW_LEDS); i++)
  {
    // Access pre-computed raw LED index directly from mapping array
    uint16_t rawIndex = SPEED_LED_POSITIONS[i];

    if (rawIndex < NUM_LEDS)
    {
      // Check if this LED is already lit by brightness preview
      if (leds[rawIndex] == brightnessColor)
      {
        // Mix colors for overlap (center LED and any other overlaps)
        leds[rawIndex] = mixedColor;
      }
      else
      {
        leds[rawIndex] = speedColor;
      }
    }
  }
}

void LEDDriver::renderWaveEffectOnClock()
{
  // Render a wave effect on the clock ring using rainbow palette
  static unsigned long lastUpdate = 0;
  static float wavePosition = 0.0f;

  unsigned long currentTime = millis();
  if (currentTime - lastUpdate > 50) // Update every 50ms
  {
    wavePosition += globalSpeed * 0.02f; // Speed affects wave movement
    if (wavePosition >= 1.0f)
    {
      wavePosition -= 1.0f;
    }
    lastUpdate = currentTime;
  }

  // Apply wave effect to clock segment
  const LEDSegment *clockSegment = segmentManager.getSegment(SEGMENT_CLOCK);
  if (clockSegment)
  {
    for (uint16_t i = 0; i < clockSegment->count; i++)
    {
      float ledPosition = (float)i / (float)clockSegment->count;
      float wavePhase = ledPosition + wavePosition;
      if (wavePhase > 1.0f)
        wavePhase -= 1.0f;

      // Create rainbow wave
      uint8_t hue = (uint8_t)(wavePhase * 255);
      CHSV hsvColor(hue, 255, globalBrightness);
      CRGB rgbColor;
      hsv2rgb_rainbow(hsvColor, rgbColor);

      uint16_t absoluteIndex = clockSegment->rawStartIndex + i;
      if (absoluteIndex < NUM_LEDS)
      {
        leds[absoluteIndex] = rgbColor;
      }
    }
  }
}

// ============================================================================
// POLE LED METHODS
// ============================================================================

void LEDDriver::setPolePixel(int index, CRGB color)
{
  if (index >= 0 && index < POLE_NUM_LEDS)
  {
    poleLeds[index] = color;
  }
}

void LEDDriver::fillPole(CRGB color)
{
  fill_solid(poleLeds, POLE_NUM_LEDS, color);
}

void LEDDriver::clearPole()
{
  fill_solid(poleLeds, POLE_NUM_LEDS, CRGB::Black);
}

uint8_t LEDDriver::getPoleColumn(uint16_t index)
{
  return index % POLE_SPIRAL_REPEAT;
}

uint8_t LEDDriver::getPoleHeight(uint16_t index)
{
  return index / POLE_SPIRAL_REPEAT;
}

void LEDDriver::updatePole()
{
  if (!polePatternManager)
  {
    return;
  }

  unsigned long currentTime = millis();

  // Update palette if changed
  static int lastPolePaletteIndex = -1;
  if (patternManager && selectedPolePaletteIndex != lastPolePaletteIndex)
  {
    ColorPalette *currentPalette = patternManager->getPaletteManager().getPalette(selectedPolePaletteIndex);
    if (currentPalette)
    {
      polePatternManager->setPalette(currentPalette);
      lastPolePaletteIndex = selectedPolePaletteIndex;
    }
  }

  // Set current pattern
  int patternCount = polePatternManager->getPatternCount();
  if (patternCount > 0)
  {
    uint8_t patternIndex = selectedPolePatternIndex % patternCount;
    polePatternManager->setCurrentPattern(patternIndex);

    // Update brightness and speed
    polePatternManager->setBrightness(poleBrightness);
    polePatternManager->setSpeed(poleSpeed);

    // Update pattern
    polePatternManager->update(currentTime);
  }
}

// ============================================================================
// FIREWORK MODE IMPLEMENTATION
// ============================================================================

void LEDDriver::exitFireworkMode()
{
  inFireworkMode = false;

  // Clean up all active fireworks
  for (int i = 0; i < MAX_ACTIVE_FIREWORKS; i++)
  {
    if (activeFireworks[i] != nullptr)
    {
      delete activeFireworks[i];
      activeFireworks[i] = nullptr;
    }
  }
  activeFireworkCount = 0;

  Serial.println("Exited Firework Mode");
}

void LEDDriver::processFireworkMode()
{
  unsigned long currentTime = millis();

  // Only auto-enter if we're not already in firework mode
  if (!inFireworkMode)
  {
    inFireworkMode = true;
    fireworkModeStartTime = currentTime;
    lastJoystickUpState = false;

    Serial.println("Entered Firework Mode! Move joystick UP to launch fireworks.");
  }

  // Check for timeout
  if (currentTime - fireworkModeStartTime > FIREWORK_MODE_TIMEOUT)
  {
    exitFireworkMode();
    return;
  }

  // Check for joystick up motion to launch firework
  bool joystickUp = (joystickState.y > (JOYSTICK_CENTER + FIREWORK_LAUNCH_THRESHOLD));

  if (joystickUp && !lastJoystickUpState)
  {
    // Rising edge - launch firework
    launchFirework(currentTime);
  }
  lastJoystickUpState = joystickUp;

  // Update active fireworks
  updateActiveFireworks(currentTime);

  // Clean up completed fireworks
  cleanupInactiveFireworks();

  // Show the LEDs
  show();
}

void LEDDriver::launchFirework(unsigned long currentTime)
{
  // Find an empty slot for new firework
  for (int i = 0; i < MAX_ACTIVE_FIREWORKS; i++)
  {
    if (activeFireworks[i] == nullptr)
    {
      // Create new firework action
      activeFireworks[i] = new FireworkAction(leds, NUM_LEDS, poleLeds, POLE_NUM_LEDS);
      activeFireworks[i]->setBrightness(globalBrightness);
      activeFireworks[i]->setSpeed(globalSpeed);
      activeFireworks[i]->trigger(currentTime);

      activeFireworkCount++;

      Serial.print("Launched firework #");
      Serial.print(i);
      Serial.print(" (Active: ");
      Serial.print(activeFireworkCount);
      Serial.println(")");
      break;
    }
  }
}

void LEDDriver::updateActiveFireworks(unsigned long currentTime)
{
  // Clear all LEDs first - fireworks will draw over this
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  if (poleLeds)
  {
    fill_solid(poleLeds, POLE_NUM_LEDS, CRGB::Black);
  }

  // Update all active fireworks
  for (int i = 0; i < MAX_ACTIVE_FIREWORKS; i++)
  {
    if (activeFireworks[i] != nullptr && activeFireworks[i]->getActive())
    {
      activeFireworks[i]->update(currentTime);
    }
  }
}

void LEDDriver::cleanupInactiveFireworks()
{
  for (int i = 0; i < MAX_ACTIVE_FIREWORKS; i++)
  {
    if (activeFireworks[i] != nullptr && activeFireworks[i]->getComplete())
    {
      delete activeFireworks[i];
      activeFireworks[i] = nullptr;
      activeFireworkCount--;

      Serial.print("Cleaned up completed firework #");
      Serial.print(i);
      Serial.print(" (Active: ");
      Serial.print(activeFireworkCount);
      Serial.println(")");
    }
  }
}

// ============================================================================
// MODE SWITCHING IMPLEMENTATION
// ============================================================================

void LEDDriver::cycleSingleClick()
{
  // Single click cycles sub-modes within current main mode
  if (currentMainMode == MAIN_MODE_EXPLORER)
  {
    // If leaving pole pattern explorer, return to auto mode
    if (currentSubMode == EXPLORER_SUBMODE_POLE_PATTERN)
    {
      // No auto-cycling - pole patterns are always manually controlled
    }

    // Cycle through Explorer sub-modes
    currentSubMode = (currentSubMode + 1) % NUM_EXPLORER_SUBMODES;

    Serial.print("Single Click - Cycled to Explorer Sub-Mode: ");
    Serial.println(getCurrentSubModeDescription());
  }
  else if (currentMainMode == MAIN_MODE_INTERACTION)
  {
    // If leaving firework mode, exit firework state
    if (currentSubMode == INTERACTION_SUBMODE_FIREWORK && inFireworkMode)
    {
      exitFireworkMode();
    }

    // Cycle through Interaction sub-modes
    currentSubMode = (currentSubMode + 1) % NUM_INTERACTION_SUBMODES;

    Serial.print("Single Click - Cycled to Interaction Sub-Mode: ");
    Serial.println(getCurrentSubModeDescription());
  }
}

void LEDDriver::cycleHoldAction()
{
  // If leaving pole pattern explorer, return to auto mode
  if (currentMainMode == MAIN_MODE_EXPLORER && currentSubMode == EXPLORER_SUBMODE_POLE_PATTERN)
  {
    // No auto-cycling - pole patterns are always manually controlled
  }

  // If leaving firework mode, exit firework state
  if (currentMainMode == MAIN_MODE_INTERACTION && currentSubMode == INTERACTION_SUBMODE_FIREWORK && inFireworkMode)
  {
    exitFireworkMode();
  }

  // Button hold cycles main modes
  currentMainMode = (currentMainMode + 1) % NUM_MAIN_MODES;
  currentSubMode = 0; // Reset to first sub-mode of new main mode

  Serial.print("Button Hold - Cycled to Main Mode: ");
  Serial.print(getCurrentModeDescription());
  Serial.print(" - Sub-Mode: ");
  Serial.println(getCurrentSubModeDescription());
}

String LEDDriver::getCurrentModeDescription() const
{
  switch (currentMainMode)
  {
  case MAIN_MODE_EXPLORER:
    return "Explorer Mode (Color/Pattern)";
  case MAIN_MODE_INTERACTION:
    return "Interaction Mode (Effects)";
  default:
    return "Unknown Mode";
  }
}

String LEDDriver::getCurrentSubModeDescription() const
{
  if (currentMainMode == MAIN_MODE_EXPLORER)
  {
    switch (currentSubMode)
    {
    case EXPLORER_SUBMODE_CLOCK_PATTERN:
      return "Clock Pattern Explorer";
    case EXPLORER_SUBMODE_CLOCK_SETTINGS:
      return "Clock Brightness/Speed";
    case EXPLORER_SUBMODE_POLE_PATTERN:
      return "Pole Pattern Explorer";
    case EXPLORER_SUBMODE_POLE_SETTINGS:
      return "Pole Brightness/Speed";
    default:
      return "Unknown Explorer Sub-Mode";
    }
  }
  else if (currentMainMode == MAIN_MODE_INTERACTION)
  {
    switch (currentSubMode)
    {
    case INTERACTION_SUBMODE_EYEBALL:
      return "Eyeball Tracking";
    case INTERACTION_SUBMODE_FIREWORK:
      return "Firework Launch";
    case INTERACTION_SUBMODE_JOLT:
      return "Jolt Magnitude";
    case INTERACTION_SUBMODE_SPEED_CTRL:
      return "Speed Control";
    default:
      return "Unknown Interaction Sub-Mode";
    }
  }
  return "Unknown Sub-Mode";
}

// ============================================================================
// POLE PATTERN CONTROL IMPLEMENTATION
// ============================================================================

void LEDDriver::setPolePatternIndex(int patternIndex)
{
  if (!polePatternManager)
  {
    return;
  }

  int patternCount = polePatternManager->getPatternCount();
  if (patternCount > 0)
  {
    selectedPolePatternIndex = patternIndex % patternCount;

    Serial.print("Pole pattern set to: ");
    Serial.println(polePatternManager->getPatternName(selectedPolePatternIndex));
  }
}

void LEDDriver::updatePolePatternSelection()
{
  // This method can be called to apply pole pattern changes
  // The actual pattern switching happens in updatePole()
}

// ============================================================================
// EXPLORER SUB-MODE IMPLEMENTATIONS
// ============================================================================

void LEDDriver::processClockPatternExplorer()
{
  // Same as the old pattern mode - joystick controls clock patterns/palettes
  processPatternModeOld(); // Reuse existing implementation
}

void LEDDriver::processClockSettings()
{
  // Same as the old brightness/speed mode for clock
  processBrightnessSpeedModeOld(); // Reuse existing implementation
}

void LEDDriver::processPolePatternExplorer()
{
  // Pole patterns are always manually controlled

  // Handle joystick input for pole pattern selection
  unsigned long currentTime = millis();

  // Get joystick input
  int deltaX = joystickState.x - JOYSTICK_CENTER;
  int deltaY = joystickState.y - JOYSTICK_CENTER;

  // Static variables for timing and previous position
  static unsigned long lastPatternChange = 0;
  static int lastDeltaX = 0;
  static int lastDeltaY = 0;
  static const unsigned long PATTERN_CHANGE_DELAY = 200; // 200ms delay between changes

  // Check if joystick is outside deadzone
  bool joystickMoved = (abs(deltaX) > JOYSTICK_DEADZONE || abs(deltaY) > JOYSTICK_DEADZONE);
  bool canChange = (currentTime - lastPatternChange) > PATTERN_CHANGE_DELAY;

  if (joystickMoved && canChange)
  {
    // Left/Right controls pole pattern (5 pole patterns available)
    if (abs(deltaX) > JOYSTICK_DEADZONE)
    {
      if (deltaX > 0 && lastDeltaX <= JOYSTICK_DEADZONE) // Right
      {
        if (polePatternManager)
        {
          int patternCount = polePatternManager->getPatternCount();
          selectedPolePatternIndex = (selectedPolePatternIndex + 1) % patternCount;
          lastPatternChange = currentTime;

          Serial.print("Pole Pattern: ");
          Serial.println(polePatternManager->getPatternName(selectedPolePatternIndex));
        }
      }
      else if (deltaX < 0 && lastDeltaX >= -JOYSTICK_DEADZONE) // Left
      {
        if (polePatternManager)
        {
          int patternCount = polePatternManager->getPatternCount();
          selectedPolePatternIndex = (selectedPolePatternIndex + patternCount - 1) % patternCount;
          lastPatternChange = currentTime;

          Serial.print("Pole Pattern: ");
          Serial.println(polePatternManager->getPatternName(selectedPolePatternIndex));
        }
      }
    }

    // Up/Down controls pole palette
    if (abs(deltaY) > JOYSTICK_DEADZONE && patternManager)
    {
      if (deltaY > 0 && lastDeltaY <= JOYSTICK_DEADZONE) // Up - next palette
      {
        selectedPolePaletteIndex = (selectedPolePaletteIndex + 1) % patternManager->getPaletteManager().getPaletteCount();
        lastPatternChange = currentTime;

        // Update all pole patterns with new palette
        ColorPalette *newPalette = patternManager->getPaletteManager().getPalette(selectedPolePaletteIndex);
        if (newPalette)
        {
          Serial.print("Pole Palette: ");
          Serial.println(newPalette->getName());
        }
      }
      else if (deltaY < 0 && lastDeltaY >= -JOYSTICK_DEADZONE) // Down - previous palette
      {
        int paletteCount = patternManager->getPaletteManager().getPaletteCount();
        selectedPolePaletteIndex = (selectedPolePaletteIndex + paletteCount - 1) % paletteCount;
        lastPatternChange = currentTime;

        // Update all pole patterns with new palette
        ColorPalette *newPalette = patternManager->getPaletteManager().getPalette(selectedPolePaletteIndex);
        if (newPalette)
        {
          Serial.print("Pole Palette: ");
          Serial.println(newPalette->getName());
        }
      }
    }
  }

  // Store previous joystick position
  lastDeltaX = deltaX;
  lastDeltaY = deltaY;

  // Clear main LEDs (eye/clock) so only pole shows
  fill_solid(leds, NUM_LEDS, CRGB::Black);

  // The pole pattern will be updated by the regular updatePole() call
  // which now uses selectedPolePatternIndex (always manual control)

  show();
}

void LEDDriver::processPoleSettings()
{
  // Handle pole brightness and speed adjustment using discrete levels (like clock settings)
  unsigned long currentTime = millis();

  // Get joystick input
  int yDiff = joystickState.y - JOYSTICK_CENTER;
  int xDiff = joystickState.x - JOYSTICK_CENTER;

  static int lastPoleBrightnessChange = 0;
  static unsigned long lastPoleBrightnessChangeTime = 0;
  static int lastPoleSpeedChange = 0;
  static unsigned long lastPoleSpeedChangeTime = 0;
  const unsigned long CHANGE_INTERVAL = 200;

  // Handle pole brightness control (Y-axis) - discrete levels
  if (abs(yDiff) > JOYSTICK_DEADZONE &&
      (currentTime - lastPoleBrightnessChangeTime) > CHANGE_INTERVAL)
  {
    if (yDiff > 0 && lastPoleBrightnessChange <= 0) // Up - increase brightness
    {
      if (poleBrightness < POLE_BRIGHTNESS_MAX)
      {
        poleBrightness = min(POLE_BRIGHTNESS_MAX, poleBrightness + ((POLE_BRIGHTNESS_MAX - POLE_BRIGHTNESS_MIN) / BRIGHTNESS_LEVELS));
        Serial.print("Pole brightness increased to: ");
        Serial.println(poleBrightness);
        lastPoleBrightnessChange = 1;
        lastPoleBrightnessChangeTime = currentTime;
      }
    }
    else if (yDiff < 0 && lastPoleBrightnessChange >= 0) // Down - decrease brightness
    {
      if (poleBrightness > POLE_BRIGHTNESS_MIN)
      {
        poleBrightness = max(POLE_BRIGHTNESS_MIN, poleBrightness - ((POLE_BRIGHTNESS_MAX - POLE_BRIGHTNESS_MIN) / BRIGHTNESS_LEVELS));
        Serial.print("Pole brightness decreased to: ");
        Serial.println(poleBrightness);
        lastPoleBrightnessChange = -1;
        lastPoleBrightnessChangeTime = currentTime;
      }
    }
  }
  else if (abs(yDiff) <= JOYSTICK_DEADZONE)
  {
    lastPoleBrightnessChange = 0; // Reset when joystick returns to center
  }

  // Handle pole speed control (X-axis) - discrete levels
  if (abs(xDiff) > JOYSTICK_DEADZONE &&
      (currentTime - lastPoleSpeedChangeTime) > CHANGE_INTERVAL)
  {
    if (xDiff > 0 && lastPoleSpeedChange <= 0) // Right - increase speed
    {
      if (poleSpeed < POLE_SPEED_MAX)
      {
        poleSpeed = min(POLE_SPEED_MAX, poleSpeed + ((POLE_SPEED_MAX - POLE_SPEED_MIN) / SPEED_LEVELS));
        Serial.print("Pole speed increased to: ");
        Serial.println(poleSpeed);
        lastPoleSpeedChange = 1;
        lastPoleSpeedChangeTime = currentTime;
      }
    }
    else if (xDiff < 0 && lastPoleSpeedChange >= 0) // Left - decrease speed
    {
      if (poleSpeed > POLE_SPEED_MIN)
      {
        poleSpeed = max(POLE_SPEED_MIN, poleSpeed - ((POLE_SPEED_MAX - POLE_SPEED_MIN) / SPEED_LEVELS));
        Serial.print("Pole speed decreased to: ");
        Serial.println(poleSpeed);
        lastPoleSpeedChange = -1;
        lastPoleSpeedChangeTime = currentTime;
      }
    }
  }
  else if (abs(xDiff) <= JOYSTICK_DEADZONE)
  {
    lastPoleSpeedChange = 0; // Reset when joystick returns to center
  }

  // Display pole settings preview
  // Clear main LEDs
  fill_solid(leds, NUM_LEDS, CRGB::Black);

  // Show pole brightness and speed using combined preview (like clock settings)
  uint8_t brightnessLevel = map(poleBrightness, POLE_BRIGHTNESS_MIN, POLE_BRIGHTNESS_MAX, 1, BRIGHTNESS_LEVELS);
  uint8_t speedLevel = map(poleSpeed * 10, POLE_SPEED_MIN * 10, POLE_SPEED_MAX * 10, 1, SPEED_LEVELS);
  renderCombinedPreview(brightnessLevel, speedLevel);

  show();
}

// ============================================================================
// INTERACTION SUB-MODE IMPLEMENTATIONS
// ============================================================================

void LEDDriver::processEyeballMode()
{
  // Eye Mode: Always show eyeball + red directional line on clock
  // - Top 7 LEDs of pole show global pattern
  // - Drips fall along diagonal "stick" paths (e.g., LED 0->7->13->20...)
  // - Clock shows red line pointing in joystick direction
  // - Eye always rendered

  unsigned long currentTime = millis();

  // Drip system configuration
  static const uint8_t MAX_DRIPS = 40;   // Increased from 20 to 40 for more consistent dripping
  static const uint8_t DRIP_HEIGHT = 3;  // 3 rows tall
  static const uint8_t POLE_HEIGHT = 23; // Total pole height in rows (300 LEDs / 13 columns = ~23 rows)
  static const uint8_t POLE_COLUMNS = 13;
  static const float DRIP_SPEED = 1.25f;             // Drip fall speed (rows per frame)
  static const unsigned long DRIP_SPAWN_DELAY = 100; // ms between drip spawns (reduced from 200ms to 100ms for 2x speed)

  // Pole column geometry
  // Physical structure: Every 13 LEDs forms a vertical column
  // LED 0 is at BOTTOM, LED 299 is at TOP
  // Column N consists of LEDs: N, N+13, N+26, N+39, ...

  // Drip structure
  struct Drip
  {
    bool active;
    uint8_t column;     // Column (0-12)
    float height;       // Current height in rows (float for smooth movement, decreases as it falls)
    uint8_t colorIndex; // Index into palette for color progression
  };

  static Drip drips[MAX_DRIPS] = {0};
  static unsigned long lastDripSpawn = 0;

  // Clear pole LEDs
  fill_solid(poleLeds, POLE_NUM_LEDS, CRGB::Black);

  // 1. Render top 7 LEDs (last 7 indices) with RED color consistently
  for (uint8_t i = 0; i < 7; i++)
  {
    uint16_t poleIndex = POLE_NUM_LEDS - 7 + i;
    poleLeds[poleIndex] = CRGB::Red;
  }

  // Calculate joystick deltas
  int deltaX = joystickState.x - JOYSTICK_CENTER;
  int deltaY = joystickState.y - JOYSTICK_CENTER;

  // Check if joystick is active (outside deadzone)
  bool joystickActive = (abs(deltaX) > JOYSTICK_DEADZONE || abs(deltaY) > JOYSTICK_DEADZONE);

  uint8_t targetColumn = POLE_COLUMNS / 2; // Default to center

  if (joystickActive)
  {
    // Calculate angle from joystick position (in degrees, 0 = North/12 o'clock)
    float angleRadians = atan2(deltaX, deltaY);
    float angleDegrees = angleRadians * 180.0f / PI;
    if (angleDegrees < 0)
      angleDegrees += 360.0f;

    // Map angle to column (0-12)
    // 0° (North) = center column (6)
    // Smooth interpolation around 360°
    targetColumn = (uint8_t)((angleDegrees / 360.0f) * POLE_COLUMNS) % POLE_COLUMNS;

    // Spawn new drip if enough time has passed
    if (currentTime - lastDripSpawn >= DRIP_SPAWN_DELAY)
    {
      // Find an inactive drip slot
      for (uint8_t i = 0; i < MAX_DRIPS; i++)
      {
        if (!drips[i].active)
        {
          drips[i].active = true;
          drips[i].column = targetColumn;
          drips[i].height = (float)(POLE_HEIGHT - 1); // Start at top row (just below red LEDs)
          drips[i].colorIndex = 0;                    // Start with first color

          lastDripSpawn = currentTime;
          break;
        }
      }
    }
  }

  // 2. Update and render all active drips
  // Get current palette for drip colors
  ColorPalette *currentPalette = nullptr;
  if (patternManager)
  {
    currentPalette = patternManager->getPaletteManager().getPalette(selectedPaletteIndex);
  }

  for (uint8_t i = 0; i < MAX_DRIPS; i++)
  {
    if (drips[i].active)
    {
      // Move drip DOWN (decreasing height)
      drips[i].height -= DRIP_SPEED * globalSpeed;

      // Check if drip has fallen off bottom
      if (drips[i].height < -DRIP_HEIGHT)
      {
        drips[i].active = false;
        continue;
      }

      // Render 3-row drip with fade in/out
      for (uint8_t row = 0; row < DRIP_HEIGHT; row++)
      {
        int dripRow = (int)drips[i].height - row;

        if (dripRow >= 0 && dripRow < POLE_HEIGHT)
        {
          // Calculate LED index: column + (row * 13)
          uint16_t poleIndex = drips[i].column + (dripRow * POLE_SPIRAL_REPEAT);

          if (poleIndex < POLE_NUM_LEDS - 7) // Don't overwrite top 7 red LEDs
          {
            // Calculate fade factor based on position in drip
            uint8_t fadeFactor;
            if (row == 0) // Top of drip (head)
            {
              fadeFactor = 255;
            }
            else if (row == 1) // Middle
            {
              fadeFactor = 180;
            }
            else // Bottom (tail)
            {
              fadeFactor = 100;
            }

            // Apply fade-in/out based on drip age
            float ageInRows = (POLE_HEIGHT - 1) - drips[i].height;
            if (ageInRows < 1.0f) // Fade in during first row
            {
              fadeFactor = (uint8_t)(fadeFactor * ageInRows);
            }
            else if (drips[i].height < 1.0f) // Fade out during last row
            {
              fadeFactor = (uint8_t)(fadeFactor * (drips[i].height + DRIP_HEIGHT));
            }

            // Get color from palette (cycling through palette as drip falls)
            CRGB dripColor;
            if (currentPalette)
            {
              uint8_t paletteIndex = (drips[i].colorIndex + (row * 20)) % 256;
              dripColor = currentPalette->getColor(paletteIndex);
            }
            else
            {
              dripColor = CRGB::White;
            }

            dripColor.nscale8(fadeFactor);
            poleLeds[poleIndex] = dripColor;
          }
        }
      }

      // Update color index for next frame (slow color progression)
      drips[i].colorIndex = (drips[i].colorIndex + 1) % 256;
    }
  }

  // 3. Clear clock and eye segments
  segmentManager.fillSegment(leds, SEGMENT_CLOCK, CRGB::Black);
  for (int i = 0; i < 5; i++)
  {
    segmentManager.fillSegment(leds, i + SEGMENT_EYE_4, CRGB::Black);
  }

  // 4. Draw red line on clock
  if (joystickActive)
  {
    float angleRadians = atan2(deltaX, deltaY);
    float angleDegrees = angleRadians * 180.0f / PI;
    if (angleDegrees < 0)
      angleDegrees += 360.0f;

    int magnitude = sqrt(deltaX * deltaX + deltaY * deltaY);
    int maxMagnitude = JOYSTICK_MAX - JOYSTICK_CENTER;
    int lineWidth = map(constrain(magnitude, JOYSTICK_DEADZONE, maxMagnitude),
                        JOYSTICK_DEADZONE, maxMagnitude, 1, 20);

    uint16_t rawIndices[20];
    uint8_t count = segmentManager.getRawLEDsAtAngle(SEGMENT_CLOCK, angleDegrees,
                                                     lineWidth, rawIndices, 20);

    for (uint8_t i = 0; i < count; i++)
    {
      leds[rawIndices[i]] = CRGB::Red;
      leds[rawIndices[i]].nscale8(globalBrightness);
    }
  }

  // 5. Render eye
  if (eyeRenderer)
  {
    if (joystickActive)
    {
      eyeRenderer->updateEyePosition(joystickState.x, joystickState.y);
    }
    else
    {
      eyeRenderer->updateEyePosition(JOYSTICK_CENTER, JOYSTICK_CENTER);
    }
    eyeRenderer->renderEye();
  }

  show();
}

void LEDDriver::processJoltMode()
{
  // Jolt Mode: Magnitude-based palette expansion
  // Up: Expands from center outward
  // Down: Fills from edges inward
  // Deadzone: Shows center row
  unsigned long currentTime = millis();

  // Handle palette selection with left/right joystick movement
  int deltaX = joystickState.x - JOYSTICK_CENTER;

  // Static variables for timing and previous position
  static unsigned long lastPaletteChange = 0;
  static int lastDeltaX = 0;
  static const unsigned long PALETTE_CHANGE_DELAY = 300; // 300ms delay between changes

  // Check if joystick moved horizontally and enough time has passed
  bool canChangePalette = (currentTime - lastPaletteChange) > PALETTE_CHANGE_DELAY;

  if (abs(deltaX) > JOYSTICK_DEADZONE && canChangePalette && patternManager)
  {
    if (deltaX > 0 && lastDeltaX <= JOYSTICK_DEADZONE) // Right - next palette
    {
      selectedJoltPaletteIndex = (selectedJoltPaletteIndex + 1) % patternManager->getPaletteManager().getPaletteCount();
      lastPaletteChange = currentTime;

      ColorPalette *newPalette = patternManager->getPaletteManager().getPalette(selectedJoltPaletteIndex);
      if (newPalette)
      {
        Serial.print("Jolt Palette: ");
        Serial.println(newPalette->getName());
      }
    }
    else if (deltaX < 0 && lastDeltaX >= -JOYSTICK_DEADZONE) // Left - previous palette
    {
      int paletteCount = patternManager->getPaletteManager().getPaletteCount();
      selectedJoltPaletteIndex = (selectedJoltPaletteIndex + paletteCount - 1) % paletteCount;
      lastPaletteChange = currentTime;

      ColorPalette *newPalette = patternManager->getPaletteManager().getPalette(selectedJoltPaletteIndex);
      if (newPalette)
      {
        Serial.print("Jolt Palette: ");
        Serial.println(newPalette->getName());
      }
    }
  }

  // Store previous joystick X position
  lastDeltaX = deltaX;

  // Get joystick Y delta
  int deltaY = joystickState.y - JOYSTICK_CENTER;

  // Determine jolt direction and magnitude
  if (abs(deltaY) <= JOLT_DEADZONE_THRESHOLD)
  {
    // Deadzone: Show center row with pattern
    renderJoltEffectDeadzone();
  }
  else if (deltaY > 0)
  {
    // Moving UP: Original behavior (expand from center outward)
    uint8_t magnitude = calculateJoltMagnitude(joystickState.y);
    renderJoltEffectOutward(magnitude);
  }
  else
  {
    // Moving DOWN: New behavior (fill from edges inward)
    uint8_t magnitude = calculateJoltMagnitudeDown(joystickState.y);
    renderJoltEffectInward(magnitude);
  }

  show();
}

void LEDDriver::processSpeedControlMode()
{
  // Speed Control Mode: Display current pattern and adjust speed with joystick

  // Display the current pattern with current settings
  if (patternManager)
  {
    // Update pattern with current global settings
    Pattern *currentPattern = patternManager->getCurrentPattern();
    if (currentPattern)
    {
      currentPattern->setSpeed(globalSpeed);
      currentPattern->setBrightness(globalBrightness);
      patternManager->update(millis());
    }
  }

  // Handle vertical joystick for speed adjustment and palette switching
  int deltaY = joystickState.y - JOYSTICK_CENTER;

  // Handle horizontal joystick for pattern switching
  int deltaX = joystickState.x - JOYSTICK_CENTER;

  // Static variables for smooth adjustments
  static unsigned long lastSpeedChange = 0;
  static unsigned long lastPatternChange = 0;
  static unsigned long lastPaletteChange = 0;
  static const unsigned long SPEED_CHANGE_DELAY = 100;   // Update speed every 100ms
  static const unsigned long PATTERN_CHANGE_DELAY = 300; // Prevent rapid pattern switching
  static const unsigned long PALETTE_CHANGE_DELAY = 300; // Prevent rapid palette switching
  static int lastDeltaX = 0;
  static int lastDeltaY = 0;
  unsigned long currentTime = millis();

  bool canChangeSpeed = (currentTime - lastSpeedChange) > SPEED_CHANGE_DELAY;
  bool canChangePattern = (currentTime - lastPatternChange) > PATTERN_CHANGE_DELAY;
  bool canChangePalette = (currentTime - lastPaletteChange) > PALETTE_CHANGE_DELAY;

  // Speed adjustment (vertical up)
  if (canChangeSpeed)
  {
    // Map joystick Y to speed range
    // Up (positive deltaY): SPEED_CONTROL_MIN_SPEED to SPEED_CONTROL_MAX_SPEED
    // Deadzone: SPEED_CONTROL_DEADZONE_SPEED
    // Down is used for palette switching

    if (abs(deltaY) < JOYSTICK_DEADZONE)
    {
      // Deadzone: Set speed to configured deadzone speed
      globalSpeed = SPEED_CONTROL_DEADZONE_SPEED;
      poleSpeed = SPEED_CONTROL_DEADZONE_SPEED;
    }
    else if (deltaY > 0)
    {
      // Moving up: Map to MIN_SPEED - MAX_SPEED
      // Use constrain to ensure we stay within joystick bounds
      int clampedDelta = constrain(deltaY, JOYSTICK_DEADZONE, JOYSTICK_MAX - JOYSTICK_CENTER);

      // Map to integer range (e.g., 100-1000 for 1.0-10.0) then convert to float
      int minSpeedInt = (int)(SPEED_CONTROL_MIN_SPEED * 100);
      int maxSpeedInt = (int)(SPEED_CONTROL_MAX_SPEED * 100);
      globalSpeed = map(clampedDelta, JOYSTICK_DEADZONE, JOYSTICK_MAX - JOYSTICK_CENTER, minSpeedInt, maxSpeedInt) / 100.0f;
      poleSpeed = globalSpeed; // Sync pole speed
    }
    else // deltaY < 0 - handled by palette switching below
    {
      // Moving down: Set to deadzone speed (palette switching happens separately)
      globalSpeed = SPEED_CONTROL_DEADZONE_SPEED;
      poleSpeed = SPEED_CONTROL_DEADZONE_SPEED;
    }

    // Constrain final speeds to configured bounds
    globalSpeed = constrain(globalSpeed, SPEED_CONTROL_MIN_SPEED, SPEED_CONTROL_MAX_SPEED);
    poleSpeed = constrain(poleSpeed, SPEED_CONTROL_MIN_SPEED, SPEED_CONTROL_MAX_SPEED);

    lastSpeedChange = currentTime;

    // Optional: Print speed for debugging
    static float lastPrintedSpeed = -1.0f;
    if (abs(globalSpeed - lastPrintedSpeed) > 0.05f) // Only print if changed significantly
    {
      Serial.print("Speed: ");
      Serial.println(globalSpeed);
      lastPrintedSpeed = globalSpeed;
    }
  }

  // Pattern switching (horizontal left/right)
  if (canChangePattern && patternManager)
  {
    // Detect edge-triggered movement (joystick just crossed deadzone)
    if (abs(deltaX) > JOYSTICK_DEADZONE)
    {
      if (deltaX > 0 && lastDeltaX <= JOYSTICK_DEADZONE) // Right - next pattern
      {
        selectedPatternIndex = (selectedPatternIndex + 1) % patternManager->getPatternCount();
        patternManager->setCurrentPattern(selectedPatternIndex, false);
        lastPatternChange = currentTime;

        Serial.print("Pattern: ");
        Serial.println(patternManager->getCurrentPattern()->getName());
      }
      else if (deltaX < 0 && lastDeltaX >= -JOYSTICK_DEADZONE) // Left - previous pattern
      {
        selectedPatternIndex = (selectedPatternIndex + patternManager->getPatternCount() - 1) % patternManager->getPatternCount();
        patternManager->setCurrentPattern(selectedPatternIndex, false);
        lastPatternChange = currentTime;

        Serial.print("Pattern: ");
        Serial.println(patternManager->getCurrentPattern()->getName());
      }
    }

    lastDeltaX = deltaX;
  }

  // Palette switching (vertical down)
  if (canChangePalette && patternManager)
  {
    PaletteManager &palManager = patternManager->getPaletteManager();

    // Detect edge-triggered downward movement
    if (deltaY < -JOYSTICK_DEADZONE && lastDeltaY >= -JOYSTICK_DEADZONE)
    {
      selectedPaletteIndex = (selectedPaletteIndex + 1) % palManager.getPaletteCount();
      selectedPolePaletteIndex = selectedPaletteIndex; // Sync pole palette

      // Apply palette to clock/eye patterns
      patternManager->setCurrentPalette(selectedPaletteIndex);

      lastPaletteChange = currentTime;

      ColorPalette *palette = palManager.getPalette(selectedPaletteIndex);
      Serial.print("Palette: ");
      Serial.println(palette ? palette->getName() : "Unknown");
    }

    lastDeltaY = deltaY;
  }

  show();
}

// ============================================================================
// JOLT MODE HELPER IMPLEMENTATIONS
// ============================================================================

uint8_t LEDDriver::calculateJoltMagnitude(int joystickY)
{
  // Calculate distance from center (only upward motion)
  int deltaY = joystickY - JOYSTICK_CENTER;

  // Only consider upward motion (positive deltaY)
  if (deltaY <= JOLT_DEADZONE_THRESHOLD)
  {
    return 0; // In deadzone - center activation
  }

  // Smooth dynamic mapping from deadzone to maximum
  // Map deltaY from JOLT_DEADZONE_THRESHOLD to JOLT_LEVEL_5_THRESHOLD -> 1 to 255
  int clampedDelta = constrain(deltaY, JOLT_DEADZONE_THRESHOLD, JOLT_LEVEL_5_THRESHOLD);
  uint8_t magnitude = map(clampedDelta, JOLT_DEADZONE_THRESHOLD, JOLT_LEVEL_5_THRESHOLD, 1, 255);

  return magnitude;
}

uint8_t LEDDriver::calculateJoltMagnitudeDown(int joystickY)
{
  // Calculate distance from center (only downward motion)
  int deltaY = joystickY - JOYSTICK_CENTER;

  // Only consider downward motion (negative deltaY)
  if (deltaY >= -JOLT_DEADZONE_THRESHOLD)
  {
    return 0; // In deadzone or moving up
  }

  // Smooth dynamic mapping from deadzone to maximum (inverted for downward)
  // Map deltaY from -JOLT_DEADZONE_THRESHOLD to -JOLT_LEVEL_5_THRESHOLD -> 1 to 255
  int clampedDelta = constrain(-deltaY, JOLT_DEADZONE_THRESHOLD, JOLT_LEVEL_5_THRESHOLD);
  uint8_t magnitude = map(clampedDelta, JOLT_DEADZONE_THRESHOLD, JOLT_LEVEL_5_THRESHOLD, 1, 255);

  return magnitude;
}

void LEDDriver::renderJoltEffectDeadzone()
{
  // Clear all LEDs first
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  if (poleLeds)
  {
    fill_solid(poleLeds, POLE_NUM_LEDS, CRGB::Black);
  }

  // Deadzone: Light up center row of pole with pattern colors
  if (poleLeds && POLE_NUM_LEDS > 0 && patternManager)
  {
    ColorPalette *currentPalette = patternManager->getPaletteManager().getPalette(selectedJoltPaletteIndex);

    // Light up middle row (row 11 or 12 out of ~23 rows)
    int centerRow = (POLE_NUM_LEDS / POLE_SPIRAL_REPEAT) / 2;

    for (uint8_t col = 0; col < POLE_SPIRAL_REPEAT; col++)
    {
      uint16_t poleIndex = col + (centerRow * POLE_SPIRAL_REPEAT);
      if (poleIndex < POLE_NUM_LEDS)
      {
        CRGB color = CRGB::White;
        if (currentPalette)
        {
          float position = (float)col / (float)POLE_SPIRAL_REPEAT;
          color = currentPalette->getColorSmooth(position);
          color.nscale8(globalBrightness);
        }
        poleLeds[poleIndex] = color;
      }
    }
  }

  // Center eye (EYE_0)
  if (EYE_0_RAW_START < NUM_LEDS)
  {
    leds[EYE_0_RAW_START] = CRGB::White;
    leds[EYE_0_RAW_START].nscale8(globalBrightness);
  }
}

void LEDDriver::renderJoltEffectOutward(uint8_t magnitude)
{
  // Clear all LEDs first
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  if (poleLeds)
  {
    fill_solid(poleLeds, POLE_NUM_LEDS, CRGB::Black);
  }

  // Original behavior: expand from center outward
  renderJoltPoleOutward(magnitude);
  renderJoltEyeClockOutward(magnitude);
}

void LEDDriver::renderJoltEffectInward(uint8_t magnitude)
{
  // Clear all LEDs first
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  if (poleLeds)
  {
    fill_solid(poleLeds, POLE_NUM_LEDS, CRGB::Black);
  }

  // New behavior: fill from edges inward
  renderJoltPoleInward(magnitude);
  renderJoltEyeClockInward(magnitude);
}

void LEDDriver::renderJoltPoleOutward(uint8_t magnitude)
{
  if (!poleLeds || POLE_NUM_LEDS == 0)
    return;

  // Calculate how much of the pole to light up based on magnitude (0-255)
  float expansionPercent = (float)magnitude / 255.0f; // Smooth 0-100% expansion
  int ledsToLight = (int)(expansionPercent * POLE_NUM_LEDS);

  // Find center of pole
  int centerIndex = POLE_NUM_LEDS / 2;
  int halfRange = ledsToLight / 2;

  // Light up LEDs expanding from center both up and down
  for (int i = 0; i < halfRange; i++)
  {
    // Get color from selected palette based on position
    CRGB color = CRGB::White; // Default fallback

    if (patternManager)
    {
      ColorPalette *currentPalette = patternManager->getPaletteManager().getPalette(selectedJoltPaletteIndex);
      if (currentPalette && halfRange > 0)
      {
        float palettePosition = (float)i / (float)halfRange; // 0.0 to 1.0
        color = currentPalette->getColorSmooth(palettePosition);
        // Apply global brightness
        color.nscale8(globalBrightness);
      }
      else
      {
        // Fallback to rainbow if no palette available
        uint8_t hue = (i * 255) / max(1, halfRange); // Avoid division by zero
        color = CHSV(hue, 255, globalBrightness);
      }
    }
    else
    {
      // Fallback to rainbow if no pattern manager
      uint8_t hue = (i * 255) / max(1, halfRange); // Avoid division by zero
      color = CHSV(hue, 255, globalBrightness);
    }

    // Light up both directions from center
    int upIndex = centerIndex + i;
    int downIndex = centerIndex - i;

    if (upIndex < POLE_NUM_LEDS)
    {
      poleLeds[upIndex] = color;
    }
    if (downIndex >= 0)
    {
      poleLeds[downIndex] = color;
    }
  }
}

void LEDDriver::renderJoltEyeClockOutward(uint8_t magnitude)
{
  // Smooth expansion based on magnitude (0-255)
  // Map magnitude to ring expansion: 0-42 = EYE_0, 43-85 = +EYE_1, etc.
  float ringExpansion = (magnitude / 255.0f) * 6.0f; // 0.0 to 6.0 rings

  // Light up complete rings and partial ring based on smooth expansion
  for (int ring = 0; ring < 6; ring++)
  {
    float ringThreshold = (float)ring;

    if (ringExpansion > ringThreshold)
    {
      // Calculate how much of this ring to light up
      float ringProgress = min(1.0f, ringExpansion - ringThreshold);

      // Get color from selected palette for this ring
      CRGB color = CRGB::White; // Default fallback

      if (patternManager)
      {
        ColorPalette *currentPalette = patternManager->getPaletteManager().getPalette(selectedJoltPaletteIndex);
        if (currentPalette)
        {
          float palettePosition = (float)ring / 6.0f; // 0.0 to 1.0 across 6 rings
          color = currentPalette->getColorSmooth(palettePosition);
          // Apply brightness and ring progress
          color.nscale8((uint8_t)(globalBrightness * ringProgress));
        }
        else
        {
          // Fallback to rainbow if no palette available
          uint8_t hue = (ring * 255) / 6; // 6 total rings including clock
          color = CHSV(hue, 255, (uint8_t)(globalBrightness * ringProgress));
        }
      }
      else
      {
        // Fallback to rainbow if no pattern manager
        uint8_t hue = (ring * 255) / 6; // 6 total rings including clock
        color = CHSV(hue, 255, (uint8_t)(globalBrightness * ringProgress));
      }

      // Light up the appropriate ring
      int rawStartLED, count;
      switch (ring)
      {
      case 0:
        rawStartLED = EYE_0_RAW_START;
        count = EYE_0_COUNT;
        break;
      case 1:
        rawStartLED = EYE_1_RAW_START;
        count = EYE_1_COUNT;
        break;
      case 2:
        rawStartLED = EYE_2_RAW_START;
        count = EYE_2_COUNT;
        break;
      case 3:
        rawStartLED = EYE_3_RAW_START;
        count = EYE_3_COUNT;
        break;
      case 4:
        rawStartLED = EYE_4_RAW_START;
        count = EYE_4_COUNT;
        break;
      case 5:
        rawStartLED = CLOCK_RAW_START;
        count = CLOCK_COUNT;
        break;
      default:
        continue;
      }

      // For partial rings, only light up portion based on progress
      int ledsToLight = (int)(count * ringProgress);
      if (ledsToLight == 0 && ringProgress > 0)
        ledsToLight = 1; // At least 1 LED

      // Fill the ring with rainbow color
      for (int i = 0; i < ledsToLight; i++)
      {
        int ledIndex = rawStartLED + i;
        if (ledIndex < NUM_LEDS)
        {
          leds[ledIndex] = color;
        }
      }
    }
  }
}

// ============================================================================
// JOLT MODE INWARD VARIANTS (Filling from edges inward)
// ============================================================================

void LEDDriver::renderJoltPoleInward(uint8_t magnitude)
{
  if (!poleLeds || POLE_NUM_LEDS == 0)
    return;

  // Calculate how much of the pole to fill based on magnitude (0-255)
  // Fill from top and bottom edges toward center
  float expansionPercent = (float)magnitude / 255.0f; // Smooth 0-100% expansion
  int totalRows = POLE_NUM_LEDS / POLE_SPIRAL_REPEAT;
  // At magnitude 255, we should fill (totalRows / 2) rows from each side, meeting in the middle
  // Add +1 to ensure we reach the center row
  int maxRowsFromEachSide = (totalRows + 1) / 2; // Rounds up to include center
  int rowsToFill = (int)(expansionPercent * maxRowsFromEachSide);

  // Fill from top and bottom toward center
  for (int rowOffset = 0; rowOffset < rowsToFill; rowOffset++)
  {
    // Get color from selected palette based on position
    CRGB color = CRGB::White; // Default fallback

    if (patternManager)
    {
      ColorPalette *currentPalette = patternManager->getPaletteManager().getPalette(selectedJoltPaletteIndex);
      if (currentPalette && rowsToFill > 0)
      {
        float palettePosition = (float)rowOffset / (float)rowsToFill; // 0.0 to 1.0
        color = currentPalette->getColorSmooth(palettePosition);
        color.nscale8(globalBrightness);
      }
      else
      {
        uint8_t hue = (rowOffset * 255) / max(1, rowsToFill);
        color = CHSV(hue, 255, globalBrightness);
      }
    }
    else
    {
      uint8_t hue = (rowOffset * 255) / max(1, rowsToFill);
      color = CHSV(hue, 255, globalBrightness);
    }

    // Fill row from top (high indices)
    int topRow = totalRows - 1 - rowOffset;
    for (uint8_t col = 0; col < POLE_SPIRAL_REPEAT; col++)
    {
      uint16_t topIndex = col + (topRow * POLE_SPIRAL_REPEAT);
      if (topIndex < POLE_NUM_LEDS)
      {
        poleLeds[topIndex] = color;
      }
    }

    // Fill row from bottom (low indices)
    int bottomRow = rowOffset;
    for (uint8_t col = 0; col < POLE_SPIRAL_REPEAT; col++)
    {
      uint16_t bottomIndex = col + (bottomRow * POLE_SPIRAL_REPEAT);
      if (bottomIndex < POLE_NUM_LEDS)
      {
        poleLeds[bottomIndex] = color;
      }
    }
  }
}

void LEDDriver::renderJoltEyeClockInward(uint8_t magnitude)
{
  // Fill from outer edges (clock) inward to center (EYE_0)
  // Smooth expansion based on magnitude (0-255)
  // At magnitude 255, we want all 6 rings fully lit (ringExpansion >= 6.0)
  float ringExpansion = (magnitude / 255.0f) * 6.0f; // 0.0 to 6.0 rings

  // Light up rings from outside in: CLOCK (5) -> EYE_4 (4) -> ... -> EYE_0 (0)
  for (int ring = 5; ring >= 0; ring--)
  {
    // Calculate which ring we're filling based on expansion
    // At magnitude 0, no rings lit
    // At magnitude 255, all 6 rings lit (including center EYE_0)
    int ringsFromOutside = 5 - ring; // 0 for CLOCK, 5 for EYE_0
    float ringThreshold = (float)ringsFromOutside;

    // Use >= to ensure the last ring (EYE_0) lights up when ringExpansion reaches exactly 6.0
    if (ringExpansion >= ringThreshold)
    {
      // Calculate how much of this ring to light up
      float ringProgress = min(1.0f, ringExpansion - ringThreshold);

      // Get color from selected palette for this ring
      CRGB color = CRGB::White; // Default fallback

      if (patternManager)
      {
        ColorPalette *currentPalette = patternManager->getPaletteManager().getPalette(selectedJoltPaletteIndex);
        if (currentPalette)
        {
          // Invert palette position so outer rings get different colors than inner
          float palettePosition = (float)ringsFromOutside / 6.0f; // 0.0 to 1.0 across 6 rings
          color = currentPalette->getColorSmooth(palettePosition);
          // Apply brightness and ring progress
          color.nscale8((uint8_t)(globalBrightness * ringProgress));
        }
        else
        {
          // Fallback to rainbow if no palette available
          uint8_t hue = (ringsFromOutside * 255) / 6;
          color = CHSV(hue, 255, (uint8_t)(globalBrightness * ringProgress));
        }
      }
      else
      {
        // Fallback to rainbow if no pattern manager
        uint8_t hue = (ringsFromOutside * 255) / 6;
        color = CHSV(hue, 255, (uint8_t)(globalBrightness * ringProgress));
      }

      // Light up the appropriate ring
      int rawStartLED, count;
      switch (ring)
      {
      case 0:
        rawStartLED = EYE_0_RAW_START;
        count = EYE_0_COUNT;
        break;
      case 1:
        rawStartLED = EYE_1_RAW_START;
        count = EYE_1_COUNT;
        break;
      case 2:
        rawStartLED = EYE_2_RAW_START;
        count = EYE_2_COUNT;
        break;
      case 3:
        rawStartLED = EYE_3_RAW_START;
        count = EYE_3_COUNT;
        break;
      case 4:
        rawStartLED = EYE_4_RAW_START;
        count = EYE_4_COUNT;
        break;
      case 5:
        rawStartLED = CLOCK_RAW_START;
        count = CLOCK_COUNT;
        break;
      default:
        continue;
      }

      // For partial rings, only light up portion based on progress
      int ledsToLight = (int)(count * ringProgress);
      if (ledsToLight == 0 && ringProgress > 0)
        ledsToLight = 1; // At least 1 LED

      // Fill the ring with color
      for (int i = 0; i < ledsToLight; i++)
      {
        int ledIndex = rawStartLED + i;
        if (ledIndex < NUM_LEDS)
        {
          leds[ledIndex] = color;
        }
      }
    }
  }
}
