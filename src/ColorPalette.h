#ifndef COLOR_PALETTE_H
#define COLOR_PALETTE_H

#include <Arduino.h>
#include <FastLED.h>

/**
 * @brief Color palette class for managing collections of colors
 *
 * This class provides functionality for creating, managing, and using
 * color palettes with LED patterns. It supports both predefined FastLED
 * palettes and custom color arrays.
 */
class ColorPalette
{
private:
  CRGBPalette16 palette; // FastLED palette (16 colors)
  String name;           // Palette name
  String description;    // Palette description
  bool isCustom;         // True if custom palette, false if predefined

public:
  /**
   * @brief Constructor for predefined FastLED palette
   * @param palette FastLED palette
   * @param name Palette name
   * @param description Palette description
   */
  ColorPalette(const CRGBPalette16 &palette, const String &name, const String &description);

  /**
   * @brief Constructor for custom color array
   * @param colors Array of CRGB colors
   * @param numColors Number of colors in array (max 16)
   * @param name Palette name
   * @param description Palette description
   */
  ColorPalette(const CRGB *colors, int numColors, const String &name, const String &description);

  /**
   * @brief Get color from palette at specific index
   * @param index Color index (0-255)
   * @return CRGB color
   */
  CRGB getColor(uint8_t index) const;

  /**
   * @brief Get color from palette with smooth blending
   * @param position Position in palette (0.0 to 1.0)
   * @return CRGB color with smooth interpolation
   */
  CRGB getColorSmooth(float position) const;

  /**
   * @brief Get the FastLED palette
   * @return Reference to CRGBPalette16
   */
  const CRGBPalette16 &getPalette() const { return palette; }

  /**
   * @brief Get palette name
   * @return Palette name
   */
  String getName() const { return name; }

  /**
   * @brief Get palette description
   * @return Palette description
   */
  String getDescription() const { return description; }

  /**
   * @brief Check if palette is custom
   * @return true if custom palette
   */
  bool getIsCustom() const { return isCustom; }

  /**
   * @brief Update palette with new colors
   * @param colors Array of CRGB colors
   * @param numColors Number of colors in array
   */
  void updateColors(const CRGB *colors, int numColors);

  /**
   * @brief Create a gradient palette between two colors
   * @param color1 Starting color
   * @param color2 Ending color
   * @param name Palette name
   * @param description Palette description
   * @return ColorPalette with gradient
   */
  static ColorPalette createGradient(CRGB color1, CRGB color2, const String &name, const String &description);

  /**
   * @brief Create a palette from HSV range
   * @param startHue Starting hue (0-255)
   * @param endHue Ending hue (0-255)
   * @param saturation Saturation (0-255)
   * @param value Value/brightness (0-255)
   * @param name Palette name
   * @param description Palette description
   * @return ColorPalette with HSV range
   */
  static ColorPalette createHSVRange(uint8_t startHue, uint8_t endHue, uint8_t saturation, uint8_t value, const String &name, const String &description);
};

/**
 * @brief Palette manager class for handling multiple color palettes
 */
class PaletteManager
{
private:
  static const int MAX_PALETTES = 30; // Increased to accommodate WLED palettes
  ColorPalette *palettes[MAX_PALETTES];
  int paletteCount;
  int currentPaletteIndex;

public:
  /**
   * @brief Constructor
   */
  PaletteManager();

  /**
   * @brief Destructor
   */
  ~PaletteManager();

  /**
   * @brief Initialize with predefined palettes
   */
  void initialize();

  /**
   * @brief Add a palette to the manager
   * @param palette Pointer to ColorPalette
   * @return true if added successfully
   */
  bool addPalette(ColorPalette *palette);

  /**
   * @brief Get palette by index
   * @param index Palette index
   * @return Pointer to ColorPalette or nullptr if invalid
   */
  ColorPalette *getPalette(int index);

  /**
   * @brief Get palette by name
   * @param name Palette name
   * @return Pointer to ColorPalette or nullptr if not found
   */
  ColorPalette *getPalette(const String &name);

  /**
   * @brief Get current palette
   * @return Pointer to current ColorPalette
   */
  ColorPalette *getCurrentPalette();

  /**
   * @brief Set current palette by index
   * @param index Palette index
   * @return true if set successfully
   */
  bool setCurrentPalette(int index);

  /**
   * @brief Set current palette by name
   * @param name Palette name
   * @return true if set successfully
   */
  bool setCurrentPalette(const String &name);

  /**
   * @brief Get number of palettes
   * @return Number of palettes
   */
  int getPaletteCount() const { return paletteCount; }

  /**
   * @brief Get current palette index
   * @return Current palette index
   */
  int getCurrentPaletteIndex() const { return currentPaletteIndex; }

  /**
   * @brief Get palette name by index
   * @param index Palette index
   * @return Palette name or empty string if invalid
   */
  String getPaletteName(int index);

  /**
   * @brief Cycle to next palette
   */
  void nextPalette();

  /**
   * @brief Cycle to previous palette
   */
  void previousPalette();

  /**
   * @brief Print all palette names to Serial
   */
  void printPalettes();
};

// Predefined palette constants
namespace PredefinedPalettes
{
  extern const CRGB WARM_COLORS[];
  extern const CRGB COOL_COLORS[];
  extern const CRGB FIRE_COLORS[];
  extern const CRGB OCEAN_COLORS[];
  extern const CRGB FOREST_COLORS[];
  extern const CRGB SUNSET_COLORS[];
  extern const CRGB NEON_COLORS[];
  extern const CRGB PASTEL_COLORS[];
}

#endif // COLOR_PALETTE_H
