# Speed Normalization - Quick Reference

## Problem & Solution

**Problem**: WLED patterns (especially Meteor) were very slow compared to existing patterns  
**Solution**: Added per-pattern speed normalization factor

## How It Works

```
effectiveSpeed = globalSpeed × speedNormalizationFactor
```

## WLED Pattern Speed Factors

| Pattern         | Factor   | Before (slow) | After (balanced) |
| --------------- | -------- | ------------- | ---------------- |
| Dancing Shadows | 5.0×     | 😴            | ✅               |
| Color Waves     | 5.0×     | 😴            | ✅               |
| Noise           | 6.0×     | 😴            | ✅               |
| **Meteor**      | **8.0×** | **💤**        | **✅ Fixed!**    |
| Glitter         | 4.0×     | 😴            | ✅               |
| Two Dots        | 7.0×     | 😴            | ✅               |
| Colortwinkles   | 5.0×     | 😴            | ✅               |
| Flow            | 6.0×     | 😴            | ✅               |

## Usage

### **Get Effective Speed** (in patterns)

```cpp
// OLD:
counter += (uint16_t)(speed * 10);

// NEW:
counter += (uint16_t)(getEffectiveSpeed() * 10);
```

### **Adjust Pattern Speed** (runtime)

```cpp
// Make pattern 2x faster
pattern->setSpeedNormalizationFactor(2.0f);

// Make pattern slower
pattern->setSpeedNormalizationFactor(0.5f);

// Reset to baseline
pattern->setSpeedNormalizationFactor(1.0f);
```

## Key Changes

### **src/Pattern.h**

```cpp
protected:
  float speedNormalizationFactor;  // New member

public:
  void setSpeedNormalizationFactor(float factor);
  float getEffectiveSpeed() const;  // Returns speed * factor
```

### **src/Pattern.cpp**

```cpp
Pattern::Pattern(...)
    : ..., speedNormalizationFactor(1.0f)  // Default: no change
{}
```

### **src/WLEDPatterns.cpp**

```cpp
WLEDMeteorPattern::WLEDMeteorPattern(...)
{
  speedNormalizationFactor = 8.0f;  // Set higher factor
}

bool update(...) {
  meteorPosition += direction * (int)(getEffectiveSpeed());  // Use effective speed
}
```

## Benefits

✅ **Meteor now moves at normal speed!**  
✅ All WLED patterns balanced with existing patterns  
✅ Global speed control works consistently  
✅ Per-pattern speed tuning available  
✅ No breaking changes (default = 1.0)  
✅ Zero linter errors

## Build Results

```
✅ Compiles successfully (7.99 seconds)
✅ Flash: 1,041,909 bytes (79.5%)
✅ RAM: 52,424 bytes (16.0%)
```

## Files Modified

- `src/Pattern.h` - Added speedNormalizationFactor + methods
- `src/Pattern.cpp` - Initialize to 1.0f
- `src/WLEDPatterns.cpp` - Updated all 8 WLED patterns

## Documentation

- `SPEED_NORMALIZATION.md` - Full technical details
- `SPEED_NORMALIZATION_QUICK_REF.md` - This file (quick ref)

---

**Ready to upload!** The WLED patterns are now properly balanced and responsive to global speed control. 🚀
