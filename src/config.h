#ifndef CONFIG_H
#define CONFIG_H

#include <stdint.h>

// ============================================================================
// LED CONFIGURATION & INDEXING SYSTEM
// ============================================================================
// Physical wiring: CLOCK (0-99) -> EYE rings (100-160)
// All code uses LOGICAL indices where 0 = 12 o'clock position
// Helper arrays (CLOCK_LED_MAP, EYE_X_LED_MAP) convert logical->raw indices

// Total LED counts
#define CLOCK_TOTAL_LEDS 100 // Clock ring LED count
#define EYE_TOTAL_LEDS 61    // Total LEDs in all eye rings
#define NUM_LEDS 161         // Total LED count (100 Clock + 61 Eye)

// Hardware configuration
#define DATA_PIN 13      // GPIO pin connected to LED data line
#define LED_TYPE WS2812B // LED type
#define COLOR_ORDER GRB  // Color order for LED strip

// Pole LED Configuration
#define POLE_NUM_LEDS 300 // Total LEDs in pole spiral strip
#define POLE_DATA_PIN 2   // GPIO pin connected to pole LED data line
#define POLE_LED_TYPE WS2812B
#define POLE_COLOR_ORDER GRB

// Pole Spiral Geometry
#define POLE_SPIRAL_REPEAT 13                                                                   // LEDs per spiral revolution (every 13 LEDs = same column)
#define POLE_SPIRAL_REVOLUTIONS ((POLE_NUM_LEDS + POLE_SPIRAL_REPEAT - 1) / POLE_SPIRAL_REPEAT) // ~23 revolutions
#define POLE_HALF_COLUMN_OFFSET 7                                                               // LED 7 is ~half way around from LED 0 (opposite side)
#define POLE_HEIGHT_LEVELS (POLE_SPIRAL_REVOLUTIONS)                                            // Height levels for vertical effects

// Joystick Configuration
#define JOYSTICK_X_PIN 34     // GPIO34 (ADC1_CH6) for X-axis
#define JOYSTICK_Y_PIN 35     // GPIO35 (ADC1_CH7) for Y-axis
#define JOYSTICK_BUTTON_PIN 4 // GPIO4 for joystick button

// ============================================================================
// RAW LED INDICES (Physical Wiring Order)
// ============================================================================
// Clock: Raw indices 0-99, starts at 6 o'clock, goes counter-clockwise
// Eye:   Raw indices 100-160, starts at 6 o'clock (EYE_4), goes clockwise

// Clock Ring - Raw Indices (counter-clockwise from 6 o'clock)
#define CLOCK_RAW_START 0
#define CLOCK_RAW_END 99
#define CLOCK_COUNT 100

// Eye Ring Raw Indices (clockwise from 6 o'clock, outermost first)
#define EYE_4_RAW_START 100 // Outermost ring
#define EYE_4_COUNT 24
#define EYE_4_RAW_END (EYE_4_RAW_START + EYE_4_COUNT - 1) // 124

#define EYE_3_RAW_START (EYE_4_RAW_END + 1) // 125
#define EYE_3_COUNT 16
#define EYE_3_RAW_END (EYE_3_RAW_START + EYE_3_COUNT - 1) // 140

#define EYE_2_RAW_START (EYE_3_RAW_END + 1) // 141
#define EYE_2_COUNT 12
#define EYE_2_RAW_END (EYE_2_RAW_START + EYE_2_COUNT - 1) // 152

#define EYE_1_RAW_START (EYE_2_RAW_END + 1) // 153
#define EYE_1_COUNT 8
#define EYE_1_RAW_END (EYE_1_RAW_START + EYE_1_COUNT - 1) // 160

#define EYE_0_RAW_START (EYE_1_RAW_END + 1) // 160 (center LED)
#define EYE_0_COUNT 1
#define EYE_0_RAW_END EYE_0_RAW_START // 160

// ============================================================================
// LOGICAL LED MAPPING ARRAYS
// ============================================================================
// These arrays map logical indices (0 = 12 o'clock) to raw indices
// Usage: leds[CLOCK_LED_MAP[logical_index]] = color;

extern const uint16_t CLOCK_LED_MAP[CLOCK_COUNT];
extern const uint16_t EYE_4_LED_MAP[EYE_4_COUNT];
extern const uint16_t EYE_3_LED_MAP[EYE_3_COUNT];
extern const uint16_t EYE_2_LED_MAP[EYE_2_COUNT];
extern const uint16_t EYE_1_LED_MAP[EYE_1_COUNT];

// Combined eye mapping array (all eye rings in one array, outer to inner)
// EYE_4[0-23] -> EYE_3[24-39] -> EYE_2[40-51] -> EYE_1[52-59] -> EYE_0[60]
extern const uint16_t EYE_TOTAL_LED_MAP[EYE_TOTAL_LEDS];

