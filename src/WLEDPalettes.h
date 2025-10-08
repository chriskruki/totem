#ifndef WLED_PALETTES_H
#define WLED_PALETTES_H

#include <FastLED.h>

/**
 * @file WLEDPalettes.h
 * @brief WLED Color Palette Definitions
 *
 * This file contains gradient palette definitions extracted from WLED.
 * These palettes are designed for use with FastLED's CRGBPalette16 and
 * integrate seamlessly with the existing ColorPalette system.
 *
 * Source: WLED project (https://github.com/Aircoookie/WLED)
 * Each palette is defined as a gradient with position/color pairs.
 */

namespace WLEDPalettes
{
  // ============================================================================
  // WLED Gradient Palette Definitions
  // ============================================================================
  // Format: {position (0-255), R, G, B, ...}
  // Use with: CRGBPalette16 palette = palette_name;

  /**
   * @brief Pink Candy - Sweet pink gradient
   * Soft pinks transitioning through coral to hot pink
   */
  DEFINE_GRADIENT_PALETTE(pink_candy_gp){
      0, 255, 192, 203,   // Light pink
      85, 255, 182, 193,  // Pink
      170, 255, 105, 180, // Hot pink
      255, 255, 20, 147   // Deep pink
  };

  /**
   * @brief Hult - Chris Hults signature palette
   * Rich blend of purples, blues, and warm tones
   */
  DEFINE_GRADIENT_PALETTE(hult_gp){
      0, 3, 3, 3,
      10, 12, 10, 8,
      35, 32, 28, 20,
      60, 60, 45, 25,
      93, 95, 55, 35,
      128, 130, 75, 45,
      160, 110, 85, 125,
      193, 80, 95, 205,
      226, 40, 55, 135,
      255, 10, 20, 70};

  /**
   * @brief Fairy Reaf - Underwater coral reef colors
   * Aqua blues transitioning through coral pinks
   */
  DEFINE_GRADIENT_PALETTE(fairy_reaf_gp){
      0, 1, 70, 80,       // Deep aqua
      50, 1, 100, 120,    // Light aqua
      100, 50, 130, 140,  // Cyan
      150, 120, 160, 170, // Light cyan
      200, 180, 100, 120, // Coral pink
      255, 255, 30, 80    // Hot coral
  };

  /**
   * @brief Sunset - Vibrant sunset gradient
   * Deep purple through orange to yellow
   */
  DEFINE_GRADIENT_PALETTE(sunset_gp){
      0, 120, 0, 0,      // Deep red
      60, 180, 50, 0,    // Red-orange
      120, 255, 100, 0,  // Orange
      180, 255, 180, 0,  // Yellow-orange
      240, 255, 255, 0,  // Yellow
      255, 255, 255, 100 // Pale yellow
  };

  /**
   * @brief Atlantica - Deep ocean blues and greens
   * Mysterious underwater color scheme
   */
  DEFINE_GRADIENT_PALETTE(atlantica_gp){
      0, 0, 28, 112,     // Deep blue
      60, 32, 96, 255,   // Ocean blue
      120, 0, 150, 160,  // Teal
      180, 0, 200, 200,  // Cyan
      240, 0, 255, 160,  // Aqua
      255, 128, 255, 200 // Light aqua
  };

  /**
   * @brief Cloud - Soft white and gray gradient
   * Subtle whites transitioning through grays
   */
  DEFINE_GRADIENT_PALETTE(cloud_gp){
      0, 70, 130, 180,    // Steel blue
      64, 135, 206, 250,  // Light sky blue
      128, 240, 248, 255, // Alice blue (very light)
      192, 255, 255, 255, // Pure white
      255, 220, 220, 220  // Light gray
  };

  /**
   * @brief Sherbet - Pastel rainbow gradient
   * Soft pastel colors like rainbow sherbet ice cream
   */
  DEFINE_GRADIENT_PALETTE(sherbet_gp){
      0, 255, 182, 193,   // Light pink
      40, 255, 218, 185,  // Peach
      80, 255, 255, 153,  // Pastel yellow
      120, 204, 255, 204, // Pastel green
      160, 179, 229, 252, // Pastel blue
      200, 230, 190, 255, // Pastel purple
      255, 255, 182, 193  // Light pink (loop)
  };

  // ============================================================================
  // Palette Registry
  // ============================================================================
  // Array of all WLED gradient palettes for easy registration

  struct WLEDPaletteEntry
  {
    const TProgmemRGBGradientPalette_byte *palette;
    const char *name;
    const char *description;
  };

  const WLEDPaletteEntry WLED_PALETTES[] = {
      {pink_candy_gp, "Pink Candy", "Sweet pink gradient"},
      {hult_gp, "Hult", "Chris Hult signature palette"},
      {fairy_reaf_gp, "Fairy Reaf", "Underwater coral reef"},
      {sunset_gp, "Sunset", "Vibrant sunset colors"},
      {atlantica_gp, "Atlantica", "Deep ocean blues"},
      {cloud_gp, "Cloud", "Soft white and gray"},
      {sherbet_gp, "Sherbet", "Pastel rainbow sherbet"}};

  const int WLED_PALETTE_COUNT = sizeof(WLED_PALETTES) / sizeof(WLEDPaletteEntry);

} // namespace WLEDPalettes

#endif // WLED_PALETTES_H
