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
  irisPositions[0].ledCount = 5;
  irisPositions[0].ledIndices[0] = 60; // EYE_0 (center)
  irisPositions[0].ledIndices[1] = 56; // EYE_1 top
  irisPositions[0].ledIndices[2] = 58; // EYE_1 right
  irisPositions[0].ledIndices[3] = 52; // EYE_1 bottom (duplicate center for now)
  irisPositions[0].ledIndices[4] = 54; // EYE_1 left

  // NORTH (1) - Looking up
  irisPositions[1].ledCount = 5;
  irisPositions[1].ledIndices[0] = 24; // EYE_0 (center)
  irisPositions[1].ledIndices[1] = 0;  // EYE_1 top
  irisPositions[1].ledIndices[2] = 25; // EYE_2 top-right
  irisPositions[1].ledIndices[3] = 39; // EYE_2 top
  irisPositions[1].ledIndices[4] = 40; // EYE_2 top-left

  // NORTHEAST (2) - Looking up-right
  irisPositions[2].ledCount = 5;
  irisPositions[2].ledIndices[0] = 3;  // EYE_0 (center)
  irisPositions[2].ledIndices[1] = 26; // EYE_1 right
  irisPositions[2].ledIndices[2] = 41; // EYE_2 top-right
  irisPositions[2].ledIndices[3] = 25; // EYE_2 right
  irisPositions[2].ledIndices[4] = 27; // EYE_3 top-right

  // EAST (3) - Looking right
  irisPositions[3].ledCount = 5;
  irisPositions[3].ledIndices[0] = 28; // EYE_0 (center)
  irisPositions[3].ledIndices[1] = 6;  // EYE_1 right
  irisPositions[3].ledIndices[2] = 43; // EYE_2 right
  irisPositions[3].ledIndices[3] = 27; // EYE_2 bottom-right
  irisPositions[3].ledIndices[4] = 29; // EYE_3 right

  // SOUTHEAST (4) - Looking down-right
  irisPositions[4].ledCount = 5;
  irisPositions[4].ledIndices[0] = 9;  // EYE_0 (center)
  irisPositions[4].ledIndices[1] = 30; // EYE_1 bottom
  irisPositions[4].ledIndices[2] = 29; // EYE_2 bottom-right
  irisPositions[4].ledIndices[3] = 31; // EYE_2 bottom
  irisPositions[4].ledIndices[4] = 44; // EYE_3 bottom-right

  // SOUTH (5) - Looking down
  irisPositions[5].ledCount = 5;
  irisPositions[5].ledIndices[0] = 46; // EYE_0 (center)
  irisPositions[5].ledIndices[1] = 32; // EYE_1 bottom
  irisPositions[5].ledIndices[2] = 12; // EYE_2 bottom-right
  irisPositions[5].ledIndices[3] = 33; // EYE_2 bottom
  irisPositions[5].ledIndices[4] = 31; // EYE_2 bottom-left

  // SOUTHWEST (6) - Looking down-left
  irisPositions[6].ledCount = 5;
  irisPositions[6].ledIndices[0] = 15; // EYE_0 (center)
  irisPositions[6].ledIndices[1] = 34; // EYE_1 left
  irisPositions[6].ledIndices[2] = 56; // EYE_2 bottom-left
  irisPositions[6].ledIndices[3] = 35; // EYE_2 bottom
  irisPositions[6].ledIndices[4] = 33; // EYE_3 bottom-left

  // WEST (7) - Looking left
  irisPositions[7].ledCount = 5;
  irisPositions[7].ledIndices[0] = 18; // EYE_0 (center)
  irisPositions[7].ledIndices[1] = 36; // EYE_1 left
  irisPositions[7].ledIndices[2] = 37; // EYE_2 left
  irisPositions[7].ledIndices[3] = 35; // EYE_2 top-left
  irisPositions[7].ledIndices[4] = 49; // EYE_3 left

  // NORTHWEST (8) - Looking up-left
  irisPositions[8].ledCount = 5;
  irisPositions[8].ledIndices[0] = 21; // EYE_0 (center)
  irisPositions[8].ledIndices[1] = 38; // EYE_1 top
  irisPositions[8].ledIndices[2] = 39; // EYE_2 top-left
  irisPositions[8].ledIndices[3] = 50; // EYE_2 left
  irisPositions[8].ledIndices[4] = 51; // EYE_3 top-left
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
