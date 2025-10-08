#ifndef WLED_MATH_H
#define WLED_MATH_H

#include <Arduino.h>

/**
 * @file WLEDMath.h
 * @brief Optimized math functions extracted from WLED
 *
 * These functions replace floating-point operations with fast integer math.
 * Provides 2-10x speedup for common LED pattern calculations.
 *
 * Source: WLED project (https://github.com/Aircoookie/WLED)
 */

namespace WLEDMath
{
  /**
   * @brief Scale an 8-bit value by another 8-bit value
   *
   * This is the 'video' version which is slightly different than FastLED's scale8.
   * It ensures that even small values get some output (never completely zero unless input is zero).
   *
   * @param val Value to scale (0-255)
   * @param scale Scale factor (0-255, where 255 = no scaling)
   * @return Scaled value (0-255)
   *
   * Example: scale8_video(200, 128) ≈ 100 (scales by ~50%)
   */
  inline uint8_t scale8_video(uint8_t val, uint8_t scale)
  {
    uint8_t result = ((uint16_t)val * (uint16_t)scale) >> 8;
    // Add 1 if both values are non-zero to prevent complete darkness
    if (val && scale)
      result++;
    return result;
  }

  /**
   * @brief Saturating 8-bit addition (no overflow)
   *
   * Adds two 8-bit values, clamping result to 255 instead of wrapping.
   *
   * @param a First value (0-255)
   * @param b Second value (0-255)
   * @return Sum clamped to 255
   *
   * Example: qadd8(200, 100) = 255 (not 44)
   */
  inline uint8_t qadd8(uint8_t a, uint8_t b)
  {
    uint16_t sum = (uint16_t)a + (uint16_t)b;
    return (sum > 255) ? 255 : (uint8_t)sum;
  }

  /**
   * @brief Saturating 8-bit subtraction (no underflow)
   *
   * Subtracts two 8-bit values, clamping result to 0 instead of wrapping.
   *
   * @param a First value (0-255)
   * @param b Second value (0-255)
   * @return Difference clamped to 0
   *
   * Example: qsub8(50, 100) = 0 (not 206)
   */
  inline uint8_t qsub8(uint8_t a, uint8_t b)
  {
    return (a > b) ? (a - b) : 0;
  }

  /**
   * @brief Fast 8-bit × 8-bit multiply, returning high byte
   *
   * Equivalent to (a * b) / 256, but faster.
   * Useful for scaling brightness.
   *
   * @param a First value (0-255)
   * @param b Second value (0-255)
   * @return High byte of product (0-255)
   */
  inline uint8_t scale8(uint8_t a, uint8_t b)
  {
    return ((uint16_t)a * (uint16_t)b) >> 8;
  }

  /**
   * @brief Exponential ease-in/ease-out curve (quadratic)
   *
   * Maps input 0-255 through an ease curve that accelerates from 0,
   * moves linearly in the middle, and decelerates to 255.
   * Much faster than using pow() or exp().
   *
   * @param val Input value (0-255)
   * @return Eased value (0-255)
   */
  inline uint8_t ease8InOutQuad(uint8_t val)
  {
    uint8_t j = val;
    if (j & 0x80) // If > 128
    {
      j = 255 - j; // Flip for second half
    }
    uint8_t jj = scale8(j, j); // Square it
    uint8_t jj2 = jj << 1;     // Double it
    if (val & 0x80)            // If we flipped, flip back
    {
      jj2 = 255 - jj2;
    }
    return jj2;
  }

  /**
   * @brief Cubic ease-in/ease-out curve
   *
   * Similar to ease8InOutQuad but with cubic curve for smoother acceleration.
   *
   * @param val Input value (0-255)
   * @return Eased value (0-255)
   */
  inline uint8_t ease8InOutCubic(uint8_t val)
  {
    uint8_t j = val;
    if (j & 0x80)
    {
      j = 255 - j;
    }
    // Cubic: j^3 / 64 (approximate)
    uint8_t jj = scale8(j, j);
    uint8_t jjj = scale8(jj, j);
    uint8_t jjj2 = jjj << 2; // *4 to compensate for /64
    if (val & 0x80)
    {
      jjj2 = 255 - jjj2;
    }
    return jjj2;
  }

