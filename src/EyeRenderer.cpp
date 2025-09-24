#include "EyeRenderer.h"
#include <Arduino.h>

EyeRenderer::EyeRenderer(CRGB *leds, SegmentManager *segManager)
    : leds(leds), segmentManager(segManager), currentDirection(0)
{
  // Set default eye colors
  irisColor = CRGB::Blue;
  scleraColor = CRGB(10, 10, 10); // Very dim background

  // Initialize preset iris positions
  initializeIrisPositions();
}

void EyeRenderer::initializeIrisPositions()
{
  // Define preset iris positions for each direction
  // Direction indices: 0=Center, 1=N, 2=NE, 3=E, 4=SE, 5=S, 6=SW, 7=W, 8=NW

  // CENTER (0) - Iris in the center area
  irisPositions[0].ledCount = 9;
  irisPositions[0].ledIndices[0] = 60; // EYE_0 (center)
  irisPositions[0].ledIndices[1] = 52; // EYE_1 top
  irisPositions[0].ledIndices[2] = 53; // EYE_1 right
  irisPositions[0].ledIndices[3] = 54; // EYE_1 bottom (duplicate center for now)
  irisPositions[0].ledIndices[4] = 55; // EYE_1 left
  irisPositions[0].ledIndices[5] = 56; // EYE_1 left
  irisPositions[0].ledIndices[6] = 57; // EYE_1 left
  irisPositions[0].ledIndices[7] = 58; // EYE_1 left
  irisPositions[0].ledIndices[8] = 59; // EYE_1 left

  // NORTH (1) - Looking up
  irisPositions[1].ledCount = 9;
  irisPositions[1].ledIndices[0] = 24; // EYE_0 (center)
  irisPositions[1].ledIndices[1] = 22; // EYE_1 top
  irisPositions[1].ledIndices[2] = 23; // EYE_1 right
  irisPositions[1].ledIndices[3] = 0;  // EYE_1 bottom (duplicate center for now)
  irisPositions[1].ledIndices[4] = 1;  // EYE_1 left
  irisPositions[1].ledIndices[5] = 2;  // EYE_1 left
  irisPositions[1].ledIndices[6] = 39; // EYE_1 left
  irisPositions[1].ledIndices[7] = 25; // EYE_1 left
  irisPositions[1].ledIndices[8] = 40; // EYE_1 left

  // NORTHEAST (2) - Looking up-right
  irisPositions[2].ledCount = 10;
  irisPositions[2].ledIndices[0] = 26; // EYE_0 (center)
  irisPositions[2].ledIndices[1] = 1;  // EYE_1 top
  irisPositions[2].ledIndices[2] = 2;  // EYE_1 right
  irisPositions[2].ledIndices[3] = 3;  // EYE_1 bottom (duplicate center for now)
  irisPositions[2].ledIndices[4] = 4;  // EYE_1 left
  irisPositions[2].ledIndices[5] = 5;  // EYE_1 left
  irisPositions[2].ledIndices[6] = 25; // EYE_1 left
  irisPositions[2].ledIndices[7] = 27; // EYE_1 left
  irisPositions[2].ledIndices[8] = 41; // EYE_1 left
  irisPositions[2].ledIndices[9] = 42; // EYE_1 left

  // EAST (3) - Looking right
  irisPositions[3].ledCount = 9;
  irisPositions[3].ledIndices[0] = 28; // EYE_0 (center)
  irisPositions[3].ledIndices[1] = 4;  // EYE_1 top
  irisPositions[3].ledIndices[2] = 5;  // EYE_1 right
  irisPositions[3].ledIndices[3] = 6;  // EYE_1 bottom (duplicate center for now)
  irisPositions[3].ledIndices[4] = 7;  // EYE_1 left
  irisPositions[3].ledIndices[5] = 8;  // EYE_1 left
  irisPositions[3].ledIndices[6] = 27; // EYE_1 left
  irisPositions[3].ledIndices[7] = 29; // EYE_1 left
  irisPositions[3].ledIndices[8] = 43; // EYE_1 left

  // SOUTHEAST (4) - Looking down-right
  irisPositions[4].ledCount = 10;
  irisPositions[4].ledIndices[0] = 30; // EYE_0 (center)
  irisPositions[4].ledIndices[1] = 7;  // EYE_1 top
  irisPositions[4].ledIndices[2] = 8;  // EYE_1 right
  irisPositions[4].ledIndices[3] = 9;  // EYE_1 bottom (duplicate center for now)
  irisPositions[4].ledIndices[4] = 10; // EYE_1 left
  irisPositions[4].ledIndices[5] = 11; // EYE_1 left
  irisPositions[4].ledIndices[6] = 29; // EYE_1 left
  irisPositions[4].ledIndices[7] = 31; // EYE_1 left
  irisPositions[4].ledIndices[8] = 44; // EYE_1 left
  irisPositions[4].ledIndices[9] = 45; // EYE_1 left

  // SOUTH (5) - Looking down
  irisPositions[5].ledCount = 9;
  irisPositions[5].ledIndices[0] = 32; // EYE_0 (center)
  irisPositions[5].ledIndices[1] = 10; // EYE_1 top
  irisPositions[5].ledIndices[2] = 11; // EYE_1 right
  irisPositions[5].ledIndices[3] = 12; // EYE_1 bottom (duplicate center for now)
  irisPositions[5].ledIndices[4] = 13; // EYE_1 left
  irisPositions[5].ledIndices[5] = 14; // EYE_1 left
  irisPositions[5].ledIndices[6] = 31; // EYE_1 left
  irisPositions[5].ledIndices[7] = 33; // EYE_1 left
  irisPositions[5].ledIndices[8] = 46; // EYE_1 left

  // SOUTHWEST (6) - Looking down-left
  irisPositions[6].ledCount = 10;
  irisPositions[6].ledIndices[0] = 34; // EYE_0 (center)
  irisPositions[6].ledIndices[1] = 13; // EYE_1 top
  irisPositions[6].ledIndices[2] = 14; // EYE_1 right
  irisPositions[6].ledIndices[3] = 15; // EYE_1 bottom (duplicate center for now)
  irisPositions[6].ledIndices[4] = 16; // EYE_1 left
  irisPositions[6].ledIndices[5] = 17; // EYE_1 left
  irisPositions[6].ledIndices[6] = 33; // EYE_1 left
  irisPositions[6].ledIndices[7] = 35; // EYE_1 left
  irisPositions[6].ledIndices[8] = 47; // EYE_1 left
  irisPositions[6].ledIndices[9] = 48; // EYE_1 left

  // WEST (7) - Looking left
  irisPositions[7].ledCount = 9;
  irisPositions[7].ledIndices[0] = 36; // EYE_0 (center)
  irisPositions[7].ledIndices[1] = 16; // EYE_1 left
  irisPositions[7].ledIndices[2] = 17; // EYE_1 left
  irisPositions[7].ledIndices[3] = 18; // EYE_1 left
  irisPositions[7].ledIndices[4] = 19; // EYE_1 left
  irisPositions[7].ledIndices[5] = 20; // EYE_1 left
  irisPositions[7].ledIndices[6] = 35; // EYE_1 left
  irisPositions[7].ledIndices[7] = 37; // EYE_1 left
  irisPositions[7].ledIndices[8] = 49; // EYE_1 left

  // NORTHWEST (8) - Looking up-left
  irisPositions[8].ledCount = 10;
  irisPositions[8].ledIndices[0] = 38; // EYE_0 (center)
  irisPositions[8].ledIndices[1] = 19; // EYE_1 top
  irisPositions[8].ledIndices[2] = 20; // EYE_1 right
  irisPositions[8].ledIndices[3] = 21; // EYE_1 bottom (duplicate center for now)
  irisPositions[8].ledIndices[4] = 22; // EYE_1 left
  irisPositions[8].ledIndices[5] = 23; // EYE_1 left
  irisPositions[8].ledIndices[6] = 37; // EYE_1 left
  irisPositions[8].ledIndices[7] = 39; // EYE_1 left
  irisPositions[8].ledIndices[8] = 50; // EYE_1 left
  irisPositions[8].ledIndices[9] = 51; // EYE_1 left
}

