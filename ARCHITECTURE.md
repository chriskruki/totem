# LED Totem Architecture

## LED Layout & Indexing System

### Physical Wiring Order

```
Total LEDs: 161
├─ Clock Ring: Raw indices 0-99 (100 LEDs)
└─ Eye Rings:  Raw indices 100-160 (61 LEDs)
```

### Raw Index Mapping (Physical Wiring)

- **Clock**: 0-99 → Data flows counter-clockwise from 6 o'clock position
- **Eye**: 100-160 → Data flows clockwise from 6 o'clock position (outermost ring)

### Logical Index System

All code uses **logical indices** where index 0 = 12 o'clock (top) position.

#### Clock Ring (100 LEDs)

- Direction: Counter-clockwise from 6 o'clock (raw)
- Logical 0 = 12 o'clock = Raw 50
- Logical 25 = 3 o'clock = Raw 25
- Logical 50 = 6 o'clock = Raw 0
- Logical 75 = 9 o'clock = Raw 75

#### Eye Rings (61 LEDs total)

All rings start at 6 o'clock (raw), go clockwise, logical 0 = 12 o'clock

**EYE_4 (Outermost, 24 LEDs)**

- Raw: 100-123, Local eye indices: 0-23
- Logical 0 (12 o'clock) = Raw 111 (adjusted -1 for physical alignment)

**EYE_3 (16 LEDs)**

- Raw: 124-139, Local eye indices: 24-39
- Logical 0 (12 o'clock) = Raw 131 (adjusted -1 for physical alignment)

**EYE_2 (12 LEDs)**

- Raw: 140-151, Local eye indices: 40-51
- Logical 0 (12 o'clock) = Raw 145 (adjusted -1 for physical alignment)

**EYE_1 (8 LEDs)**

- Raw: 152-159, Local eye indices: 52-59
- Logical 0 (12 o'clock) = Raw 155 (adjusted -1 for physical alignment)

**EYE_0 (Center, 1 LED)**

- Raw: 160, Local eye index: 60

### Helper Arrays

Pre-computed mapping arrays convert logical indices to raw indices:

- `CLOCK_LED_MAP[100]` - Clock logical→raw mapping
- `EYE_4_LED_MAP[24]` - EYE_4 logical→raw mapping (outermost ring)
- `EYE_3_LED_MAP[16]` - EYE_3 logical→raw mapping
- `EYE_2_LED_MAP[12]` - EYE_2 logical→raw mapping
- `EYE_1_LED_MAP[8]` - EYE_1 logical→raw mapping
- `EYE_TOTAL_LED_MAP[61]` - Combined eye mapping (all rings outer→inner)

### Usage Example

```cpp
// Access clock LED at 12 o'clock (top)
leds[CLOCK_LED_MAP[0]] = CRGB::Red;  // Sets raw index 51

// Access EYE_4 LED at 3 o'clock (right)
leds[EYE_4_LED_MAP[6]] = CRGB::Blue;  // Sets raw index 119

// Access all eye LEDs as one contiguous array (outer to inner)
for (int i = 0; i < EYE_TOTAL_LEDS; i++) {
    leds[EYE_TOTAL_LED_MAP[i]] = CRGB::Purple;  // Lights all eye LEDs
}
// Index 0-23:   EYE_4 (outermost)
// Index 24-39:  EYE_3
// Index 40-51:  EYE_2
// Index 52-59:  EYE_1
// Index 60:     EYE_0 (center)

// SegmentManager automatically handles mapping
segmentManager.setSegmentPositionColor(leds, SEGMENT_CLOCK, 0.0f, CRGB::Green, 1);
int16_t rawIdx = segmentManager.getRawLEDIndex(SEGMENT_CLOCK, 0);  // Returns 51
```

### Brightness & Speed Preview LEDs

Position arrays store pre-computed raw LED indices for O(1) access:

**Brightness Preview** (Vertical line, 6 o'clock → 12 o'clock):

```cpp
const uint16_t BRIGHTNESS_LED_POSITIONS[9] = {
    EYE_4_LED_MAP[12],  // Level 0: EYE_4 at 6 o'clock (bottom) - 24/2 - 1
    EYE_3_LED_MAP[8],   // Level 1: EYE_3 at 6 o'clock - 16/2 - 1
    EYE_2_LED_MAP[6],   // Level 2: EYE_2 at 6 o'clock - 12/2 - 1
    EYE_1_LED_MAP[4],   // Level 3: EYE_1 at 6 o'clock - 8/2 - 1
    EYE_0_RAW_START,    // Level 4: EYE_0 center (raw 160)
    EYE_1_LED_MAP[0],   // Level 5: EYE_1 at 12 o'clock (top)
    EYE_2_LED_MAP[0],   // Level 6: EYE_2 at 12 o'clock
    EYE_3_LED_MAP[0],   // Level 7: EYE_3 at 12 o'clock
    EYE_4_LED_MAP[0]    // Level 8: EYE_4 at 12 o'clock
};
```

**Speed Preview** (Horizontal line, 9 o'clock → 3 o'clock):

```cpp
const uint16_t SPEED_LED_POSITIONS[9] = {
    EYE_4_LED_MAP[18],  // Level 0: EYE_4 at 9 o'clock (left) - 3*24/4 - 1
    EYE_3_LED_MAP[12],  // Level 1: EYE_3 at 9 o'clock - 3*16/4 - 1
    EYE_2_LED_MAP[9],   // Level 2: EYE_2 at 9 o'clock - 3*12/4 - 1
    EYE_1_LED_MAP[6],   // Level 3: EYE_1 at 9 o'clock - 3*8/4 - 1
    EYE_0_RAW_START,    // Level 4: EYE_0 center (raw 160)
    EYE_1_LED_MAP[2],   // Level 5: EYE_1 at 3 o'clock (right) - 8/4 - 1
    EYE_2_LED_MAP[3],   // Level 6: EYE_2 at 3 o'clock - 12/4 - 1
    EYE_3_LED_MAP[4],   // Level 7: EYE_3 at 3 o'clock - 16/4 - 1
    EYE_4_LED_MAP[5]    // Level 8: EYE_4 at 3 o'clock - 24/4 - 1
};
```

**Usage** (Direct O(1) access):

```cpp
// Simple, fast access - no runtime conversion needed
uint16_t rawIndex = BRIGHTNESS_LED_POSITIONS[level];
leds[rawIndex] = CRGB::White;
```

### Segment Manager Integration

`SegmentManager` methods automatically use logical→raw mapping:

- `getSegmentLEDByPosition()` - Converts logical position to raw index
- `setSegmentLED()` - Accepts logical index, writes to raw index
- No `REVERSE_DIRECTION` flags needed - orientation handled by mapping arrays

## Joystick Input System

### Hardware & Configuration

**ADC Input**: 12-bit resolution (0-4095 range)

- `JOYSTICK_CENTER`: 1790 (calibrated neutral)
- `JOYSTICK_DEADZONE`: 300 (prevents jitter)
- `BUTTON_DEBOUNCE_MS`: 50ms
- `BUTTON_HOLD_DURATION`: 2000ms (mode switching threshold)

### Input Processing Pipeline

**1. Raw Read** → **2. Deadzone Filter** → **3. State Storage** → **4. Mode Distribution**

```cpp
// 1. Raw ADC read (0-4095)
rawX = analogRead(JOYSTICK_X_PIN);
rawY = analogRead(JOYSTICK_Y_PIN);

// 2. Deadzone: snap to center if within threshold
if (abs(rawX - JOYSTICK_CENTER) < JOYSTICK_DEADZONE) rawX = JOYSTICK_CENTER;

// 3. Store in joystickState struct
joystickState.x = rawX;
joystickState.y = rawY;

// 4. Calculate deltas for modes
deltaX = joystickState.x - JOYSTICK_CENTER;  // -1790 to +2305
deltaY = joystickState.y - JOYSTICK_CENTER;
magnitude = sqrt(deltaX² + deltaY²);
angle = atan2(x, y) * 180/π;
```

### Button State Machine

**Hold Detection** (2-second hold → main mode cycling):

```cpp
// Called continuously every readJoystick() cycle (every 10ms)
// Not just on button state change - allows detection while held

if (buttonPressed && !buttonHeldDown) {
    buttonHeldDown = true;
    buttonPressStartTime = currentTime;
    holdActionTriggered = false;
}

if (buttonPressed && buttonHeldDown && !holdActionTriggered) {
    holdDuration = currentTime - buttonPressStartTime;
    if (holdDuration >= BUTTON_HOLD_DURATION) {  // 2000ms
        holdActionTriggered = true;
        cycleHoldAction();
    }
}

if (!buttonPressed && buttonHeldDown) {
    buttonHeldDown = false;
    holdActionTriggered = false;
}
```

**Single-Click** (release after short press → sub-mode cycling):

```cpp
static bool wasPressed = false;
if (joystickState.buttonPressed) wasPressed = true;
if (!joystickState.buttonPressed && wasPressed && !holdActionTriggered) {
    cycleSingleClick();
    wasPressed = false;
}
```

### Coordinate Transformation & Usage

**Direction Mapping** (8 cardinal directions):

- Angle → Direction index (0-8): Center, N, NE, E, SE, S, SW, W, NW
- Used by: EyeRenderer for iris positioning

**Magnitude Mapping**:

- Linear: `map(magnitude, 0, MAX, 0, targetRange)`
- Used by: Jolt mode for expansion intensity
- Dynamic: `map(deltaY, JOLT_DEADZONE, JOLT_MAX, 1, 255)`

**Discrete Level Mapping**:

- `map(value, MIN, MAX, 1, 9)` → 9 discrete brightness/speed levels
- Includes hysteresis: change only when crossing thresholds

### Communication Flow

```
LEDDriver (owns joystickState)
    ↓ readJoystick() every 10ms
    ↓ processes button (hold/click detection)
    ↓ routes to active mode
    ├→ EyeRenderer(x, y) → angle → iris position
    ├→ JoltMode(y) → magnitude → ring expansion
    ├→ FireworkMode(y) → upward threshold → trigger launch
    ├→ PatternExplorer(x, y) → discrete selection
    └→ SettingsMode(x, y) → brightness/speed adjustment
```

**No Direct Communication**: Modes don't read joystick directly; LEDDriver passes processed values.

## Mode System

### Main Modes

- `MAIN_MODE_EXPLORER` (0) - Pattern/Color exploration
- `MAIN_MODE_INTERACTION` (1) - Interactive effects

### Explorer Sub-Modes

- `EXPLORER_SUBMODE_CLOCK_PATTERN` (0) - Clock patterns/palettes
- `EXPLORER_SUBMODE_CLOCK_SETTINGS` (1) - Clock brightness/speed
- `EXPLORER_SUBMODE_POLE_PATTERN` (2) - Pole patterns/palettes
- `EXPLORER_SUBMODE_POLE_SETTINGS` (3) - Pole brightness/speed

### Interaction Sub-Modes

#### `INTERACTION_SUBMODE_EYEBALL` (0) - Eye Tracking Mode

**Input**: Joystick X, Y position  
**Processing**:

```cpp
deltaX = joystickState.x - CENTER;
deltaY = joystickState.y - CENTER;
angle = atan2(deltaX, deltaY) * 180/π;  // -180° to +180°
direction = mapAngleToDirection(angle); // 0-8 (Center, N, NE, E, SE, S, SW, W, NW)
```

**Rendering**:

1. PatternManager updates clock ring with current pattern
2. If joystick active (outside deadzone):
   - Clear eye segments (5 rings)
   - EyeRenderer calculates iris position from direction
   - Render iris LEDs (preset positions for 9 directions)
   - Iris follows joystick movement in real-time

**Smoothing**: Optional `EYE_SMOOTHING_FACTOR` exponential averaging for fluid motion

#### `INTERACTION_SUBMODE_FIREWORK` (1) - Firework Launcher Mode

**Input**: Joystick Y position + button press  
**Trigger Condition**:

```cpp
upwardMotion = (joystickState.y - CENTER) > FIREWORK_THRESHOLD;
if (buttonPressed && upwardMotion) → trigger();
```

**Phases** (ActionPattern):

1. **PHASE_IDLE**: Displays clock pattern, waits for trigger
2. **PHASE_LAUNCH**: Brief visual cue (currently skipped, duration = 0)
3. **PHASE_EXPLOSION**: Rings expand from center (EYE_0 → EYE_4 → CLOCK)
   - Duration: 267ms (3x faster than original)
   - Each ring lights with palette color, brief delay between rings
4. **PHASE_FADEOUT**: 2-second gradual fade from full brightness to black
   - `fadeIntensity` linearly decreases: 1.0 → 0.0
   - All rings fade simultaneously

**One-Shot**: After fadeout completes, returns to PHASE_IDLE (displays pattern)

#### `INTERACTION_SUBMODE_JOLT` (2) - Magnitude-Based Expansion Mode

**Input**: Joystick Y (vertical), X (horizontal)  
**Vertical Processing** (magnitude):

```cpp
deltaY = joystickState.y - CENTER;  // Only upward motion considered
if (deltaY <= JOLT_DEADZONE_THRESHOLD) {
    magnitude = 0;  // Deadzone: show center LED only
} else {
    // Dynamic smooth mapping
    magnitude = map(deltaY, JOLT_DEADZONE, JOLT_MAX, 1, 255);
}
```

**Horizontal Processing** (palette selection):

```cpp
deltaX = joystickState.x - CENTER;
if (abs(deltaX) > DEADZONE && elapsed > 300ms) {
    if (deltaX > 0) nextPalette();
    if (deltaX < 0) prevPalette();
}
```

**Rendering** (magnitude-based expansion):

_Eye/Clock_ (6 total rings):

```cpp
ringExpansion = (magnitude / 255.0) * 6.0;  // 0.0 to 6.0
for (ring = 0; ring < 6; ring++) {
    ringProgress = min(1.0, ringExpansion - ring);  // 0.0 to 1.0
    color = palette.getColorSmooth(ring / 6.0);
    brightness = globalBrightness * ringProgress;
    fillRing(ring, color, brightness);
}
```

_Pole_ (expands from center up/down):

```cpp
expansionPercent = magnitude / 255.0;       // 0% to 100%
ledsToLight = expansionPercent * POLE_LEDS;
halfRange = ledsToLight / 2;
for (i = 0; i < halfRange; i++) {
    color = palette.getColorSmooth(i / halfRange);
    poleLeds[center + i] = color;  // Up
    poleLeds[center - i] = color;  // Down
}
```

**Deadzone State** (magnitude = 0):

- Center eye LED (EYE_0): White
- Middle pole LED: White
- All other LEDs: Black

### Special Modes

- `SPECIAL_MODE_SETTINGS` (99) - Quadrant settings interface
- `SPECIAL_MODE_CALIBRATION` (98) - Joystick calibration

## Power Optimization

### Wireless Management

When `ENABLE_WIFI_AP = false`:

- WiFi fully disabled (esp_wifi_deinit)
- Bluetooth fully disabled (esp_bt_controller_deinit)
- CPU frequency reduced to `POWER_OPTIMIZED_CPU_FREQ` (160MHz default)
- Power savings: ~95-105 mA (80% ESP32 reduction)

### Battery Life Impact

- ESP32 power: 115mA → 22mA (5.2x improvement)
- Total system improvement: 19-130% depending on LED usage

## Pattern System

### Pattern Types

- `Pattern` - Base class for clock/eye patterns
- `PolePattern` - Base class for pole patterns (supports palettes)
- `ActionPattern` - One-time triggerable effects (e.g., fireworks)

### Palette Support

- Clock/Eye: CRGBPalette16 via PatternManager
- Pole: ColorPalette via PolePattern::palette
- Jolt Mode: Uses `selectedJoltPaletteIndex` for dynamic palettes

### Global Settings

Pattern/palette explorers set global values:

- Clock: `selectedPatternIndex`, `selectedPaletteIndex`
- Pole: `selectedPolePatternIndex`, `selectedPolePaletteIndex`
- Other modes display these global settings

## Joystick System

### Calibration

- Center: ~1790 (12-bit ADC, 0-4095 range)
- Deadzone: 300
- Auto-calibration mode via special mode

### Button Input

- **Single Click** (quick press/release): Cycle sub-mode within current main mode
- **Hold (2 seconds)**: Cycle to next main mode
  - Duration configurable via `BUTTON_HOLD_DURATION` (default: 2000ms)
  - Threshold: 100ms minimum to distinguish from bounce
- Debounce: 200ms to prevent false triggers
- Hold detection tracks press duration and triggers action at threshold

### Input Mapping

- Jolt Mode: Dynamic mapping (deadzone→max = 1→255 magnitude)
- Eye Mode: atan2(x, y) for directional control
- Settings Mode: Quadrant-based (4 regions)

## Performance Notes

### Multi-Pattern Processing

- Fireworks: Max 5 simultaneous instances
- Each pattern updates independently
- ESP32 @ 160MHz handles calculations efficiently
- Teensy 4.1 unnecessary for current implementation

### Frame Rate

- Target: 60 FPS (16ms LED_UPDATE_INTERVAL)
- Joystick: 10ms read interval
- Pattern updates: Per-pattern timing control
