#include "ColorPalette.h"

// ============================================================================
// ColorPalette Implementation
// ============================================================================

ColorPalette::ColorPalette(const CRGBPalette16 &palette, const String &name, const String &description)
    : palette(palette), name(name), description(description), isCustom(false)
{
}

ColorPalette::ColorPalette(const CRGB *colors, int numColors, const String &name, const String &description)
    : name(name), description(description), isCustom(true)
{

  // Create palette from color array
  if (numColors <= 0 || !colors)
  {
    // Default to white if invalid input
    fill_solid(palette, 16, CRGB::White);
    return;
  }

  // Fill palette with provided colors, repeating if necessary
  for (int i = 0; i < 16; i++)
  {
    palette[i] = colors[i % numColors];
  }
}

CRGB ColorPalette::getColor(uint8_t index) const
{
  return ColorFromPalette(palette, index, 255, LINEARBLEND);
}

CRGB ColorPalette::getColorSmooth(float position) const
{
  position = constrain(position, 0.0f, 1.0f);
  uint8_t index = (uint8_t)(position * 255.0f);
  return getColor(index);
}

void ColorPalette::updateColors(const CRGB *colors, int numColors)
{
  if (numColors <= 0 || !colors)
    return;

  // Update palette with new colors
  for (int i = 0; i < 16; i++)
  {
    palette[i] = colors[i % numColors];
  }
  isCustom = true;
}

ColorPalette ColorPalette::createGradient(CRGB color1, CRGB color2, const String &name, const String &description)
{
  CRGB gradientColors[2] = {color1, color2};
  return ColorPalette(gradientColors, 2, name, description);
}

ColorPalette ColorPalette::createHSVRange(uint8_t startHue, uint8_t endHue, uint8_t saturation, uint8_t value, const String &name, const String &description)
{
  CRGB hsvColors[16];

  for (int i = 0; i < 16; i++)
  {
    uint8_t hue = map(i, 0, 15, startHue, endHue);
    hsvColors[i] = CHSV(hue, saturation, value);
  }

  return ColorPalette(hsvColors, 16, name, description);
}

// ============================================================================
// PaletteManager Implementation
// ============================================================================

PaletteManager::PaletteManager() : paletteCount(0), currentPaletteIndex(0)
{
  // Initialize palette array
  for (int i = 0; i < MAX_PALETTES; i++)
  {
    palettes[i] = nullptr;
  }
}

PaletteManager::~PaletteManager()
{
  // Clean up allocated palettes
  for (int i = 0; i < paletteCount; i++)
  {
    delete palettes[i];
  }
}

void PaletteManager::initialize()
{
  Serial.println("Initializing color palettes...");

  // Add predefined FastLED palettes
  addPalette(new ColorPalette(RainbowColors_p, "Rainbow", "Classic rainbow colors"));
  addPalette(new ColorPalette(RainbowStripeColors_p, "Rainbow Stripes", "Rainbow with black stripes"));
  addPalette(new ColorPalette(OceanColors_p, "Ocean", "Deep blue ocean colors"));
  addPalette(new ColorPalette(CloudColors_p, "Clouds", "Soft white and blue clouds"));
  addPalette(new ColorPalette(LavaColors_p, "Lava", "Hot lava colors"));
  addPalette(new ColorPalette(ForestColors_p, "Forest", "Green forest colors"));
  addPalette(new ColorPalette(PartyColors_p, "Party", "Bright party colors"));
  addPalette(new ColorPalette(HeatColors_p, "Heat", "Fire heat colors"));

  // Add custom predefined palettes
  addPalette(new ColorPalette(PredefinedPalettes::WARM_COLORS, 4, "Warm", "Warm sunset colors"));
  addPalette(new ColorPalette(PredefinedPalettes::COOL_COLORS, 4, "Cool", "Cool blue and purple"));
  addPalette(new ColorPalette(PredefinedPalettes::FIRE_COLORS, 4, "Fire", "Flickering fire"));
  addPalette(new ColorPalette(PredefinedPalettes::OCEAN_COLORS, 4, "Deep Ocean", "Deep ocean blues"));
  addPalette(new ColorPalette(PredefinedPalettes::FOREST_COLORS, 4, "Forest Green", "Various greens"));
  addPalette(new ColorPalette(PredefinedPalettes::SUNSET_COLORS, 4, "Sunset", "Beautiful sunset"));
  addPalette(new ColorPalette(PredefinedPalettes::NEON_COLORS, 4, "Neon", "Bright neon colors"));
  addPalette(new ColorPalette(PredefinedPalettes::PASTEL_COLORS, 4, "Pastel", "Soft pastel colors"));

  Serial.print("Loaded ");
  Serial.print(paletteCount);
  Serial.println(" color palettes");
}

