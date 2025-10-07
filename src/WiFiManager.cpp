#include "WiFiManager.h"
#include "LEDDriver.h"
#include "config.h"

WiFiManager::WiFiManager(LEDDriver *ledDriverRef)
    : server(WEB_SERVER_PORT),
      wifiEnabled(ENABLE_WIFI_AP),
      wifiStarted(false),
      ledDriver(ledDriverRef)
{
}

void WiFiManager::initialize()
{
  if (wifiEnabled)
  {
    enableWiFi();
  }
  else
  {
    // Completely disable all wireless for maximum power savings
    disableAllWireless();
    Serial.println("All wireless disabled for power optimization (can be enabled with 'wifi on' command)");
  }
}

void WiFiManager::update()
{
  if (wifiEnabled && wifiStarted)
  {
    server.handleClient();
    if (ENABLE_CAPTIVE_PORTAL)
    {
      dnsServer.processNextRequest();
    }
  }
}

bool WiFiManager::enableWiFi()
{
  if (wifiStarted)
  {
    Serial.println("WiFi already started");
    return true;
  }

  Serial.println("Starting WiFi Access Point and Web Server...");

  try
  {
    setupWiFiAP();
    setupWebServer();
    if (ENABLE_CAPTIVE_PORTAL)
    {
      setupCaptivePortal();
    }

    wifiEnabled = true;
    wifiStarted = true;
    Serial.println("WiFi started successfully! Use 'wifi off' to disable.");
    return true;
  }
  catch (...)
  {
    Serial.println("Failed to start WiFi");
    return false;
  }
}

bool WiFiManager::disableWiFi()
{
  if (!wifiStarted)
  {
    Serial.println("WiFi already stopped");
    return true;
  }

  Serial.println("Stopping WiFi Access Point and Web Server...");

  // Stop the web server
  server.stop();

  // Stop DNS server if captive portal is enabled
  if (ENABLE_CAPTIVE_PORTAL)
  {
    dnsServer.stop();
  }

  // Disconnect all clients and stop AP
  WiFi.softAPdisconnect(true);
  WiFi.mode(WIFI_OFF);

  wifiEnabled = false;
  wifiStarted = false;

  Serial.println("WiFi stopped successfully! Use 'wifi on' to re-enable.");
  return true;
}

void WiFiManager::disableAllWireless()
{
  Serial.println("Disabling all wireless functionality for maximum power savings...");

  // === WiFi Shutdown ===
  try
  {
    // Disconnect any existing connections
    WiFi.disconnect(true);

    // Stop WiFi completely
    WiFi.mode(WIFI_OFF);

    // Stop the WiFi driver
    esp_wifi_stop();

    // Deinitialize WiFi driver to free memory and save power
    esp_wifi_deinit();

    Serial.println("✓ WiFi completely disabled");
  }
  catch (...)
  {
    Serial.println("⚠ WiFi disable encountered minor issues (likely already disabled)");
  }

  // === Bluetooth Shutdown ===
  try
  {
    // Disable Bluetooth controller
    esp_bt_controller_disable();

    // Deinitialize Bluetooth controller
    esp_bt_controller_deinit();

    // Release Bluetooth memory for both Classic and BLE
    esp_bt_mem_release(ESP_BT_MODE_BTDM);

    Serial.println("✓ Bluetooth completely disabled");
  }
  catch (...)
  {
    Serial.println("⚠ Bluetooth disable encountered minor issues (likely already disabled)");
  }

  // === Power Optimization ===
  // Disable unused power domains for additional savings
  esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_SLOW_MEM, ESP_PD_OPTION_OFF);
  esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_FAST_MEM, ESP_PD_OPTION_OFF);

// Reduce CPU frequency for additional power savings
#if ENABLE_CPU_FREQUENCY_REDUCTION
  uint32_t currentFreq = getCpuFrequencyMhz();
  if (currentFreq > POWER_OPTIMIZED_CPU_FREQ)
  {
    setCpuFrequencyMhz(POWER_OPTIMIZED_CPU_FREQ);
    Serial.printf("✓ CPU frequency reduced: %dMHz → %dMHz\n", currentFreq, POWER_OPTIMIZED_CPU_FREQ);
  }