/**
 * @brief Convert logical LED index to raw LED index across entire strip
 * @param logicalIndex Logical index (0 = 12 o'clock on clock, 0-160 total)
 * @return Raw LED index for FastLED array
 *
 * Logical mapping:
 * - 0-99: Clock ring (logical 0 = 12 o'clock position)
 * - 100-160: Eye rings (logical 100 = 12 o'clock position on EYE_4)
 */
inline uint16_t logicalToRawIndex(uint16_t logicalIndex)
{
    if (logicalIndex < CLOCK_COUNT)
    {
        // Clock ring: Use CLOCK_LED_MAP
        return CLOCK_LED_MAP[logicalIndex];
    }
    else if (logicalIndex < NUM_LEDS)
    {
        // Eye rings: Use EYE_TOTAL_LED_MAP
        uint16_t eyeLogicalIndex = logicalIndex - CLOCK_COUNT;
        return EYE_TOTAL_LED_MAP[eyeLogicalIndex];
    }
    else
    {
        // Out of bounds, return 0 (safe fallback)
        return 0;
    }
}

// Segment Type Definitions
#define SEGMENT_CLOCK 0
#define SEGMENT_EYE_4 1
#define SEGMENT_EYE_3 2
#define SEGMENT_EYE_2 3
#define SEGMENT_EYE_1 4
#define SEGMENT_EYE_0 5

#define NUM_EYE_RINGS 5   // EYE_4, EYE_3, EYE_2, EYE_1, EYE_0
#define NUM_TOTAL_RINGS 6 // All eye rings + clock ring

// Debug and Testing Options
#define ENABLE_SEGMENT_DEBUG false // Enable segment debugging output
#define SEGMENT_TEST_INTERVAL 2000 // Segment test cycle interval (ms)

// Brightness and Speed Mode Settings
#define BRIGHTNESS_LEVELS 9 // Number of brightness levels (1-10)
#define SPEED_LEVELS 9      // Number of speed levels (1-10)

// ============================================================================
// BRIGHTNESS & SPEED PREVIEW LED POSITIONS (Pre-computed Raw Indices)
// ============================================================================
// These arrays store RAW LED indices by accessing the mapping arrays
// This provides O(1) lookup without needing SegmentManager at runtime

// Brightness preview: Vertical line from 6 o'clock (bottom, level 0) to 12 o'clock (top, level 8)
#define BRIGHTNESS_PREVIEW_LEDS 9
const uint16_t BRIGHTNESS_LED_POSITIONS[BRIGHTNESS_PREVIEW_LEDS] = {
    EYE_4_LED_MAP[12], // Level 0: EYE_4 at 6 o'clock (bottom) - logical 11 (24/2 - 1)
    EYE_3_LED_MAP[8],  // Level 1: EYE_3 at 6 o'clock - logical 7 (16/2 - 1)
    EYE_2_LED_MAP[6],  // Level 2: EYE_2 at 6 o'clock - logical 5 (12/2 - 1)
    EYE_1_LED_MAP[4],  // Level 3: EYE_1 at 6 o'clock - logical 3 (8/2 - 1)
    EYE_0_RAW_START,   // Level 4: EYE_0 center (raw 160)
    EYE_1_LED_MAP[0],  // Level 5: EYE_1 at 12 o'clock (top)
    EYE_2_LED_MAP[0],  // Level 6: EYE_2 at 12 o'clock
    EYE_3_LED_MAP[0],  // Level 7: EYE_3 at 12 o'clock
    EYE_4_LED_MAP[0]   // Level 8: EYE_4 at 12 o'clock
};

// Speed preview: Horizontal line from 9 o'clock (left, level 0) through center to 3 o'clock (right, level 8)
#define SPEED_PREVIEW_LEDS 9
const uint16_t SPEED_LED_POSITIONS[SPEED_PREVIEW_LEDS] = {
    EYE_4_LED_MAP[18], // Level 0: EYE_4 at 9 o'clock (left) - logical 17 (3*24/4 - 1)
    EYE_3_LED_MAP[12], // Level 1: EYE_3 at 9 o'clock - logical 11 (3*16/4 - 1)
    EYE_2_LED_MAP[9],  // Level 2: EYE_2 at 9 o'clock - logical 8 (3*12/4 - 1)
    EYE_1_LED_MAP[6],  // Level 3: EYE_1 at 9 o'clock - logical 5 (3*8/4 - 1)
    EYE_0_RAW_START,   // Level 4: EYE_0 center (raw 160)
    EYE_1_LED_MAP[2],  // Level 5: EYE_1 at 3 o'clock (right) - logical 1 (8/4 - 1)
    EYE_2_LED_MAP[3],  // Level 6: EYE_2 at 3 o'clock - logical 2 (12/4 - 1)
    EYE_3_LED_MAP[4],  // Level 7: EYE_3 at 3 o'clock - logical 3 (16/4 - 1)
    EYE_4_LED_MAP[6]   // Level 8: EYE_4 at 3 o'clock - logical 5 (24/4 - 1)
};

