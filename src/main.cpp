#include <Arduino.h>
#include <WiFi.h>
#include "LEDDriver.h"
#include "config.h"

// Function declarations
void handleSerialCommands();
void printHelp();
void printSystemInfo();
void cycleDemoColors();

// Global LED driver instance
LEDDriver ledDriver;

// Timing variables for demo effects
unsigned long lastColorChange = 0;
const unsigned long COLOR_CHANGE_INTERVAL = 3000; // Change color every 3 seconds
int colorIndex = 0;

// Demo colors array
CRGB demoColors[] = {
    CRGB(STATIC_COLOR_R, STATIC_COLOR_G, STATIC_COLOR_B), // Default static color
    CRGB::Red,
    CRGB::Green,
    CRGB::Blue,
    CRGB::Yellow,
    CRGB::Purple,
    CRGB::Cyan,
    CRGB::White};
const int NUM_DEMO_COLORS = sizeof(demoColors) / sizeof(demoColors[0]);

/**
 * @brief Setup function - runs once at startup
 */
void setup()
{
  // Initialize serial communication
  Serial.begin(115200);
  delay(1000); // Give serial monitor time to connect

  Serial.println("=================================");
  Serial.println("FastLED ESP32 Custom Driver");
  Serial.println("=================================");

  // Disable WiFi to save power and reduce interference
  WiFi.mode(WIFI_OFF);
  Serial.println("WiFi disabled");

  // Initialize LED driver
  if (ledDriver.initialize())
  {
    Serial.println("LED Driver initialization successful!");
  }
  else
  {
    Serial.println("ERROR: LED Driver initialization failed!");
    while (1)
    {
      delay(1000); // Halt execution on initialization failure
    }
  }

  // Set initial static color
  ledDriver.setSolidColor(STATIC_COLOR_R, STATIC_COLOR_G, STATIC_COLOR_B);
  ledDriver.show();

  Serial.print("Displaying static color: RGB(");
  Serial.print(STATIC_COLOR_R);
  Serial.print(", ");
  Serial.print(STATIC_COLOR_G);
  Serial.print(", ");
  Serial.print(STATIC_COLOR_B);
  Serial.println(")");

  Serial.println("Setup complete! LED strip should now be lit.");
  Serial.println("Type 'help' in serial monitor for available commands.");
}

/**
 * @brief Main loop - runs continuously
 */
void loop()
{
  // Update LED driver (handles timing and display refresh)
  ledDriver.update();

  // Handle serial commands
  handleSerialCommands();

  // Optional: Cycle through demo colors (uncomment to enable)
  // cycleDemoColors();

  // Small delay to prevent overwhelming the CPU
  delay(1);
}

/**
 * @brief Handle serial monitor commands for testing
 */
void handleSerialCommands()
{
  if (Serial.available())
  {
    String command = Serial.readStringUntil('\n');
    command.trim();
    command.toLowerCase();

    if (command == "help")
    {
      printHelp();
    }
    else if (command == "clear")
    {
      ledDriver.clear();
      ledDriver.show();
      Serial.println("LEDs cleared");
    }
    else if (command == "static")
    {
      ledDriver.setSolidColor(STATIC_COLOR_R, STATIC_COLOR_G, STATIC_COLOR_B);
      ledDriver.show();
      Serial.println("Static color restored");
    }
    else if (command == "red")
    {
      ledDriver.setSolidColor(255, 0, 0);
      ledDriver.show();
      Serial.println("Color set to red");
    }
    else if (command == "green")
    {
      ledDriver.setSolidColor(0, 255, 0);
      ledDriver.show();
      Serial.println("Color set to green");
    }
    else if (command == "blue")
    {
      ledDriver.setSolidColor(0, 0, 255);
      ledDriver.show();
      Serial.println("Color set to blue");
    }
    else if (command == "white")
    {
      ledDriver.setSolidColor(255, 255, 255);
      ledDriver.show();
      Serial.println("Color set to white");
    }
    else if (command.startsWith("brightness "))
    {
      int brightness = command.substring(11).toInt();
      if (brightness >= 0 && brightness <= 255)
      {
        ledDriver.setBrightness(brightness);
        ledDriver.show();
        Serial.print("Brightness set to: ");
        Serial.println(brightness);
      }
      else
      {
        Serial.println("Invalid brightness value (0-255)");
      }
    }
    else if (command == "demo")
    {
      Serial.println("Starting color demo (cycles every 3 seconds)");
      // The cycleDemoColors function will handle this
    }
    else if (command == "info")
    {
      printSystemInfo();
    }
    else
    {
      Serial.println("Unknown command. Type 'help' for available commands.");
    }
  }
}

/**
 * @brief Print help information
 */
void printHelp()
{
  Serial.println("\n=== Available Commands ===");
  Serial.println("help         - Show this help message");
  Serial.println("clear        - Turn off all LEDs");
  Serial.println("static       - Restore default static color");
  Serial.println("red          - Set all LEDs to red");
  Serial.println("green        - Set all LEDs to green");
  Serial.println("blue         - Set all LEDs to blue");
  Serial.println("white        - Set all LEDs to white");
  Serial.println("brightness X - Set brightness (0-255)");
  Serial.println("demo         - Cycle through demo colors");
  Serial.println("info         - Show system information");
  Serial.println("========================\n");
}

/**
 * @brief Print system information
 */
void printSystemInfo()
{
  Serial.println("\n=== System Information ===");
  Serial.print("Number of LEDs: ");
  Serial.println(ledDriver.getNumLEDs());
  Serial.print("Current brightness: ");
  Serial.println(ledDriver.getBrightness());
  Serial.print("LED Type: ");
  Serial.println("WS2812B"); // Based on config
  Serial.print("Data Pin: ");
  Serial.println(DATA_PIN);
  Serial.print("Free heap: ");
  Serial.print(ESP.getFreeHeap());
  Serial.println(" bytes");
  Serial.print("CPU Frequency: ");
  Serial.print(ESP.getCpuFreqMHz());
  Serial.println(" MHz");
  Serial.println("=========================\n");
}

/**
 * @brief Cycle through demo colors (optional feature)
 */
void cycleDemoColors()
{
  unsigned long currentTime = millis();

  if (currentTime - lastColorChange >= COLOR_CHANGE_INTERVAL)
  {
    ledDriver.setSolidColor(demoColors[colorIndex]);
    ledDriver.show();

    Serial.print("Demo color ");
    Serial.print(colorIndex + 1);
    Serial.print("/");
    Serial.println(NUM_DEMO_COLORS);

    colorIndex = (colorIndex + 1) % NUM_DEMO_COLORS;
    lastColorChange = currentTime;
  }
}