void EyeRenderer::updateEyePosition(int joystickX, int joystickY)
{
  currentDirection = calculateDirection(joystickX, joystickY);

  // Debug output
  Serial.print("Eye direction: ");
  Serial.print(currentDirection);
  const char *dirNames[] = {"CENTER", "N", "NE", "E", "SE", "S", "SW", "W", "NW"};
  Serial.print(" (");
  Serial.print(dirNames[currentDirection]);
  Serial.println(")");
}

uint8_t EyeRenderer::calculateDirection(int joystickX, int joystickY)
{
  // Convert to center-relative coordinates
  float x = (float)(joystickX - JOYSTICK_CENTER);
  float y = (float)(joystickY - JOYSTICK_CENTER);

  // Check if in deadzone (center)
  float distance = sqrt(x * x + y * y);
  if (distance < JOYSTICK_DEADZONE)
  {
    return 0; // CENTER
  }

  // Calculate angle and convert to direction
  float angle = getAngleDegrees(x, y);

  // Map angle to 8 directions
  // Adjust angle so 0° = North, then divide into 8 sectors of 45° each
  if (angle >= 337.5f || angle < 22.5f)
    return 1; // N
  else if (angle >= 22.5f && angle < 67.5f)
    return 2; // NE
  else if (angle >= 67.5f && angle < 112.5f)
    return 3; // E
  else if (angle >= 112.5f && angle < 157.5f)
    return 4; // SE
  else if (angle >= 157.5f && angle < 202.5f)
    return 5; // S
  else if (angle >= 202.5f && angle < 247.5f)
    return 6; // SW
  else if (angle >= 247.5f && angle < 292.5f)
    return 7; // W
  else        // 292.5f to 337.5f
    return 8; // NW
}

float EyeRenderer::getAngleDegrees(float x, float y)
{
  // Calculate angle in degrees, with 0° = North (positive Y)
  // Since segments are now reversed, use original atan2(x, y) coordinate system
  float angle = atan2(x, y) * 180.0f / PI;

  // Convert to 0-360 range
  if (angle < 0)
  {
    angle += 360.0f;
  }

  return angle;
}

void EyeRenderer::renderEye()
{
  if (!segmentManager)
  {
    return;
  }

  // Clear all eye segments first
  for (uint8_t segType = SEGMENT_EYE_4; segType <= SEGMENT_EYE_0; segType++)
  {
    segmentManager->clearSegment(leds, segType);
  }

  // Light up the LEDs for the current iris position
  const IrisPosition &position = irisPositions[currentDirection];

  for (uint8_t i = 0; i < position.ledCount; i++)
  {
    uint16_t ledIndex = position.ledIndices[i];
    if (ledIndex < NUM_LEDS)
    {
      leds[ledIndex] = irisColor;
    }
  }
}

void EyeRenderer::setEyeColors(CRGB irisColor, CRGB scleraColor)
{
  this->irisColor = irisColor;
  this->scleraColor = scleraColor;
}
