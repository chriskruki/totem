#include "SystemManager.h"
#include "config.h"

// Demo colors array (moved from main.cpp)
const CRGB SystemManager::demoColors[] = {
    CRGB(STATIC_COLOR_R, STATIC_COLOR_G, STATIC_COLOR_B), // Default static color
    CRGB::Red,
    CRGB::Green,
    CRGB::Blue,
    CRGB::Yellow,
    CRGB::Purple,
    CRGB::Cyan,
    CRGB::White};

const int SystemManager::NUM_DEMO_COLORS = sizeof(SystemManager::demoColors) / sizeof(SystemManager::demoColors[0]);

SystemManager::SystemManager(LEDDriver *ledDriverRef, WiFiManager *wifiManagerRef)
    : ledDriver(ledDriverRef),
      wifiManager(wifiManagerRef),
      lastColorChange(0),
      colorIndex(0)
{
}

void SystemManager::initialize()
{
  // Initialize serial communication
  Serial.begin(115200);
  delay(1000); // Give serial monitor time to connect

  Serial.println("=================================");
  Serial.println("LED Clock Totem");
  Serial.println("=================================");

  // Initialize modules
  if (wifiManager)
  {
    wifiManager->initialize();
  }

  if (ledDriver)
  {
    if (ledDriver->initialize())
    {
      Serial.println("LED Driver initialization successful!");

      // Set initial static color
      ledDriver->setSolidColor(STATIC_COLOR_R, STATIC_COLOR_G, STATIC_COLOR_B);
      ledDriver->show();

      Serial.print("Displaying static color: RGB(");
      Serial.print(STATIC_COLOR_R);
      Serial.print(", ");
      Serial.print(STATIC_COLOR_G);
      Serial.print(", ");
      Serial.print(STATIC_COLOR_B);
      Serial.println(")");
    }
    else
    {
      Serial.println("ERROR: LED Driver initialization failed!");
      while (1)
      {
        delay(1000); // Halt execution on initialization failure
      }
    }
  }

  Serial.println("Setup complete! LED strip should now be lit.");
  Serial.println("Type 'help' in serial monitor for available commands.");
}

void SystemManager::update()
{
  // Update LED driver
  if (ledDriver)
  {
    ledDriver->update();
  }

  // Update WiFi manager
  if (wifiManager)
  {
    wifiManager->update();
  }

  // Handle serial commands
  handleSerialCommands();

  // Optional: Cycle through demo colors (uncomment to enable)
  // cycleDemoColors();
}

void SystemManager::handleSerialCommands()
{
  if (Serial.available())
  {
    String command = Serial.readStringUntil('\n');
    command.trim();
    command.toLowerCase();

    // Try WiFi manager commands first
    if (wifiManager && wifiManager->handleSerialCommand(command))
    {
      return; // Command was handled by WiFi manager
    }

    // Handle LED-specific commands
    handleLEDCommands(command);

    // Handle system-level commands
    handleSystemCommands(command);
  }
}