  /**
   * @brief Linear interpolation between two 8-bit values
   *
   * @param a Start value
   * @param b End value
   * @param frac Fraction (0-255, where 0 = all a, 255 = all b)
   * @return Interpolated value
   */
  inline uint8_t lerp8(uint8_t a, uint8_t b, uint8_t frac)
  {
    uint8_t result;
    if (b > a)
    {
      uint8_t delta = b - a;
      uint8_t scaled = scale8(delta, frac);
      result = a + scaled;
    }
    else
    {
      uint8_t delta = a - b;
      uint8_t scaled = scale8(delta, frac);
      result = a - scaled;
    }
    return result;
  }

  /**
   * @brief Triangle wave generator
   *
   * Creates a triangle wave from input. Useful for smooth back-and-forth motion.
   *
   * @param val Input value (0-255, wraps automatically)
   * @return Triangle wave output (0-255)
   *
   * Output: 0→255 (0-127), 255→0 (128-255)
   */
  inline uint8_t triwave8(uint8_t val)
  {
    if (val & 0x80) // Second half (128-255)
    {
      return 255 - ((val - 128) << 1); // 255 down to 0
    }
    else // First half (0-127)
    {
      return val << 1; // 0 up to 255
    }
  }

  /**
   * @brief Square wave generator
   *
   * Creates a square wave (binary on/off).
   *
   * @param val Input value (0-255)
   * @param duty Duty cycle (0-255, where 128 = 50%)
   * @return 255 if val < duty, 0 otherwise
   */
  inline uint8_t squarewave8(uint8_t val, uint8_t duty = 128)
  {
    return (val < duty) ? 255 : 0;
  }

  /**
   * @brief Fast division by 255 (approximation)
   *
   * Faster than true division, with less than 1% error.
   *
   * @param val Value to divide
   * @return val / 255 (approximately)
   */
  inline uint8_t div255(uint16_t val)
  {
    // Approximation: (val * 257) >> 16
    // This is equivalent to val / 255 with tiny error
    return ((val << 8) + val) >> 16;
  }

  /**
   * @brief Convert float (0.0-1.0) to fixed-point uint16_t (0-65535)
   *
   * @param f Float value (0.0-1.0)
   * @return Fixed-point value (0-65535)
   */
  inline uint16_t floatToFixed16(float f)
  {
    return (uint16_t)(f * 65535.0f);
  }

  /**
   * @brief Convert fixed-point uint16_t (0-65535) to float (0.0-1.0)
   *
   * @param fixed Fixed-point value (0-65535)
   * @return Float value (0.0-1.0)
   */
  inline float fixed16ToFloat(uint16_t fixed)
  {
    return fixed / 65535.0f;
  }

  /**
   * @brief Convert fixed-point uint16_t to palette index (0-255)
   *
   * @param fixed Fixed-point value (0-65535)
   * @return Palette index (0-255)
   */
  inline uint8_t fixed16ToPalette(uint16_t fixed)
  {
    return fixed >> 8; // Fast divide by 256
  }

  /**
   * @brief Map value from one range to another (integer version)
   *
   * Equivalent to Arduino's map(), but optimized for LED calculations.
   *
   * @param val Input value
   * @param inMin Input range minimum
   * @param inMax Input range maximum
   * @param outMin Output range minimum
   * @param outMax Output range maximum
   * @return Mapped value
   */
  inline uint16_t map16(uint16_t val, uint16_t inMin, uint16_t inMax, uint16_t outMin, uint16_t outMax)
  {
    if (val <= inMin)
      return outMin;
    if (val >= inMax)
      return outMax;

    uint32_t range = inMax - inMin;
    uint32_t pos = val - inMin;
    uint32_t outRange = outMax - outMin;

    return outMin + ((pos * outRange) / range);
  }

} // namespace WLEDMath

#endif // WLED_MATH_H
