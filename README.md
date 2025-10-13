# PurrBit

**Motion controlled by chaos** 🐱

An intelligent, interactive cat toy featuring unpredictable movement patterns powered by an ATtiny412 microcontroller and CST6118 motor driver. Keep your feline friend entertained with three distinct play modes ranging from gentle to absolutely chaotic.

## Features

### 🎮 Three Play Modes

- **CHILL Mode** - Gentle, slow movements for calm play sessions
  - 60% speed with predictable alternating patterns
  - 2 seconds forward → 1 second pause → 2 seconds reverse → 3 seconds pause
  - Perfect for senior cats or initial introduction

- **WARMUP Mode** - Moderate intensity with surprise bursts
  - Random forward/reverse sequences
  - 50-80% speed with medium chaos
  - Unpredictable bursts keep cats engaged

- **CRAZY Mode** - Maximum chaos for high-energy cats
  - Rapid, erratic movements with direction changes
  - 60-100% speed with random patterns
  - Includes quick pulses, rapid reversals, and full-power bursts

### 🔄 Bidirectional Motor Control

- True forward and reverse motion for realistic prey-like behavior
- PWM-controlled speed for smooth operation
- Hardware-enforced 70% duty cycle limiter for motor safety

### 🛡️ Safety Features

- **20-second placement pause** when changing modes
  - Motor stops completely to allow safe repositioning
  - Prevents startling your cat with sudden movements
  
- **Auto-sleep after 10 minutes**
  - Automatic power down for battery conservation
  - Prevents over-stimulation

- **Manual sleep mode**
  - Long press (1.5 seconds) on button to manually enter sleep
  - Wake up with a quick button press

### 💡 User Interface

- **Single button control**
  - Short press: Cycle through play modes (CHILL → WARMUP → CRAZY)
  - Long press: Enter sleep mode
  
- **LED status indicators**
  - Visual feedback for current play mode
  - Red and green LED combinations

## Hardware

### Components

- **Microcontroller**: ATtiny412 (8-bit AVR with 4KB Flash)
- **Motor Driver**: CST6118 single-channel DC motor driver
- **Control**: Single tactile button with hardware pull-up
- **Indicators**: Red and green status LEDs

### Pin Configuration

| Pin | Function | Description |
|-----|----------|-------------|
| PA6 | Motor INA | TCD0 WOA PWM output |
| PA7 | Motor INB | GPIO for direction control |
| PA1 | Button | Mode control with internal pull-up |
| PA2 | Green LED | Status indicator |
| PA3 | Red LED | Status indicator |

### Motor Control Modes

- **Forward (Mode A)**: PA6=PWM, PA7=LOW (conduction ↔ standby)
- **Reverse (Mode B)**: PA6=PWM, PA7=HIGH (conduction ↔ brake, duty inverted)

## Building & Flashing

This project uses [PlatformIO](https://platformio.org/) for development and building.

### Prerequisites

- PlatformIO Core or PlatformIO IDE
- UPDI programmer (or USB-to-serial adapter configured for UPDI)

### Build

```bash
pio run
```

### Upload

Connect your UPDI programmer to `/dev/ttyUSB0` (or modify `platformio.ini` for your setup):

```bash
pio run --target upload
```

### Configuration

All timing and behavior parameters can be customized in `src/config.h`:

- `DEBOUNCE_MS` - Button debounce time (default: 40ms)
- `LONG_PRESS_MS` - Long press threshold (default: 1500ms)
- `AUTO_SLEEP_MS` - Auto-sleep timer (default: 10 minutes)
- `MODE_PAUSE_MS` - Placement pause duration (default: 20 seconds)
- `MAX_DUTY_PERCENT` - Motor duty cycle limiter (default: 70%)

## License

See [LICENSE](LICENSE) file for details.

## Contributing

This is a personal cat toy project, but suggestions and improvements are welcome! Feel free to open issues or submit pull requests.

---

**Made with ❤️ for cats who need more chaos in their lives**

