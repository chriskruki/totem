#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>

// Forward declaration
class LEDDriver;

/**
 * @brief WiFi and Web Server Manager
 *
 * Handles WiFi Access Point, web server, captive portal, and all web-related functionality.
 * Provides a clean interface for enabling/disabling WiFi and serving web content.
 */
class WiFiManager
{
private:
  // Web server and DNS server instances
  WebServer server;
  DNSServer dnsServer;

  // WiFi state tracking
  bool wifiEnabled;
  bool wifiStarted;

  // Reference to LED driver for status queries
  LEDDriver *ledDriver;

  // Private setup methods
  void setupWiFiAP();
  void setupWebServer();
  void setupCaptivePortal();

  // Web request handlers
  void handleRoot();
  void handleStatus();
  void handleCaptivePortal();
  void handleNotFound();

  // Helper methods for HTML generation
  String generateStatusHTML();
  String generateStatusJSON();

public:
  /**
   * @brief Constructor
   * @param ledDriverRef Reference to the LED driver instance
   */
  WiFiManager(LEDDriver *ledDriverRef);

  /**
   * @brief Initialize WiFi manager (call in setup())
   */
  void initialize();

  /**
   * @brief Update WiFi manager (call in main loop)
   */
  void update();

  /**
   * @brief Enable WiFi Access Point and web server
   * @return true if successful, false otherwise
   */
  bool enableWiFi();

  /**
   * @brief Disable WiFi Access Point and web server
   * @return true if successful, false otherwise
   */
  bool disableWiFi();

  /**
   * @brief Check if WiFi is enabled
   * @return true if WiFi is enabled
   */
  bool isWiFiEnabled() const { return wifiEnabled; }

  /**
   * @brief Check if WiFi is started and running
   * @return true if WiFi is started and running
   */
  bool isWiFiStarted() const { return wifiStarted; }

  /**
   * @brief Get WiFi status information
   * @param ssid Reference to store SSID
   * @param ip Reference to store IP address
   * @param clients Reference to store number of connected clients
   */
  void getWiFiStatus(String &ssid, IPAddress &ip, int &clients);

  /**
   * @brief Handle WiFi-related serial commands
   * @param command The command string to process
   * @return true if command was handled, false if not recognized
   */
  bool handleSerialCommand(const String &command);

  /**
   * @brief Print WiFi status to serial
   */
  void printWiFiStatus();
};

#endif // WIFI_MANAGER_H
