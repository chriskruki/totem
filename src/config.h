#ifndef CONFIG_H
#define CONFIG_H

// LED Configuration
#define NUM_LEDS 61      // Adjust based on your LED strip length
#define DATA_PIN 13      // GPIO pin connected to LED data line
#define LED_TYPE WS2812B // Change if using different LED type (APA102, WS2811, etc.)
#define COLOR_ORDER GRB  // Color order for your LED strip

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
#define ENABLE_WIFI_AP true         // Enable/disable WiFi access point
#define WIFI_AP_SSID "CLOCK"        // WiFi network name (SSID)
#define WIFI_AP_PASSWORD "$pankm3!" // WiFi password (min 8 characters)
#define WIFI_AP_CHANNEL 1           // WiFi channel (1-13)
#define WIFI_AP_MAX_CONNECTIONS 4   // Maximum simultaneous connections
#define WEB_SERVER_PORT 80          // HTTP server port
#define WEB_UPDATE_INTERVAL 1000    // Web status update interval (ms)

// Captive Portal Configuration
#define ENABLE_CAPTIVE_PORTAL true         // Enable captive portal functionality
#define DNS_SERVER_PORT 53                 // DNS server port for captive portal
#define CAPTIVE_PORTAL_TITLE "Clock Totem" // Title shown in captive portal

// Joystick Configuration
#define JOYSTICK_X_PIN 34     // GPIO34 (ADC1_CH6) for X-axis
#define JOYSTICK_Y_PIN 35     // GPIO35 (ADC1_CH7) for Y-axis
#define JOYSTICK_BUTTON_PIN 4 // GPIO4 for joystick button

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

// Mode definitions
#define MODE_MAIN 0        // Main mode - shows selected pattern + palette
#define MODE_SETTINGS 1    // Settings mode - quadrant-based interface
#define MODE_POINTER 2     // Pointer mode - red background with chase pointer
#define MODE_PATTERN 3     // Pattern browse mode - joystick controls pattern/palette
#define MODE_CALIBRATION 4 // Joystick calibration mode
#define NUM_MODES 3        // Total number of normal modes for single-click cycling (Main, Pointer, Pattern)

// Joystick sensitivity settings
#define BUTTON_DEBOUNCE_MS 200 // Button debounce time

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
#define SETTINGS_SPEED_MAX 10.0f      // Maximum global speed
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