// Brightness settings (0-255)
#define DEFAULT_BRIGHTNESS 50
#define MAX_BRIGHTNESS 200

// Power Management and Safety Limits
#define MAX_CURRENT_MA 1500        // Maximum allowed current draw in milliamps
#define LED_CURRENT_MA_PER_LED 60  // Max current per WS2812B LED at full white (mA)
#define SAFETY_MARGIN_PERCENT 80   // Use only 80% of max current for safety
#define VOLTAGE_5V 5.0             // Operating voltage for LED strip
#define POWER_LIMIT_WATTS 25       // Maximum power consumption limit (watts)
#define ENABLE_POWER_LIMITING true // Enable/disable power limiting feature

// WiFi Access Point Configuration
#define ENABLE_WIFI_AP false        // Enable/disable WiFi access point
#define WIFI_AP_SSID "CLOCK"        // WiFi network name (SSID)
#define WIFI_AP_PASSWORD "$pankm3!" // WiFi password (min 8 characters)
#define WIFI_AP_CHANNEL 1           // WiFi channel (1-13)
#define WIFI_AP_MAX_CONNECTIONS 4   // Maximum simultaneous connections
#define WEB_SERVER_PORT 80          // HTTP server port
#define WEB_UPDATE_INTERVAL 1000    // Web status update interval (ms)

// Power Optimization Configuration
#define ENABLE_CPU_FREQUENCY_REDUCTION true // Reduce CPU frequency when wireless is disabled
#define POWER_OPTIMIZED_CPU_FREQ 160        // CPU frequency in MHz when power optimized (80, 160, or 240)

// Captive Portal Configuration
#define ENABLE_CAPTIVE_PORTAL false        // Enable captive portal functionality
#define DNS_SERVER_PORT 53                 // DNS server port for captive portal
#define CAPTIVE_PORTAL_TITLE "Clock Totem" // Title shown in captive portal

// Joystick calibration values
#define JOYSTICK_MIN 0        // Minimum ADC value (12-bit ADC)
#define JOYSTICK_MAX 4095     // Maximum ADC value (12-bit ADC)
#define JOYSTICK_CENTER 1790  // Center position (0-4095 range)
#define JOYSTICK_DEADZONE 300 // Deadzone around center (scaled for 4095 range)

// Update intervals
#define LED_UPDATE_INTERVAL 16    // ~60 FPS (16ms)
#define JOYSTICK_READ_INTERVAL 10 // Read joystick every 10ms

// Default static color (RGB values 0-255)
#define STATIC_COLOR_R 255
#define STATIC_COLOR_G 0
#define STATIC_COLOR_B 100

// ============================================================================
// MODE SYSTEM DEFINITIONS
// ============================================================================

// Main Mode Categories
#define MAIN_MODE_EXPLORER 0    // Color/Pattern Explorer Mode
#define MAIN_MODE_INTERACTION 1 // Interaction Modes
#define NUM_MAIN_MODES 2        // Number of main modes

// Explorer Mode Sub-Modes (Main Mode 0)
#define EXPLORER_SUBMODE_CLOCK_PATTERN 0  // Clock Pattern Explorer
#define EXPLORER_SUBMODE_CLOCK_SETTINGS 1 // Clock Brightness/Speed
#define EXPLORER_SUBMODE_POLE_PATTERN 2   // Pole Pattern Explorer
#define EXPLORER_SUBMODE_POLE_SETTINGS 3  // Pole Brightness/Speed
#define NUM_EXPLORER_SUBMODES 4           // Number of explorer sub-modes

// Interaction Mode Sub-Modes (Main Mode 1)
#define INTERACTION_SUBMODE_EYEBALL 0    // Eyeball tracking mode
#define INTERACTION_SUBMODE_FIREWORK 1   // Firework launch mode
#define INTERACTION_SUBMODE_JOLT 2       // Jolt magnitude mode
#define INTERACTION_SUBMODE_SPEED_CTRL 3 // Speed control mode
#define NUM_INTERACTION_SUBMODES 4       // Number of interaction sub-modes

// Special Modes (not in normal cycle)
#define SPECIAL_MODE_SETTINGS 99    // Settings mode - quadrant-based interface
#define SPECIAL_MODE_CALIBRATION 98 // Joystick calibration mode

// Current mode tracking
extern uint8_t currentMainMode;
extern uint8_t currentSubMode;

// Joystick sensitivity settings
#define BUTTON_DEBOUNCE_MS 200 // Button debounce time

