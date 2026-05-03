# PurrBit

A cat toy built around an ATtiny412 and a CST6118 motor driver. Moves a string in one of three intensity modes.

## Modes

- **CHILL** — slow, predictable movements: 2s forward, pause, 2s reverse, pause. Speed 60%.
- **WARMUP** — random directions, speed 50–80%, occasional short bursts.
- **CRAZY** — erratic direction changes, speed 60–100%, pulses and sharp reversals.

A short button press cycles through modes. On each switch the motor stops for 20 seconds so you can reposition the toy. After 10 minutes of inactivity the device sleeps on its own. A long press (1.5 s) puts it to sleep manually; a short press wakes it back up.

## Hardware

| Component | Part |
|-----------|------|
| MCU | ATtiny412 |
| Motor driver | CST6118 |
| Button | tactile, internal pull-up |
| Indicators | red + green LED |

### Pin mapping

| Pin | Function |
|-----|----------|
| PA6 | Motor INA (TCD0 WOA PWM) |
| PA7 | Motor INB (GPIO) |
| PA1 | Button |
| PA2 | Green LED |
| PA3 | Red LED |

Forward: PA6=PWM, PA7=LOW. Reverse: PA6=PWM, PA7=HIGH (duty inverted). Hardware duty cycle cap: 70%.

## Build & flash

The project uses [PlatformIO](https://platformio.org/).

```bash
# build
pio run

# flash (UPDI on /dev/ttyUSB0)
pio run --target upload
```

Change the programmer port in `platformio.ini` if needed.

## Configuration

Timing and behaviour parameters live in `src/config.h`:

| Parameter | Default |
|-----------|---------|
| `DEBOUNCE_MS` | 40 ms |
| `LONG_PRESS_MS` | 1500 ms |
| `AUTO_SLEEP_MS` | 10 min |
| `MODE_PAUSE_MS` | 20 s |
| `MAX_DUTY_PERCENT` | 70% |
