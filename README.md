# FastLED ESP32 Custom Driver

A custom FastLED driver for ESP32 with scaffolding for joystick-controlled LED patterns. This project provides a solid foundation for interactive LED control and can be extended with various input methods.

## Features

- **Custom LED Driver Class**: Object-oriented approach for clean, maintainable code
- **FastLED Integration**: Optimized for WS2812B LEDs with configurable parameters
- **Joystick Ready**: Scaffolding in place for 2-axis joystick integration
- **Serial Commands**: Interactive testing via serial monitor
- **Power Management**: Built-in power limiting and brightness control
- **Extensible Architecture**: Easy to add new patterns and effects

## Hardware Requirements

- **ESP32 Development Board** (ESP32-DevKit or similar)
- **LED Strip** (WS2812B/NeoPixel recommended)
- **2-Axis Joystick Module** (for future phase)
- **5V Power Supply** (capacity depends on LED count)
- **Jumper Wires** and **Breadboard**

## Wiring Diagram

### Current Phase (Static Color)

```
ESP32          LED Strip
GPIO2    -->   Data In (DI)
5V       -->   VCC (+5V)
GND      -->   GND
```

### Future Phase (With Joystick)

```
ESP32          Joystick Module
A0       -->   X-axis (VRX)
A1       -->   Y-axis (VRY)
GPIO4    -->   Button (SW)
3.3V     -->   VCC
GND      -->   GND
```

## Software Setup

### Prerequisites

- [PlatformIO](https://platformio.org/) installed in VS Code or PlatformIO Core
- Git (for version control)

### Installation

1. **Clone the repository:**

   ```bash
   git clone <repository-url>
   cd totem
   ```

2. **Open in PlatformIO:**

   - Open VS Code with PlatformIO extension
   - File → Open Folder → Select the project directory
   - PlatformIO will automatically detect the project

3. **Configure your setup:**

   - Edit `src/config.h` to match your hardware:
     - `NUM_LEDS`: Number of LEDs in your strip
     - `DATA_PIN`: GPIO pin connected to LED data line
     - `LED_TYPE`: Your LED strip type (WS2812B default)
     - `STATIC_COLOR_R/G/B`: Default RGB color values

4. **Build and upload:**

   ```bash
   pio run --target upload
   ```

5. **Monitor serial output:**
   ```bash
   pio device monitor
   ```

## Configuration

### Key Settings in `src/config.h`

| Setting              | Description                | Default   |
| -------------------- | -------------------------- | --------- |
| `NUM_LEDS`           | Number of LEDs in strip    | 150       |
| `DATA_PIN`           | GPIO pin for LED data      | 2         |
| `LED_TYPE`           | LED strip type             | WS2812B   |
| `DEFAULT_BRIGHTNESS` | Initial brightness (0-255) | 128       |
| `STATIC_COLOR_R/G/B` | Default RGB color          | 255,0,100 |

## Usage

### Serial Commands

Once uploaded, open the serial monitor (115200 baud) and try these commands:

- `help` - Show all available commands
- `static` - Display default static color
- `red` - Set all LEDs to red
- `green` - Set all LEDs to green
- `blue` - Set all LEDs to blue
- `white` - Set all LEDs to white
- `clear` - Turn off all LEDs
- `brightness X` - Set brightness (0-255)
- `info` - Show system information

### Example Serial Session

```
FastLED ESP32 Custom Driver
===========================
LED Driver initialized with 150 LEDs
Displaying static color: RGB(255, 0, 100)
Setup complete! LED strip should now be lit.

> help
=== Available Commands ===
help         - Show this help message
clear        - Turn off all LEDs
static       - Restore default static color
...

> brightness 100
Brightness set to: 100

> blue
Color set to blue
```

## Code Structure

```
src/
├── main.cpp          # Main program loop and serial commands
├── LEDDriver.h       # LED driver class header
├── LEDDriver.cpp     # LED driver implementation
└── config.h          # Configuration constants

platformio.ini        # PlatformIO configuration
README.md             # This file
```

### Key Classes

**`LEDDriver`** - Main driver class with methods:

- `initialize()` - Set up FastLED and hardware
- `setSolidColor()` - Set all LEDs to one color
- `setLED()` - Control individual LEDs
- `setBrightness()` - Adjust brightness
- `update()` - Handle timing and refresh (call in main loop)
- `readJoystick()` - Placeholder for joystick input
- `processJoystickInput()` - Placeholder for joystick processing

## Future Development

### Phase 2: Joystick Integration

The codebase includes scaffolding for joystick control:

1. **Hardware Setup**: Connect 2-axis joystick to analog pins A0/A1
2. **Implement `readJoystick()`**: Read analog values and button state
3. **Implement `processJoystickInput()`**: Map joystick to LED patterns
4. **Add Pattern Modes**: Different effects based on joystick movement

### Suggested Joystick Features

- **Position Control**: Joystick X/Y maps to LED position
- **Color Control**: Movement changes hue/saturation
- **Brightness Control**: Y-axis controls brightness
- **Pattern Selection**: Button press cycles through patterns
- **Speed Control**: Distance from center controls animation speed

### Example Joystick Implementation Ideas

```cpp
// Map joystick X to LED position (0 to NUM_LEDS-1)
int ledPosition = map(joystickState.x, 0, 1023, 0, NUM_LEDS-1);

// Map joystick Y to hue (0-255)
uint8_t hue = map(joystickState.y, 0, 1023, 0, 255);

// Create moving dot effect
clear();
setLED(ledPosition, CHSV(hue, 255, 255));
show();
```

## Troubleshooting

### Common Issues

1. **LEDs don't light up:**

   - Check wiring connections
   - Verify power supply capacity
   - Confirm `DATA_PIN` and `NUM_LEDS` settings

2. **Flickering or wrong colors:**

   - Check `LED_TYPE` and `COLOR_ORDER` in config.h
   - Ensure stable power supply
   - Add capacitor across power lines

3. **Serial monitor shows garbled text:**

   - Verify baud rate is set to 115200
   - Check USB cable connection

4. **Upload fails:**
   - Hold BOOT button during upload
   - Try different USB port
   - Reset ESP32 before upload

### Power Considerations

- Each LED can draw up to 60mA at full brightness
- Calculate total current: `NUM_LEDS × 0.06A × brightness_factor`
- Use external 5V power supply for strips with >30 LEDs
- Add 1000µF capacitor between power supply and LED strip

## Performance Notes

- **Update Rate**: ~60 FPS (16ms intervals)
- **Memory Usage**: ~3 bytes per LED for color data
- **CPU Usage**: Minimal when using hardware SPI
- **Power Limiting**: Built-in 2A limit for safety

## Contributing

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Test thoroughly
5. Submit a pull request

## License

This project is open source. Feel free to modify and distribute according to your needs.

## Acknowledgments

- [FastLED Library](https://github.com/FastLED/FastLED) - Excellent LED control library
- [PlatformIO](https://platformio.org/) - Modern embedded development platform
- ESP32 Community - For extensive documentation and examples