bool PaletteManager::addPalette(ColorPalette *palette)
{
  if (paletteCount >= MAX_PALETTES || !palette)
  {
    return false;
  }

  palettes[paletteCount] = palette;
  paletteCount++;
  return true;
}

ColorPalette *PaletteManager::getPalette(int index)
{
  if (index < 0 || index >= paletteCount)
  {
    return nullptr;
  }
  return palettes[index];
}

ColorPalette *PaletteManager::getPalette(const String &name)
{
  for (int i = 0; i < paletteCount; i++)
  {
    if (palettes[i]->getName().equalsIgnoreCase(name))
    {
      return palettes[i];
    }
  }
  return nullptr;
}

ColorPalette *PaletteManager::getCurrentPalette()
{
  if (currentPaletteIndex < 0 || currentPaletteIndex >= paletteCount)
  {
    return nullptr;
  }
  return palettes[currentPaletteIndex];
}

bool PaletteManager::setCurrentPalette(int index)
{
  if (index < 0 || index >= paletteCount)
  {
    return false;
  }
  currentPaletteIndex = index;
  return true;
}

bool PaletteManager::setCurrentPalette(const String &name)
{
  for (int i = 0; i < paletteCount; i++)
  {
    if (palettes[i]->getName().equalsIgnoreCase(name))
    {
      currentPaletteIndex = i;
      return true;
    }
  }
  return false;
}

String PaletteManager::getPaletteName(int index)
{
  if (index < 0 || index >= paletteCount)
  {
    return "";
  }
  return palettes[index]->getName();
}

void PaletteManager::nextPalette()
{
  currentPaletteIndex++;
  if (currentPaletteIndex >= paletteCount)
  {
    currentPaletteIndex = 0;
  }
}

void PaletteManager::previousPalette()
{
  currentPaletteIndex--;
  if (currentPaletteIndex < 0)
  {
    currentPaletteIndex = paletteCount - 1;
  }
}

void PaletteManager::printPalettes()
{
  Serial.println("=== Available Color Palettes ===");
  for (int i = 0; i < paletteCount; i++)
  {
    Serial.print(i);
    Serial.print(": ");
    Serial.print(palettes[i]->getName());
    Serial.print(" - ");
    Serial.println(palettes[i]->getDescription());
  }
  Serial.print("Current palette: ");
  Serial.println(getCurrentPalette() ? getCurrentPalette()->getName() : "None");
}

// ============================================================================
// Predefined Palette Colors
// ============================================================================

namespace PredefinedPalettes
{
  const CRGB WARM_COLORS[] = {
      CRGB::Red,
      CRGB::Orange,
      CRGB::Yellow,
      CRGB::OrangeRed};

  const CRGB COOL_COLORS[] = {
      CRGB::Blue,
      CRGB::Cyan,
      CRGB::Purple,
      CRGB::Indigo};

  const CRGB FIRE_COLORS[] = {
      CRGB::Black,
      CRGB::Red,
      CRGB::Orange,
      CRGB::Yellow};

  const CRGB OCEAN_COLORS[] = {
      CRGB::DarkBlue,
      CRGB::Blue,
      CRGB::DeepSkyBlue,
      CRGB::Aqua};

  const CRGB FOREST_COLORS[] = {
      CRGB::DarkGreen,
      CRGB::Green,
      CRGB::ForestGreen,
      CRGB::LimeGreen};

  const CRGB SUNSET_COLORS[] = {
      CRGB::Purple,
      CRGB::Red,
      CRGB::Orange,
      CRGB::Yellow};

  const CRGB NEON_COLORS[] = {
      CRGB::Magenta,
      CRGB::Cyan,
      CRGB::Yellow,
      CRGB::Lime};

  const CRGB PASTEL_COLORS[] = {
      CRGB(255, 182, 193), // Light Pink
      CRGB(173, 216, 230), // Light Blue
      CRGB(144, 238, 144), // Light Green
      CRGB(255, 218, 185)  // Peach
  };
}
