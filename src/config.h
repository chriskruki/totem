#ifndef CONFIG_H
#define CONFIG_H

// LED Configuration
#define NUM_LEDS 162     // Total LED count (61 Eye + 101 Clock)
#define DATA_PIN 13      // GPIO pin connected to LED data line
#define LED_TYPE WS2812B // Change if using different LED type (APA102, WS2811, etc.)
#define COLOR_ORDER GRB  // Color order for your LED strip

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

// LED Segment Configuration
// Data flow: EYE_4 (outer) -> EYE_3 -> EYE_2 -> EYE_1 -> EYE_0 (center) -> CLOCK
#define EYE_TOTAL_LEDS 61    // Total LEDs in all eye rings
#define CLOCK_TOTAL_LEDS 101 // Total LEDs in clock ring

// Eye Ring Definitions (LED indices)
#define EYE_4_START 0  // Outermost eye ring startpio
#define EYE_4_COUNT 24 // LEDs in ring 4
#define EYE_4_END (EYE_4_START + EYE_4_COUNT - 1)

#define EYE_3_START (EYE_4_END + 1) // Ring 3 start
#define EYE_3_COUNT 16              // LEDs in ring 3
#define EYE_3_END (EYE_3_START + EYE_3_COUNT - 1)

#define EYE_2_START (EYE_3_END + 1) // Ring 2 start
#define EYE_2_COUNT 12              // LEDs in ring 2
#define EYE_2_END (EYE_2_START + EYE_2_COUNT - 1)

#define EYE_1_START (EYE_2_END + 1) // Ring 1 start
#define EYE_1_COUNT 8               // LEDs in ring 1
#define EYE_1_END (EYE_1_START + EYE_1_COUNT - 1)

#define EYE_0_START (EYE_1_END + 1) // Center LED (ring 0)
#define EYE_0_COUNT 1               // Single center LED
#define EYE_0_END (EYE_0_START + EYE_0_COUNT - 1)

// Clock Ring Definition
#define CLOCK_START (EYE_0_END + 1) // Clock ring start (LED 61)
#define CLOCK_COUNT 101             // LEDs in clock ring
#define CLOCK_END (CLOCK_START + CLOCK_COUNT - 1)

// Ring Count
#define NUM_EYE_RINGS 5   // EYE_4, EYE_3, EYE_2, EYE_1, EYE_0
#define NUM_TOTAL_RINGS 6 // All eye rings + clock ring

// Segment Type Definitions
#define SEGMENT_EYE_4 0
#define SEGMENT_EYE_3 1
#define SEGMENT_EYE_2 2
#define SEGMENT_EYE_1 3
#define SEGMENT_EYE_0 4
#define SEGMENT_CLOCK 5

// Debug and Testing Options
#define ENABLE_SEGMENT_DEBUG false // Enable segment debugging output
#define SEGMENT_TEST_INTERVAL 2000 // Segment test cycle interval (ms)

// Direction Reversal Options
#define REVERSE_EYE_DIRECTION true // Reverse eye LED direction to match clock direction

// Brightness and Speed Mode Settings
#define BRIGHTNESS_LEVELS 9 // Number of brightness levels (1-10)
#define SPEED_LEVELS 9      // Number of speed levels (1-10)

// Brightness preview LEDs (vertical column in eye rings)
#define BRIGHTNESS_PREVIEW_LEDS 9
const uint16_t BRIGHTNESS_LED_POSITIONS[BRIGHTNESS_PREVIEW_LEDS] = {
    0,  // EYE_4 top
    24, // EYE_3 top
    40, // EYE_2 top
    52, // EYE_1 top
    60, // EYE_0 center
    56, // EYE_1 bottom
    46, // EYE_2 bottom
    32, // EYE_3 bottom
    12, // EYE_4 bottom
};

// Speed preview LEDs (horizontal line in eye rings)
#define SPEED_PREVIEW_LEDS 9
const uint16_t SPEED_LED_POSITIONS[SPEED_PREVIEW_LEDS] = {
    6,  // EYE_4 left
    28, // EYE_3 left
    43, // EYE_2 left
    54, // EYE_1 left
    60, // EYE_0 center
    58, // EYE_1 right
    49, // EYE_2 right
    36, // EYE_3 right
    18  // EYE_4 right
};

// Brightness settings (0-255)
#define DEFAULT_BRIGHTNESS 50
#define MAX_BRIGHTNESS 255

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
#define INTERACTION_SUBMODE_EYEBALL 0  // Eyeball tracking mode
#define INTERACTION_SUBMODE_FIREWORK 1 // Firework launch mode
#define INTERACTION_SUBMODE_JOLT 2     // Jolt magnitude mode
#define NUM_INTERACTION_SUBMODES 3     // Number of interaction sub-modes

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

// Calibration settings
#define DOUBLE_CLICK_TIMEOUT 500   // Max time between clicks for double-click (ms)
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

#endif // CONFIG_H
