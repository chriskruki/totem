#ifndef CONFIG_H
#define CONFIG_H

// LED Configuration
#define NUM_LEDS 55      // Adjust based on your LED strip length
#define DATA_PIN 13      // GPIO pin connected to LED data line
#define LED_TYPE WS2812B // Change if using different LED type (APA102, WS2811, etc.)
#define COLOR_ORDER GRB  // Color order for your LED strip

// Brightness settings (0-255)
#define DEFAULT_BRIGHTNESS 50
#define MAX_BRIGHTNESS 255

// Joystick Configuration
#define JOYSTICK_X_PIN 34     // GPIO34 (ADC1_CH6) for X-axis
#define JOYSTICK_Y_PIN 35     // GPIO35 (ADC1_CH7) for Y-axis
#define JOYSTICK_BUTTON_PIN 4 // GPIO4 for joystick button

// Joystick calibration values
#define JOYSTICK_MIN 0       // Minimum ADC value (12-bit ADC)
#define JOYSTICK_MAX 4095    // Maximum ADC value (12-bit ADC)
#define JOYSTICK_CENTER 1790 // Center position (0-4095 range)
#define JOYSTICK_DEADZONE 50 // Deadzone around center (scaled for 4095 range)

// Update intervals
#define LED_UPDATE_INTERVAL 16    // ~60 FPS (16ms)
#define JOYSTICK_READ_INTERVAL 10 // Read joystick every 10ms

// Default static color (RGB values 0-255)
#define STATIC_COLOR_R 255
#define STATIC_COLOR_G 0
#define STATIC_COLOR_B 100

// Mode definitions
#define MODE_CONFIG 0      // Brightness control mode
#define MODE_COLOR 1       // RGB color wheel mode
#define MODE_BLINK 2       // White blink placeholder mode
#define MODE_POINTER 3     // Circular pointer mode
#define MODE_CALIBRATION 4 // Joystick calibration mode
#define NUM_MODES 4        // Total number of normal modes (calibration is special)

// Joystick sensitivity settings
#define BRIGHTNESS_STEP 5      // How much brightness changes per joystick movement
#define COLOR_SENSITIVITY 2    // Sensitivity for color changes
#define BLINK_INTERVAL 500     // Blink interval in milliseconds for mode 3
#define BUTTON_DEBOUNCE_MS 200 // Button debounce time

// Pointer mode settings
#define POINTER_LED_COUNT 3    // Number of LEDs to light up in pointer mode
#define POINTER_BRIGHTNESS 255 // Brightness for pointer LEDs

// Pointer highlight colors
#define POINTER_COLOR_R 0   // Red component for pointer highlight
#define POINTER_COLOR_G 255 // Green component for pointer highlight
#define POINTER_COLOR_B 0   // Blue component for pointer highlight

// Pointer background colors
#define POINTER_BG_COLOR_R 20    // Red component for background
#define POINTER_BG_COLOR_G 0     // Green component for background
#define POINTER_BG_COLOR_B 20    // Blue component for background
#define POINTER_BG_BRIGHTNESS 64 // Brightness for background LEDs

// Calibration settings
#define DOUBLE_CLICK_TIMEOUT 500   // Max time between clicks for double-click (ms)
#define CALIBRATION_TIMEOUT 10000  // Auto-exit calibration after 10 seconds
#define CALIBRATION_BLINK_RATE 200 // Blink rate during calibration (ms)
#define MIN_JOYSTICK_RANGE 100     // Minimum acceptable range for calibration

#endif // CONFIG_H
