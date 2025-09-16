#include "LEDDriver.h"
#include <Arduino.h>

LEDDriver::LEDDriver() : brightness(DEFAULT_BRIGHTNESS),
                         lastUpdate(0),
                         needsUpdate(false)
{

  // Initialize joystick state
  joystickState.x = JOYSTICK_CENTER;
  joystickState.y = JOYSTICK_CENTER;
  joystickState.buttonPressed = false;
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

  Serial.print("Brightness set to: ");
  Serial.println(brightness);
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
  // Placeholder for joystick reading implementation
  // This will be implemented in a future phase

  /*
  // Future implementation:
  int rawX = analogRead(JOYSTICK_X_PIN);
  int rawY = analogRead(JOYSTICK_Y_PIN);
  bool buttonState = !digitalRead(JOYSTICK_BUTTON_PIN); // Inverted due to pullup

  // Apply deadzone
  if (abs(rawX - JOYSTICK_CENTER) < JOYSTICK_DEADZONE) {
      rawX = JOYSTICK_CENTER;
  }
  if (abs(rawY - JOYSTICK_CENTER) < JOYSTICK_DEADZONE) {
      rawY = JOYSTICK_CENTER;
  }

  joystickState.x = rawX;
  joystickState.y = rawY;
  joystickState.buttonPressed = buttonState;

  processJoystickInput();
  */
}

void LEDDriver::processJoystickInput()
{
  // Placeholder for joystick input processing
  // This will be implemented in a future phase

  /*
  // Future implementation ideas:
  // - Map joystick X to LED position along strip
  // - Map joystick Y to brightness or color hue
  // - Use button press to change modes or colors
  // - Create moving patterns based on joystick movement
  */
}
