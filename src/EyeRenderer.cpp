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
  irisPositions[0].ledIndices[0] = 160; // EYE_0 (center)
  irisPositions[0].ledIndices[1] = 152; // EYE_1 top
  irisPositions[0].ledIndices[2] = 153; // EYE_1 right
  irisPositions[0].ledIndices[3] = 154; // EYE_1 bottom (duplicate center for now)
  irisPositions[0].ledIndices[4] = 155; // EYE_1 left
  irisPositions[0].ledIndices[5] = 156; // EYE_1 left
  irisPositions[0].ledIndices[6] = 157; // EYE_1 left
  irisPositions[0].ledIndices[7] = 158; // EYE_1 left
  irisPositions[0].ledIndices[8] = 159; // EYE_1 left

  // NORTH (1) - Looking up
  irisPositions[1].ledCount = 9;
  irisPositions[1].ledIndices[0] = 132; // Center
  irisPositions[1].ledIndices[1] = 112;
  irisPositions[1].ledIndices[2] = 113;
  irisPositions[1].ledIndices[3] = 133;
  irisPositions[1].ledIndices[4] = 147;
  irisPositions[1].ledIndices[5] = 146;
  irisPositions[1].ledIndices[6] = 145;
  irisPositions[1].ledIndices[7] = 131;
  irisPositions[1].ledIndices[8] = 111;

  // NORTHEAST (2) - Looking up-right
  irisPositions[2].ledCount = 8;
  irisPositions[2].ledIndices[0] = 134; // Center
  irisPositions[2].ledIndices[1] = 115;
  irisPositions[2].ledIndices[2] = 116;
  irisPositions[2].ledIndices[3] = 135;
  irisPositions[2].ledIndices[4] = 148;
  irisPositions[2].ledIndices[5] = 147;
  irisPositions[2].ledIndices[6] = 133;
  irisPositions[2].ledIndices[7] = 114;

  // EAST (3) - Looking right
  irisPositions[3].ledCount = 9;
  irisPositions[3].ledIndices[0] = 136; // Center
  irisPositions[3].ledIndices[1] = 135;
  irisPositions[3].ledIndices[2] = 137;
  irisPositions[3].ledIndices[3] = 117;
  irisPositions[3].ledIndices[4] = 118;
  irisPositions[3].ledIndices[5] = 119;
  irisPositions[3].ledIndices[6] = 148;
  irisPositions[3].ledIndices[7] = 149;
  irisPositions[3].ledIndices[8] = 150;

  // SOUTHEAST (4) - Looking down-right
  irisPositions[4].ledCount = 8;
  irisPositions[4].ledIndices[0] = 138; // Center
  irisPositions[4].ledIndices[1] = 137;
  irisPositions[4].ledIndices[2] = 139;
  irisPositions[4].ledIndices[3] = 120;
  irisPositions[4].ledIndices[4] = 121;
  irisPositions[4].ledIndices[5] = 122;
  irisPositions[4].ledIndices[6] = 150;
  irisPositions[4].ledIndices[7] = 151;

  // SOUTH (5) - Looking down
  irisPositions[5].ledCount = 9;
  irisPositions[5].ledIndices[0] = 124; // Center
  irisPositions[5].ledIndices[1] = 125;
  irisPositions[5].ledIndices[2] = 139;
  irisPositions[5].ledIndices[3] = 123;
  irisPositions[5].ledIndices[4] = 100;
  irisPositions[5].ledIndices[5] = 101;
  irisPositions[5].ledIndices[6] = 151;
  irisPositions[5].ledIndices[7] = 140;
  irisPositions[5].ledIndices[8] = 141;

  // SOUTHWEST (6) - Looking down-left
  irisPositions[6].ledCount = 8;
  irisPositions[6].ledIndices[0] = 126; // Center
  irisPositions[6].ledIndices[1] = 125;
  irisPositions[6].ledIndices[2] = 127;
  irisPositions[6].ledIndices[3] = 102;
  irisPositions[6].ledIndices[4] = 103;
  irisPositions[6].ledIndices[5] = 104;
  irisPositions[6].ledIndices[6] = 141;
  irisPositions[6].ledIndices[7] = 142;

  // WEST (7) - Looking left
  irisPositions[7].ledCount = 9;
  irisPositions[7].ledIndices[0] = 128; // Center
  irisPositions[7].ledIndices[1] = 127;
  irisPositions[7].ledIndices[2] = 129;
  irisPositions[7].ledIndices[3] = 105;
  irisPositions[7].ledIndices[4] = 106;
  irisPositions[7].ledIndices[5] = 107;
  irisPositions[7].ledIndices[6] = 142;
  irisPositions[7].ledIndices[7] = 143;
  irisPositions[7].ledIndices[8] = 144;

  // NORTHWEST (8) - Looking up-left
  irisPositions[8].ledCount = 8;
  irisPositions[8].ledIndices[0] = 130; // Center
  irisPositions[8].ledIndices[1] = 129;
  irisPositions[8].ledIndices[2] = 131;
  irisPositions[8].ledIndices[3] = 108;
  irisPositions[8].ledIndices[4] = 109;
  irisPositions[8].ledIndices[5] = 110;
  irisPositions[8].ledIndices[6] = 144;
  irisPositions[8].ledIndices[7] = 145;
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
  // Negate x to fix reversed East/West directions
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
      if (i == 0)
      {
        // First LED is the iris center - red
        leds[ledIndex] = CRGB::Red;
      }
      else
      {
        // Remaining LEDs are white
        leds[ledIndex] = CRGB::White;
      }
    }
  }
}

void EyeRenderer::setEyeColors(CRGB irisColor, CRGB scleraColor)
{
  this->irisColor = irisColor;
  this->scleraColor = scleraColor;
}
