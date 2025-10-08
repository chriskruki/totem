/**
 * @file LEDMapping.cpp
 * @brief LED logical-to-raw index mapping arrays
 *
 * This file contains pre-computed mapping arrays that convert logical LED indices
 * (where 0 = 12 o'clock position) to raw hardware indices (physical wiring order).
 *
 * Physical Wiring:
 * - Clock: Raw 0-99, starts at 6 o'clock, goes counter-clockwise
 * - Eye:   Raw 100-160, starts at 6 o'clock (EYE_4), goes clockwise
 *
 * Logical Indexing:
 * - All arrays start at logical 0 = 12 o'clock (top)
 * - Provides consistent orientation across all rings
 */

#include "config.h"
#include <Arduino.h>

// ============================================================================
// CLOCK LED MAPPING (100 LEDs)
// ============================================================================
// Clock physical: Raw 0 at 6 o'clock, counter-clockwise
// Logical 0 = 12 o'clock = Raw 50 (halfway from raw 0)

const uint16_t CLOCK_LED_MAP[CLOCK_COUNT] = {
    // Logical index 0-24: 12 o'clock to 3 o'clock (right, 25 LEDs)
    50, 49, 48, 47, 46, 45, 44, 43, 42, 41, 40, 39, 38, 37, 36, 35, 34, 33, 32, 31, 30, 29, 28, 27, 26,

    // Logical index 25-49: 3 o'clock to 6 o'clock (bottom, 25 LEDs)
    25, 24, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0,

    // Logical index 50-74: 6 o'clock to 9 o'clock (left, 25 LEDs)
    99, 98, 97, 96, 95, 94, 93, 92, 91, 90, 89, 88, 87, 86, 85, 84, 83, 82, 81, 80, 79, 78, 77, 76, 75,

    // Logical index 75-99: 9 o'clock to 12 o'clock (back to top, 25 LEDs)
    74, 73, 72, 71, 70, 69, 68, 67, 66, 65, 64, 63, 62, 61, 60, 59, 58, 57, 56, 55, 54, 53, 52, 51};

// ============================================================================
// EYE_4 LED MAPPING (24 LEDs - Outermost Ring)
// ============================================================================
// Physical: Raw 100 at 6 o'clock, clockwise
// Logical 0 = 12 o'clock = Raw 111 (100 + 11, adjusted for alignment)

const uint16_t EYE_4_LED_MAP[EYE_4_COUNT] = {
    // Logical index 0-5: 12 o'clock to 3 o'clock (right)
    112, 113, 114, 115, 116, 117,

    // Logical index 6-11: 3 o'clock to 6 o'clock (bottom)
    118, 119, 120, 121, 122, 123,

    // Logical index 12-17: 6 o'clock to 9 o'clock (left)
    100, 101, 102, 103, 104, 105,

    // Logical index 18-23: 9 o'clock to 12 o'clock (back to top)
    106, 107, 108, 109, 110, 111};

// ============================================================================
// EYE_3 LED MAPPING (16 LEDs)
// ============================================================================
// Physical: Raw 124 at 6 o'clock, clockwise
// Logical 0 = 12 o'clock = Raw 131 (124 + 7, adjusted for alignment)

const uint16_t EYE_3_LED_MAP[EYE_3_COUNT] = {
    // Logical index 0-3: 12 o'clock to 3 o'clock (right)
    132, 133, 134, 135,

    // Logical index 4-7: 3 o'clock to 6 o'clock (bottom)
    136, 137, 138, 139,

    // Logical index 8-11: 6 o'clock to 9 o'clock (left)
    124, 125, 126, 127,

    // Logical index 12-15: 9 o'clock to 12 o'clock (back to top)
    128, 129, 130, 131};

// ============================================================================
// EYE_2 LED MAPPING (12 LEDs)
// ============================================================================
// Physical: Raw 140 at 6 o'clock, clockwise
// Logical 0 = 12 o'clock = Raw 145 (140 + 5, adjusted for alignment)

const uint16_t EYE_2_LED_MAP[EYE_2_COUNT] = {
    // Logical index 0-2: 12 o'clock to 3 o'clock (right)
    146, 147, 148,

    // Logical index 3-5: 3 o'clock to 6 o'clock (bottom)
    149, 150, 151,

    // Logical index 6-8: 6 o'clock to 9 o'clock (left)
    140, 141, 142,

    // Logical index 9-11: 9 o'clock to 12 o'clock (back to top)
    143, 144, 145};

// ============================================================================
// EYE_1 LED MAPPING (8 LEDs)
// ============================================================================
// Physical: Raw 152 at 6 o'clock, clockwise
// Logical 0 = 12 o'clock = Raw 155 (152 + 3, adjusted for alignment)

const uint16_t EYE_1_LED_MAP[EYE_1_COUNT] = {
    // Logical index 0-1: 12 o'clock to 3 o'clock (right)
    156, 157,

    // Logical index 2-3: 3 o'clock to 6 o'clock (bottom)
    158, 159,

    // Logical index 4-5: 6 o'clock to 9 o'clock (left)
    152, 153,

    // Logical index 6-7: 9 o'clock to 12 o'clock (back to top)
    154, 155};

// ============================================================================
// COMBINED EYE LED MAPPING (All eye rings, outer to inner - 61 LEDs total)
// ============================================================================
// This array combines all eye ring mappings into a single contiguous array
// Index 0-23:   EYE_4 (outermost)
// Index 24-39:  EYE_3
// Index 40-51:  EYE_2
// Index 52-59:  EYE_1
// Index 60:     EYE_0 (center)

const uint16_t EYE_TOTAL_LED_MAP[EYE_TOTAL_LEDS] = {
    // EYE_4: Index 0-23 (24 LEDs)
    112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111,

    // EYE_3: Index 24-39 (16 LEDs)
    132, 133, 134, 135, 136, 137, 138, 139, 124, 125, 126, 127, 128, 129, 130, 131,

    // EYE_2: Index 40-51 (12 LEDs)
    146, 147, 148, 149, 150, 151, 140, 141, 142, 143, 144, 145,

    // EYE_1: Index 52-59 (8 LEDs)
    156, 157, 158, 159, 152, 153, 154, 155,

    // EYE_0: Index 60 (1 LED - center)
    160};