void SystemManager::handleLEDCommands(const String &command)
{
  if (!ledDriver)
    return;

  if (command == "clear")
  {
    ledDriver->clear();
    ledDriver->show();
    Serial.println("LEDs cleared");
  }
  else if (command == "static")
  {
    ledDriver->setSolidColor(STATIC_COLOR_R, STATIC_COLOR_G, STATIC_COLOR_B);
    ledDriver->show();
    Serial.println("Static color restored");
  }
  else if (command == "red")
  {
    ledDriver->setSolidColor(255, 0, 0);
    ledDriver->show();
    Serial.println("Color set to red");
  }
  else if (command == "green")
  {
    ledDriver->setSolidColor(0, 255, 0);
    ledDriver->show();
    Serial.println("Color set to green");
  }
  else if (command == "blue")
  {
    ledDriver->setSolidColor(0, 0, 255);
    ledDriver->show();
    Serial.println("Color set to blue");
  }
  else if (command == "white")
  {
    ledDriver->setSolidColor(255, 255, 255);
    ledDriver->show();
    Serial.println("Color set to white");
  }
  else if (command.startsWith("brightness "))
  {
    int brightness = command.substring(11).toInt();
    if (brightness >= 0 && brightness <= 255)
    {
      ledDriver->setBrightness(brightness);
      ledDriver->show();
      Serial.print("Brightness set to: ");
      Serial.println(brightness);
    }
    else
    {
      Serial.println("Invalid brightness value (0-255)");
    }
  }
  else if (command.startsWith("mode "))
  {
    int mode = command.substring(5).toInt();
    ledDriver->setMode(mode);
  }
  else if (command == "mode")
  {
    Serial.print("Current mode: ");
    Serial.println(ledDriver->getCurrentMode());
  }
  else if (command == "color")
  {
    uint8_t r, g, b;
    ledDriver->getCurrentColor(r, g, b);
    Serial.print("Current color: RGB(");
    Serial.print(r);
    Serial.print(", ");
    Serial.print(g);
    Serial.print(", ");
    Serial.print(b);
    Serial.println(")");
  }
  else if (command == "calibrate")
  {
    Serial.println("=== ENTERING CALIBRATION MODE ===");
    ledDriver->setMode(MODE_CALIBRATION); // Force calibration mode
    Serial.println("Move joystick to all extremes.");
    Serial.println("Press joystick button to save, or wait 10s to auto-save.");
    Serial.println("LEDs will blink rapidly during calibration.");
  }
  else if (command == "bounds")
  {
    int xMin, xMax, yMin, yMax;
    ledDriver->getCalibrationBounds(xMin, xMax, yMin, yMax);
    Serial.println("=== Joystick Calibration Bounds ===");
    Serial.print("X: ");
    Serial.print(xMin);
    Serial.print(" to ");
    Serial.println(xMax);
    Serial.print("Y: ");
    Serial.print(yMin);
    Serial.print(" to ");
    Serial.println(yMax);
  }
  else if (command == "power")
  {
    Serial.println("=== Power Consumption ===");
    Serial.print("Current draw: ");
    Serial.print(ledDriver->getCurrentDraw(), 1);
    Serial.println(" mA");
    Serial.print("Power consumption: ");
    Serial.print(ledDriver->getCurrentPowerConsumption(), 2);
    Serial.println(" W");
    Serial.print("Max current limit: ");
    Serial.print(MAX_CURRENT_MA);
    Serial.println(" mA");
    Serial.print("Safe current limit (");
    Serial.print(SAFETY_MARGIN_PERCENT);
    Serial.print("%): ");
    Serial.print((float)MAX_CURRENT_MA * (SAFETY_MARGIN_PERCENT / 100.0), 0);
    Serial.println(" mA");
    Serial.print("Power limiting: ");
    Serial.println(ENABLE_POWER_LIMITING ? "ENABLED" : "DISABLED");
    Serial.print("Power limited: ");
    Serial.println(ledDriver->isPowerLimited() ? "YES" : "NO");
  }
}

void SystemManager::handleSystemCommands(const String &command)
{
  if (command == "help")
  {
    printHelp();
  }
  else if (command == "info")
  {
    printSystemInfo();
  }
  else if (command == "demo")
  {
    // The cycleDemoColors function will handle this
    Serial.println("Demo mode - uncomment cycleDemoColors() in update() method");
  }
  else if (command != "" && !command.startsWith("wifi") &&
           !command.startsWith("brightness") && !command.startsWith("mode") &&
           command != "clear" && command != "static" && command != "red" &&
           command != "green" && command != "blue" && command != "white" &&
           command != "color" && command != "calibrate" && command != "bounds" &&
           command != "power")
  {
    Serial.println("Unknown command. Type 'help' for available commands.");
  }
}