// Firework mode settings
#define MAX_ACTIVE_FIREWORKS 5        // Maximum simultaneous firework instances
#define FIREWORK_LAUNCH_THRESHOLD 200 // Joystick Y threshold to launch firework
#define FIREWORK_MODE_TIMEOUT 30000   // Auto-exit firework mode after 30 seconds

// Jolt Mode Settings
#define JOLT_MAGNITUDE_LEVELS 5     // Number of magnitude levels (1-5)
#define JOLT_DEADZONE_THRESHOLD 50  // Deadzone threshold for center activation
#define JOLT_LEVEL_1_THRESHOLD 300  // Level 1 threshold (just outside deadzone)
#define JOLT_LEVEL_2_THRESHOLD 700  // Level 2 threshold
#define JOLT_LEVEL_3_THRESHOLD 1100 // Level 3 threshold
#define JOLT_LEVEL_4_THRESHOLD 1500 // Level 4 threshold
#define JOLT_LEVEL_5_THRESHOLD 2200 // Level 5 threshold (near max magnitude from center to top)

// Pole-specific settings (separate from clock settings)
#define DEFAULT_POLE_BRIGHTNESS 128 // Default pole brightness (0-255)
#define DEFAULT_POLE_SPEED 0.5f     // Default pole speed multiplier
#define POLE_BRIGHTNESS_MIN 10      // Minimum pole brightness
#define POLE_BRIGHTNESS_MAX 255     // Maximum pole brightness
#define POLE_SPEED_MIN 0.1f         // Minimum pole speed
#define POLE_SPEED_MAX 5.0f         // Maximum pole speed

// Pointer mode settings
#define POINTER_LED_COUNT 3    // Number of LEDs to light up in pointer mode (deprecated - use width)
#define POINTER_BRIGHTNESS 100 // Brightness for pointer LEDs
#define POINTER_WIDTH_MIN 1    // Minimum pointer width (light touch)
#define POINTER_WIDTH_MAX 5    // Maximum pointer width (full deflection)

// Pointer highlight colors
#define POINTER_COLOR_HTML CRGB::HTMLColorCode::White
#define POINTER_COLOR_BRIGHTNESS 100 // Brightness for pointer LEDs

// Pointer background colors
#define POINTER_BG_COLOR_HTML CRGB::HTMLColorCode::Black
#define POINTER_BG_BRIGHTNESS 30 // Brightness for background LEDs

// Joystick Button Settings
#define BUTTON_HOLD_DURATION 1000 // Duration to hold button for mode change (ms)
#define BUTTON_HOLD_THRESHOLD 100 // Minimum hold time to distinguish from bounce (ms)

// Calibration settings
#define CALIBRATION_TIMEOUT 20000  // Auto-exit calibration after 10 seconds
#define CALIBRATION_BLINK_RATE 200 // Blink rate during calibration (ms)
#define MIN_JOYSTICK_RANGE 100     // Minimum acceptable range for calibration

// Settings Mode Configuration
#define SETTINGS_HOLD_WARNING_MS 1000 // Time before selection warning (flash effect)
#define SETTINGS_HOLD_SELECT_MS 2000  // Time to complete selection
#define SETTINGS_FLASH_INTERVAL 100   // Flash interval during selection warning (ms)
#define SETTINGS_BRIGHTNESS_MIN 1     // Minimum global brightness
#define SETTINGS_BRIGHTNESS_MAX 255   // Maximum global brightness
#define SETTINGS_SPEED_MIN 1.0f       // Minimum global speed
#define SETTINGS_SPEED_MAX 5.0f       // Maximum global speed
#define DEFAULT_GLOBAL_SPEED 1.0f     // Default global speed
#define DEFAULT_GLOBAL_BRIGHTNESS 128 // Default global brightness (50% of max)
#define SETTINGS_MAX_ITEMS 12         // Maximum items to display (patterns/palettes/speeds)

// Settings Quadrant Colors
#define SETTINGS_BRIGHTNESS_COLOR CRGB::Red // Quadrant 1 - Brightness
#define SETTINGS_SPEED_COLOR CRGB::Blue     // Quadrant 2 - Speed
#define SETTINGS_PATTERN_COLOR CRGB::Green  // Quadrant 3 - Pattern
#define SETTINGS_PALETTE_COLOR CRGB::Purple // Quadrant 4 - Palette
#define SETTINGS_TICK_COLOR CRGB::Red       // Color for selection ticks
#define SETTINGS_POINTER_FLASH_MIN 10       // Minimum brightness during flash
#define SETTINGS_POINTER_FLASH_MAX 100      // Maximum brightness during flash

#define SPEED_CONTROL_DEADZONE_SPEED 0.4f // Speed control deadzone speed
#define SPEED_CONTROL_MIN_SPEED 0.4f      // Minimum speed control speed
#define SPEED_CONTROL_MAX_SPEED 5.0f      // Maximum speed control speed

#endif // CONFIG_H