#endif

  Serial.println("✓ Wireless shutdown complete - power optimized!");
  Serial.println("💡 Estimated power savings: 55-85 mA (20-33% battery life improvement)");
  Serial.printf("💡 Total ESP32 power consumption now: ~%d-30 mA\n",
                POWER_OPTIMIZED_CPU_FREQ == 80 ? 15 : (POWER_OPTIMIZED_CPU_FREQ == 160 ? 20 : 30));
}

void WiFiManager::setupWiFiAP()
{
  Serial.println("Setting up WiFi Access Point...");

  // Configure Access Point
  WiFi.mode(WIFI_AP);
  delay(100); // Give WiFi time to switch modes

  // Try setting up the AP with explicit parameters
  bool success = WiFi.softAP(WIFI_AP_SSID, WIFI_AP_PASSWORD, WIFI_AP_CHANNEL, 0, WIFI_AP_MAX_CONNECTIONS);

  if (!success)
  {
    Serial.println("WiFi.softAP() with full config failed, trying basic config...");
    // Try with just SSID and password
    success = WiFi.softAP(WIFI_AP_SSID, WIFI_AP_PASSWORD);
  }

  if (success)
  {
    Serial.println("WiFi Access Point started successfully!");
  }
  else
  {
    Serial.println("ERROR: Failed to start WiFi Access Point!");
  }

  // Get AP IP address
  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);
  Serial.print("WiFi SSID: ");
  Serial.println(WIFI_AP_SSID);
  Serial.print("WiFi Password: ");
  Serial.println(WIFI_AP_PASSWORD);
}

void WiFiManager::setupWebServer()
{
  Serial.println("Setting up Web Server...");

  // Setup server routes using lambda functions to capture 'this'
  server.on("/", [this]()
            { handleRoot(); });
  server.on("/status", [this]()
            { handleStatus(); });

  // Captive portal detection routes - redirect to main page
  if (ENABLE_CAPTIVE_PORTAL)
  {
    // Common captive portal detection URLs
    server.on("/generate_204", [this]()
              { handleCaptivePortal(); }); // Android
    server.on("/gen_204", [this]()
              { handleCaptivePortal(); }); // Android alternative
    server.on("/ncsi.txt", [this]()
              { handleCaptivePortal(); }); // Windows
    server.on("/hotspot-detect.html", [this]()
              { handleCaptivePortal(); }); // iOS/macOS
    server.on("/connecttest.txt", [this]()
              { handleCaptivePortal(); }); // Windows
    server.on("/redirect", [this]()
              { handleCaptivePortal(); }); // Generic
    server.on("/success.txt", [this]()
              { handleCaptivePortal(); }); // Chrome
  }

  server.onNotFound([this]()
                    { handleNotFound(); });

  // Start server
  server.begin();
  Serial.print("HTTP server started on port ");
  Serial.println(WEB_SERVER_PORT);
  Serial.println("Connect to WiFi and visit http://192.168.4.1 in your browser");
}

void WiFiManager::setupCaptivePortal()
{
  Serial.println("Setting up Captive Portal...");

  // Start DNS server
  dnsServer.start(DNS_SERVER_PORT, "*", WiFi.softAPIP());
  Serial.println("DNS server started for captive portal");
  Serial.println("Devices will be automatically redirected to the web interface");
}

void WiFiManager::handleRoot()
{
  String html = generateStatusHTML();
  server.send(200, "text/html", html);
}

void WiFiManager::handleStatus()
{
  String json = generateStatusJSON();
  server.send(200, "application/json", json);
}

void WiFiManager::handleCaptivePortal()
{
  // Redirect captive portal detection requests to main page
  String redirectHTML = "<!DOCTYPE html>\n";
  redirectHTML += "<html>\n";
  redirectHTML += "<head>\n";
  redirectHTML += "<title>" + String(CAPTIVE_PORTAL_TITLE) + "</title>\n";
  redirectHTML += "<meta http-equiv='refresh' content='0; url=/'>\n";
  redirectHTML += "</head>\n";
  redirectHTML += "<body>\n";
  redirectHTML += "<h1>Redirecting to " + String(CAPTIVE_PORTAL_TITLE) + "...</h1>\n";
  redirectHTML += "<p>If you are not redirected automatically, <a href='/'>click here</a>.</p>\n";
  redirectHTML += "</body>\n";
  redirectHTML += "</html>\n";

  server.send(200, "text/html", redirectHTML);
}

