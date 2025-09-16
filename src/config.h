#ifndef CONFIG_H
#define CONFIG_H

// LED Configuration
#define NUM_LEDS 55      // Adjust based on your LED strip length
#define DATA_PIN 13      // GPIO pin connected to LED data line
#define LED_TYPE WS2812B // Change if using different LED type (APA102, WS2811, etc.)
#define COLOR_ORDER GRB  // Color order for your LED strip

// Brightness settings (0-255)
#define DEFAULT_BRIGHTNESS 128
#define MAX_BRIGHTNESS 255

// Joystick Configuration (for future use)
#define JOYSTICK_X_PIN A0     // Analog pin for X-axis
#define JOYSTICK_Y_PIN A1     // Analog pin for Y-axis
#define JOYSTICK_BUTTON_PIN 4 // Digital pin for joystick button

// Joystick calibration values
#define JOYSTICK_CENTER 512  // Center position (0-1023 range)
#define JOYSTICK_DEADZONE 50 // Deadzone around center

// Update intervals
#define LED_UPDATE_INTERVAL 16    // ~60 FPS (16ms)
#define JOYSTICK_READ_INTERVAL 10 // Read joystick every 10ms

// Default static color (RGB values 0-255)
#define STATIC_COLOR_R 255
#define STATIC_COLOR_G 0
#define STATIC_COLOR_B 100

#endif // CONFIG_H
