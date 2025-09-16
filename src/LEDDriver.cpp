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
  FastLED.setMaxPowerInVoltsAndMilliamps(5, 2000); // 5V, 2A power limit

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

      // Check for double-click to enter calibration mode
      if (buttonState && detectDoubleClick(buttonState, currentTime))
      {
        startCalibrationMode();
      }
      // Normal mode switching on single button press
      else if (buttonState && !inCalibrationMode)
      {
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
        case MODE_POINTER:
          Serial.println("Pointer Mode");
          break;
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
      setSolidColor(255, 255, 255); // White
      // Serial.println("Blink: ON");
    }
    else
    {
      setSolidColor(0, 0, 0); // Off
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

  // Set background color for all LEDs first
  setSolidColor((POINTER_BG_COLOR_R * POINTER_BG_BRIGHTNESS) / 255,
                (POINTER_BG_COLOR_G * POINTER_BG_BRIGHTNESS) / 255,
                (POINTER_BG_COLOR_B * POINTER_BG_BRIGHTNESS) / 255);

  if (magnitude > JOYSTICK_DEADZONE)
  {
    // Calculate angle in radians (atan2 returns -π to π)
    // Note: We need to flip Y coordinate to match expected behavior
    // Standard joystick: Up = higher Y values, but we want Up = 12 o'clock
    float angle = atan2(-yDiff, xDiff); // Flip Y to correct coordinate system

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

    // Light up POINTER_LED_COUNT LEDs centered around the target position
    for (int i = 0; i < POINTER_LED_COUNT; i++)
    {
      int offset = i - (POINTER_LED_COUNT / 2); // Center the group
      int ledIndex = (centerLED + offset + NUM_LEDS) % NUM_LEDS;

      // Set LED color with slight dimming for outer LEDs
      uint8_t brightness = (i == POINTER_LED_COUNT / 2) ? POINTER_BRIGHTNESS : (POINTER_BRIGHTNESS * 2 / 3);
      setLED(ledIndex,
             (POINTER_COLOR_R * brightness) / 255,
             (POINTER_COLOR_G * brightness) / 255,
             (POINTER_COLOR_B * brightness) / 255);
    }
  }

  show();
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