void WiFiManager::handleNotFound()
{
  // For captive portal, redirect unknown requests to main page
  if (ENABLE_CAPTIVE_PORTAL)
  {
    handleCaptivePortal();
    return;
  }

  // Standard 404 response
  String message = "File Not Found\n\n";
  message += "URI: " + server.uri() + "\n";
  message += "Method: " + String(server.method() == 0 ? "GET" : "POST") + "\n";
  message += "Arguments: " + String(server.args()) + "\n";

  for (uint8_t i = 0; i < server.args(); i++)
  {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }

  server.send(404, "text/plain", message);
}

String WiFiManager::generateStatusHTML()
{
  if (!ledDriver)
    return "LED Driver not available";

  String html = "<!DOCTYPE html>\n";
  html += "<html>\n";
  html += "<head>\n";
  html += "<title>" + String(CAPTIVE_PORTAL_TITLE) + " Control</title>\n";
  html += "<meta http-equiv='refresh' content='2'>\n"; // Auto-refresh every 2 seconds
  html += "</head>\n";
  html += "<body>\n";
  html += "<h1>" + String(CAPTIVE_PORTAL_TITLE) + " Status</h1>\n";

  // Current Mode
  html += "<div>\n";
  html += "<h2>Current Mode</h2>\n";
  uint8_t mode = ledDriver->getCurrentMode();
  String modeName = "";
  switch (mode)
  {
  case SPECIAL_MODE_SETTINGS:
    modeName = "Settings Mode";
    break;
  case MAIN_MODE_EXPLORER:
    modeName = "Explorer Mode";
    break;
  case MAIN_MODE_INTERACTION:
    modeName = "Interaction Mode";
    break;
  default:
    modeName = "Unknown";
    break;
  }
  html += "<p>Mode " + String(mode) + ": " + modeName + "</p>\n";
  html += "</div>\n";

  // Current Color
  html += "<div>\n";
  html += "<h2>Current Color</h2>\n";
  uint8_t r, g, b;
  ledDriver->getCurrentColor(r, g, b);
  html += "<p>RGB(" + String(r) + ", " + String(g) + ", " + String(b) + ")</p>\n";
  html += "</div>\n";

  // Brightness
  html += "<div>\n";
  html += "<h2>Brightness</h2>\n";
  html += "<p>" + String(ledDriver->getBrightness()) + " / 255</p>\n";
  html += "</div>\n";

  // Pattern Information (show in pattern and eye modes)
  if (mode == MAIN_MODE_EXPLORER || mode == MAIN_MODE_INTERACTION)
  {
    html += "<div>\n";
    html += "<h2>Pattern Information</h2>\n";
    PatternManager &patternManager = ledDriver->getPatternManager();
    Pattern *currentPattern = patternManager.getCurrentPattern();
    ColorPalette *currentPalette = patternManager.getPaletteManager().getCurrentPalette();

    if (currentPattern)
    {
      html += "<p>Current Pattern: " + currentPattern->getName() + "</p>\n";
      html += "<p>Description: " + currentPattern->getDescription() + "</p>\n";
      html += "<p>Pattern " + String(patternManager.getCurrentPatternIndex() + 1) + " of " + String(patternManager.getPatternCount()) + "</p>\n";
    }

    if (currentPalette)
    {
      html += "<p>Current Palette: " + currentPalette->getName() + "</p>\n";
      html += "<p>Palette " + String(patternManager.getPaletteManager().getCurrentPaletteIndex() + 1) + " of " + String(patternManager.getPaletteManager().getPaletteCount()) + "</p>\n";
    }

    html += "<p>Global Speed: " + String(patternManager.getGlobalSpeed(), 1) + "x</p>\n";
    html += "<p>Auto Switch: " + String(patternManager.getAutoSwitch() ? "ON" : "OFF") + "</p>\n";
    html += "</div>\n";
  }

  // Power Information
  html += "<div>\n";
  html += "<h2>Power Consumption</h2>\n";
  if (ENABLE_POWER_LIMITING)
  {
    float currentDraw = ledDriver->getCurrentDraw();
    float powerConsumption = ledDriver->getCurrentPowerConsumption();
    bool isPowerLimited = ledDriver->isPowerLimited();

    html += "<p>Current Draw: " + String(currentDraw, 1) + " mA</p>\n";
    html += "<p>Power Consumption: " + String(powerConsumption, 2) + " W</p>\n";
    html += "<p>Max Current Limit: " + String(MAX_CURRENT_MA) + " mA</p>\n";
    html += "<p>Safe Current Limit: " + String((float)MAX_CURRENT_MA * (SAFETY_MARGIN_PERCENT / 100.0), 0) + " mA</p>\n";
    html += "<p>Power Limited: " + String(isPowerLimited ? "YES" : "NO") + "</p>\n";
  }
  else
  {
    html += "<p>Power limiting disabled</p>\n";
  }
  html += "</div>\n";

  // System Information
  html += "<div>\n";
  html += "<h2>System Information</h2>\n";
  html += "<p>Number of LEDs: " + String(ledDriver->getNumLEDs()) + "</p>\n";
  html += "<p>Free Heap: " + String(ESP.getFreeHeap()) + " bytes</p>\n";
  html += "<p>CPU Frequency: " + String(ESP.getCpuFreqMHz()) + " MHz</p>\n";
  html += "<p>WiFi SSID: " + String(WIFI_AP_SSID) + "</p>\n";
  html += "</div>\n";

  // Calibration Status
  html += "<div>\n";
  html += "<h2>Joystick Status</h2>\n";
  if (ledDriver->isInCalibrationMode())
  {
    html += "<p>Status: IN CALIBRATION MODE</p>\n";
  }
  else
  {
    html += "<p>Status: Normal Operation</p>\n";
    int xMin, xMax, yMin, yMax;
    ledDriver->getCalibrationBounds(xMin, xMax, yMin, yMax);
    html += "<p>X Range: " + String(xMin) + " to " + String(xMax) + "</p>\n";
    html += "<p>Y Range: " + String(yMin) + " to " + String(yMax) + "</p>\n";
  }
  html += "</div>\n";

  html += "<div>\n";
  html += "<p><em>Page auto-refreshes every 2 seconds</em></p>\n";
  html += "</div>\n";

  html += "</body>\n";
  html += "</html>\n";

  return html;
}