void SystemManager::printHelp()
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
  Serial.println("mode         - Show current joystick mode");
  Serial.println("mode X       - Set joystick mode (0=Config, 1=Color, 2=Blink, 3=Pointer)");
  Serial.println("color        - Show current RGB color values");
  Serial.println("calibrate    - Enter joystick calibration mode");
  Serial.println("bounds       - Show current calibration bounds");
  Serial.println("power        - Show power consumption and safety limits");
  Serial.println("wifi         - Show WiFi status");
  Serial.println("wifi on      - Enable WiFi Access Point and web server");
  Serial.println("wifi off     - Disable WiFi Access Point and web server");
  Serial.println("");
  Serial.println("=== Joystick Modes ===");
  Serial.println("Mode 0: Config   - Y-axis controls brightness");
  Serial.println("Mode 1: Color    - X/Y-axis controls RGB color wheel");
  Serial.println("Mode 2: Blink    - White blink placeholder");
  Serial.println("Mode 3: Pointer  - Joystick direction lights up LEDs in circle");
  Serial.println("Button press toggles between modes");
  Serial.println("");
  Serial.println("=== Calibration ===");
  Serial.println("Method 1: Type 'calibrate' command");
  Serial.println("Method 2: Double-click joystick button");
  Serial.println("Move joystick to all extremes, then press button to save");
  Serial.println("");
  Serial.println("=== WiFi Web Interface ===");
  Serial.println("Enable WiFi: 'wifi on' - Creates access point for debugging");
  Serial.println("Disable WiFi: 'wifi off' - Saves power when not needed");
  Serial.println("Web interface available at: http://192.168.4.1 (when enabled)");
  Serial.println("========================\n");
}

void SystemManager::printSystemInfo()
{
  Serial.println("\n=== System Information ===");

  if (ledDriver)
  {
    Serial.print("Number of LEDs: ");
    Serial.println(ledDriver->getNumLEDs());
    Serial.print("Current brightness: ");
    Serial.println(ledDriver->getBrightness());
  }

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
  Serial.print("Uptime: ");
  Serial.print(millis() / 1000);
  Serial.println(" seconds");

  Serial.print("Power Limiting: ");
  Serial.println(ENABLE_POWER_LIMITING ? "ENABLED" : "DISABLED");
  if (ENABLE_POWER_LIMITING)
  {
    Serial.print("Max Current: ");
    Serial.print(MAX_CURRENT_MA);
    Serial.println(" mA");
    Serial.print("Safety Margin: ");
    Serial.print(SAFETY_MARGIN_PERCENT);
    Serial.println("%");
  }

  // WiFi Status
  if (wifiManager)
  {
    Serial.print("WiFi Status: ");
    if (wifiManager->isWiFiEnabled() && wifiManager->isWiFiStarted())
    {
      String ssid;
      IPAddress ip;
      int clients;
      wifiManager->getWiFiStatus(ssid, ip, clients);
      Serial.print("ENABLED (");
      Serial.print(clients);
      Serial.println(" clients)");
      Serial.print("Web Interface: http://");
      Serial.println(ip);
    }
    else
    {
      Serial.println("DISABLED");
    }
  }

  Serial.println("=========================\n");
}

void SystemManager::cycleDemoColors()
{
  if (!ledDriver)
    return;

  unsigned long currentTime = millis();
  const unsigned long COLOR_CHANGE_INTERVAL = 3000; // Change color every 3 seconds

  if (currentTime - lastColorChange >= COLOR_CHANGE_INTERVAL)
  {
    ledDriver->setSolidColor(demoColors[colorIndex]);
    ledDriver->show();

    Serial.print("Demo color ");
    Serial.print(colorIndex + 1);
    Serial.print("/");
    Serial.println(NUM_DEMO_COLORS);

    colorIndex = (colorIndex + 1) % NUM_DEMO_COLORS;
    lastColorChange = currentTime;
  }
}
