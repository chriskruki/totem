#include "LEDDriver.h"
#include <Arduino.h>

LEDDriver::LEDDriver() : brightness(DEFAULT_BRIGHTNESS),
                         lastUpdate(0),
                         needsUpdate(false),
                         currentMode(MODE_CONFIG),
                         currentR(STATIC_COLOR_R),
                         currentG(STATIC_COLOR_G),
                         currentB(STATIC_COLOR_B),
                         blinkState(false),
                         lastBlinkTime(0),
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

      // Check for double-click to enter pointer mode
      if (buttonState && detectDoubleClick(buttonState, currentTime))
      {
        currentMode = MODE_POINTER;
        Serial.println("Mode changed to: Pointer Mode (Double-Click)");
      }
      // Normal mode switching on single button press
      else if (buttonState && !inCalibrationMode)
      {
        // If in pointer mode, single click returns to normal mode cycling
        if (currentMode == MODE_POINTER)
        {
          currentMode = MODE_CONFIG; // Return to first mode
          Serial.println("Mode changed to: Config (Brightness Control) - Exited Pointer Mode");
        }
        else
        {
          // Normal cycling through Config -> Color -> Blink
          currentMode = (currentMode + 1) % NUM_MODES;
          Serial.print("Mode changed to: ");
          switch (currentMode)
          {
          case MODE_CONFIG:
            Serial.println("Config (Brightness Control)");
            break;
          case MODE_COLOR:
            Serial.println("Color Wheel");
            break;
          case MODE_BLINK:
            Serial.println("Blink Mode");
            break;
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
  case MODE_CONFIG:
    // Mode 1: Brightness control with Y-axis
    processBrightnessMode();
    break;

  case MODE_COLOR:
    // Mode 2: RGB color wheel control
    processColorWheelMode();
    break;

  case MODE_BLINK:
    // Mode 3: White blink placeholder
    processBlinkMode();
    break;

  case MODE_POINTER:
    // Mode 4: Circular pointer mode
    processPointerMode();
    break;
  }
}

void LEDDriver::processBrightnessMode()
{
  static unsigned long lastBrightnessChange = 0;
  static int lastYDiff = 0;
  const unsigned long BRIGHTNESS_CHANGE_INTERVAL = 100; // Only change brightness every 100ms

  // Use Y-axis to control brightness
  int yDiff = joystickState.y - JOYSTICK_CENTER;
  unsigned long currentTime = millis();

  // Only process if enough time has passed and joystick moved significantly
  if (abs(yDiff) > JOYSTICK_DEADZONE &&
      (currentTime - lastBrightnessChange) >= BRIGHTNESS_CHANGE_INTERVAL &&
      abs(yDiff - lastYDiff) > (JOYSTICK_DEADZONE / 2))
  {

    // Map joystick movement to brightness change
    int brightnessChange = 0;

    if (yDiff > 0)
    {
      // Joystick up - increase brightness
      brightnessChange = BRIGHTNESS_STEP;
    }
    else
    {
      // Joystick down - decrease brightness
      brightnessChange = -BRIGHTNESS_STEP;
    }

    // Apply brightness change
    int newBrightness = brightness + brightnessChange;
    newBrightness = constrain(newBrightness, 0, MAX_BRIGHTNESS);

    if (newBrightness != brightness)
    {
      setBrightness(newBrightness);
      // Set current color to show the brightness change
      setSolidColor(currentR, currentG, currentB);
      show();

      lastBrightnessChange = currentTime;
      lastYDiff = yDiff;
    }
  }
}

void LEDDriver::processColorWheelMode()
{
  // Use X and Y axes to control RGB values like a color wheel
  int xDiff = joystickState.x - JOYSTICK_CENTER;
  int yDiff = joystickState.y - JOYSTICK_CENTER;

  bool colorChanged = false;

  // X-axis controls red/green balance
  if (abs(xDiff) > JOYSTICK_DEADZONE)
  {
    if (xDiff > 0)
    {
      // Move right - increase red, decrease green
      currentR = constrain(currentR + COLOR_SENSITIVITY, 0, 255);
      currentG = constrain(currentG - COLOR_SENSITIVITY, 0, 255);
    }
    else
    {
      // Move left - decrease red, increase green
      currentR = constrain(currentR - COLOR_SENSITIVITY, 0, 255);
      currentG = constrain(currentG + COLOR_SENSITIVITY, 0, 255);
    }
    colorChanged = true;
  }

  // Y-axis controls blue component
  if (abs(yDiff) > JOYSTICK_DEADZONE)
  {
    if (yDiff > 0)
    {
      // Move up - increase blue
      currentB = constrain(currentB + COLOR_SENSITIVITY, 0, 255);
    }
    else
    {
      // Move down - decrease blue
      currentB = constrain(currentB - COLOR_SENSITIVITY, 0, 255);
    }
    colorChanged = true;
  }

  // Update LEDs if color changed
  if (colorChanged)
  {
    setSolidColor(currentR, currentG, currentB);
    show();

    // Only print color values when explicitly requested
    // Serial.print("RGB: (");
    // Serial.print(currentR);
    // Serial.print(", ");
    // Serial.print(currentG);
    // Serial.print(", ");
    // Serial.print(currentB);
    // Serial.println(")");
  }
}

void LEDDriver::processBlinkMode()
{
  // Mode 3: Blink white as placeholder
  unsigned long currentTime = millis();

  if (currentTime - lastBlinkTime >= BLINK_INTERVAL)
  {
    blinkState = !blinkState;
    lastBlinkTime = currentTime;

    if (blinkState)
    {
      setSolidColor(CRGB::DarkRed); // White
      // Serial.println("Blink: ON");
    }
    else
    {
      setSolidColor(CRGB::Red); // Off
      // Serial.println("Blink: OFF");
    }
    show();
  }
}

void LEDDriver::processPointerMode()
{
  // Mode 4: Circular pointer mode
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
    case MODE_CONFIG:
      Serial.println("Config (Brightness Control)");
      break;
    case MODE_COLOR:
      Serial.println("Color Wheel");
      break;
    case MODE_BLINK:
      Serial.println("Blink Mode");
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