String WiFiManager::generateStatusJSON()
{
  if (!ledDriver)
    return "{\"error\": \"LED Driver not available\"}";

  String json = "{\n";

  // Mode information
  uint8_t mode = ledDriver->getCurrentMode();
  json += "  \"mode\": " + String(mode) + ",\n";

  String modeName = "";
  switch (mode)
  {
  case SPECIAL_MODE_SETTINGS:
    modeName = "Settings";
    break;
  case MAIN_MODE_EXPLORER:
    modeName = "Explorer";
    break;
  case MAIN_MODE_INTERACTION:
    modeName = "Interaction";
    break;
  default:
    modeName = "Unknown";
    break;
  }
  json += "  \"modeName\": \"" + modeName + "\",\n";

  // Color information
  uint8_t r, g, b;
  ledDriver->getCurrentColor(r, g, b);
  json += "  \"color\": {\n";
  json += "    \"r\": " + String(r) + ",\n";
  json += "    \"g\": " + String(g) + ",\n";
  json += "    \"b\": " + String(b) + "\n";
  json += "  },\n";

  // Brightness
  json += "  \"brightness\": " + String(ledDriver->getBrightness()) + ",\n";

  // Pattern information (include in pattern and eye modes)
  if (mode == MAIN_MODE_EXPLORER || mode == MAIN_MODE_INTERACTION)
  {
    json += "  \"pattern\": {\n";
    PatternManager &patternManager = ledDriver->getPatternManager();
    Pattern *currentPattern = patternManager.getCurrentPattern();
    ColorPalette *currentPalette = patternManager.getPaletteManager().getCurrentPalette();

    if (currentPattern)
    {
      json += "    \"name\": \"" + currentPattern->getName() + "\",\n";
      json += "    \"description\": \"" + currentPattern->getDescription() + "\",\n";
      json += "    \"index\": " + String(patternManager.getCurrentPatternIndex()) + ",\n";
      json += "    \"total\": " + String(patternManager.getPatternCount()) + ",\n";
    }
    else
    {
      json += "    \"name\": null,\n";
      json += "    \"description\": null,\n";
      json += "    \"index\": -1,\n";
      json += "    \"total\": 0,\n";
    }

    if (currentPalette)
    {
      json += "    \"palette\": {\n";
      json += "      \"name\": \"" + currentPalette->getName() + "\",\n";
      json += "      \"index\": " + String(patternManager.getPaletteManager().getCurrentPaletteIndex()) + ",\n";
      json += "      \"total\": " + String(patternManager.getPaletteManager().getPaletteCount()) + "\n";
      json += "    },\n";
    }
    else
    {
      json += "    \"palette\": null,\n";
    }

    json += "    \"speed\": " + String(patternManager.getGlobalSpeed(), 2) + ",\n";
    json += "    \"autoSwitch\": " + String(patternManager.getAutoSwitch() ? "true" : "false") + "\n";
    json += "  },\n";
  }
  else
  {
    json += "  \"pattern\": null,\n";
  }

  // Power information
  if (ENABLE_POWER_LIMITING)
  {
    json += "  \"power\": {\n";
    json += "    \"currentDraw\": " + String(ledDriver->getCurrentDraw(), 1) + ",\n";
    json += "    \"powerConsumption\": " + String(ledDriver->getCurrentPowerConsumption(), 2) + ",\n";
    json += "    \"maxCurrent\": " + String(MAX_CURRENT_MA) + ",\n";
    json += "    \"safeCurrent\": " + String((float)MAX_CURRENT_MA * (SAFETY_MARGIN_PERCENT / 100.0), 0) + ",\n";
    json += "    \"isLimited\": " + String(ledDriver->isPowerLimited() ? "true" : "false") + "\n";
    json += "  },\n";
  }
  else
  {
    json += "  \"power\": null,\n";
  }

  // System information
  json += "  \"system\": {\n";
  json += "    \"numLEDs\": " + String(ledDriver->getNumLEDs()) + ",\n";
  json += "    \"freeHeap\": " + String(ESP.getFreeHeap()) + ",\n";
  json += "    \"cpuFreq\": " + String(ESP.getCpuFreqMHz()) + ",\n";
  json += "    \"inCalibration\": " + String(ledDriver->isInCalibrationMode() ? "true" : "false") + "\n";
  json += "  }\n";

  json += "}\n";

  return json;
}

void WiFiManager::getWiFiStatus(String &ssid, IPAddress &ip, int &clients)
{
  if (wifiStarted)
  {
    ssid = String(WIFI_AP_SSID);
    ip = WiFi.softAPIP();
    clients = WiFi.softAPgetStationNum();
  }
  else
  {
    ssid = "";
    ip = IPAddress(0, 0, 0, 0);
    clients = 0;
  }
}

bool WiFiManager::handleSerialCommand(const String &command)
{
  if (command == "wifi on" || command == "wifi enable")
  {
    if (!wifiEnabled)
    {
      return enableWiFi();
    }
    else if (!wifiStarted)
    {
      return enableWiFi();
    }
    else
    {
      Serial.println("WiFi is already enabled and running");
      return true;
    }
  }
  else if (command == "wifi off" || command == "wifi disable")
  {
    if (wifiEnabled && wifiStarted)
    {
      return disableWiFi();
    }
    else
    {
      Serial.println("WiFi is already disabled");
      return true;
    }
  }
  else if (command == "wifi status" || command == "wifi")
  {
    printWiFiStatus();
    return true;
  }

  return false; // Command not handled
}

void WiFiManager::printWiFiStatus()
{
  Serial.println("=== WiFi Status ===");
  Serial.print("WiFi Enabled: ");
  Serial.println(wifiEnabled ? "YES" : "NO");
  Serial.print("WiFi Started: ");
  Serial.println(wifiStarted ? "YES" : "NO");
  if (wifiStarted)
  {
    Serial.print("SSID: ");
    Serial.println(WIFI_AP_SSID);
    Serial.print("IP Address: ");
    Serial.println(WiFi.softAPIP());
    Serial.print("Connected Clients: ");
    Serial.println(WiFi.softAPgetStationNum());
    Serial.println("Web interface: http://192.168.4.1");
    if (ENABLE_CAPTIVE_PORTAL)
    {
      Serial.println("Captive portal: ENABLED");
    }
  }
}
