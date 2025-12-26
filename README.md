# Neema ESP32 - Window Blinds Tilt Control

An Arduino ESP32 project that controls window blind slats using a potentiometer and stepper motor. Turn the potentiometer to open or close the blinds by rotating the tilt wand mechanism. Perfect for automated or remote-controlled window blinds with WiFi capabilities.

## Features

- Smooth blinds slat control with acceleration/deceleration
- Potentiometer input for manual position control
- Configurable rotation range (default 180° for typical blinds)
- Real-time position feedback via Serial monitor
- Deadband filtering to reduce jitter from potentiometer noise
- 1/32 microstepping for precise, quiet operation
- Battery-powered or USB-C powered operation
- 12-bit ADC resolution for smoother position control
- ESP32-S3 platform ready for WiFi/Bluetooth future enhancements
- Easy configuration for different blind types

## Hardware Requirements

### Tested Configuration
This project has been configured for:
- **Arduino Nano ESP32-S3** - Board with 3.3V logic (NOT 5V!)
- **Stepperonline 17HS19-2004S1** - NEMA 17 stepper motor
  - Rated current: 2.0A per phase
  - Holding torque: 59Ncm (83.55 oz-in)
  - Step angle: 1.8° (200 steps/revolution)
  - Resistance: 1.5Ω per phase
- **Pololu DRV8825 Stepper Driver**
  - Supports up to 2.2A per coil (sufficient for 2.0A motor)
  - 1/32 microstepping capability
  - Built-in current limiting
  - Compatible with 3.3V logic levels
- **10K Potentiometer** (3-terminal linear taper)
- **12V Battery** for motor power (or external power supply)
- **USB-C cable** for programming and optional 5V power to ESP32
- Jumper wires and breadboard for prototyping

## ⚠️ CRITICAL: ESP32-S3 uses 3.3V Logic (NOT 5V!)

**IMPORTANT:** The ESP32-S3 operates at 3.3V logic levels. Connecting 5V to any GPIO pin can damage the board!

**All connections must use 3.3V:**
- Potentiometer power: 3.3V (NOT 5V)
- DRV8825 VDD: 3.3V (NOT 5V)
- DRV8825 M0, M1, M2: 3.3V (NOT 5V)

**12V connections are OK:**
- DRV8825 VMOT can still use 12V (this is motor power, isolated from logic)

### Wiring Diagram

#### Potentiometer
- Middle pin → ESP32 A0 (GPIO1)
- One outer pin → ESP32 **3.3V** (NOT 5V!)
- Other outer pin → ESP32 GND

#### DRV8825 Stepper Driver Connections
**Control Pins:**
- STEP → ESP32 GPIO5
- DIR → ESP32 GPIO6
- ENABLE → ESP32 GPIO7
- SLEEP → Connect to RESET pin (or 3.3V to keep enabled)
- RESET → Connect to SLEEP pin (or 3.3V)

**Microstepping Configuration (for 1/32 mode):**
- M0 → ESP32 **3.3V** (NOT 5V!)
- M1 → ESP32 **3.3V** (NOT 5V!)
- M2 → ESP32 **3.3V** (NOT 5V!)

**Power:**
- VMOT → 12V battery positive (motor power)
- GND → 12V battery negative AND ESP32 GND (common ground - CRITICAL!)
- VDD → ESP32 **3.3V** (logic power, NOT 5V!)

**Motor Connections:**
- A1, A2 → Motor coil A (one pair from motor)
- B1, B2 → Motor coil B (other pair from motor)

#### Arduino Nano ESP32-S3 Power Options

**Option 1: USB-C Power** (Recommended for development/testing)
- Connect USB-C cable to ESP32
- Board provides 3.3V automatically
- 12V battery powers motor only (via DRV8825 VMOT)
- Common ground between ESP32 and battery

**Option 2: External 12V to VIN** (Battery operation)
- 12V battery positive → ESP32 VIN pin
- 12V battery negative → ESP32 GND
- ESP32's onboard regulator provides 3.3V
- Same 12V powers motor via DRV8825 VMOT

**Important:** The DRV8825 can get HOT. Consider adding a heatsink if running continuously at high current.

**CRITICAL - Common Ground:** All components MUST share a common ground. If using multiple power supplies (e.g., USB for ESP32 and 12V for motor), ensure all grounds are connected together. Without common ground, the motor may energize but not step.

### Wiring Comparison: Nano Every vs ESP32-S3

If you're migrating from the Nano Every version, here are the wiring changes:

| Connection | Nano Every | Nano ESP32-S3 | Notes |
|------------|------------|---------------|-------|
| Potentiometer + | 5V | **3.3V** | ⚠️ CRITICAL CHANGE |
| Potentiometer middle | A0 | A0 (GPIO1) | Same physical pin |
| Potentiometer - | GND | GND | No change |
| DRV8825 STEP | Pin 3 | GPIO5 | Pin number change |
| DRV8825 DIR | Pin 2 | GPIO6 | Pin number change |
| DRV8825 ENABLE | Pin 4 | GPIO7 | Pin number change |
| DRV8825 VDD | 5V | **3.3V** | ⚠️ CRITICAL CHANGE |
| DRV8825 M0, M1, M2 | 5V | **3.3V** | ⚠️ CRITICAL CHANGE |
| DRV8825 VMOT | 12V | 12V | No change |
| DRV8825 GND | GND | GND | Common ground required |
| Motor coils | A1,A2,B1,B2 | A1,A2,B1,B2 | No change |

### DRV8825 Current Limiting Setup (CRITICAL!)

**You MUST set the current limit on the DRV8825 before powering the motor.** Failure to do so can damage the motor or driver.

For the 17HS19-2004S1 (2.0A motor), set current limit to approximately **1.4A** (70% of rated current):

1. **Tools needed:**
   - Small flathead screwdriver (for trimpot adjustment)
   - Multimeter

2. **Procedure:**
   - Power the DRV8825 (12V to VMOT, GND connected, ESP32 powered)
   - Set multimeter to DC voltage mode
   - Measure voltage between the trimpot wiper and GND
   - Adjust trimpot to get **0.7V** (Vref = Current / 2 = 1.4A / 2)
   - Start low and gradually increase while testing motor

3. **Formula:**
   ```
   Current Limit = Vref × 2
   Target: 1.4A → Vref = 0.7V
   ```

4. **Testing:**
   - Start with Vref around 0.5V (1.0A) for initial testing
   - If motor skips steps or lacks torque, increase slightly
   - If motor gets too hot, decrease current
   - Don't exceed 0.8V (1.6A) for this motor

## Software Requirements

### Board Support
Install ESP32 board support in Arduino:

**Using Arduino IDE:**
1. File → Preferences
2. Add to "Additional Board Manager URLs":
   ```
   https://espressif.github.io/arduino-esp32/package_esp32_index.json
   ```
3. Tools → Board → Boards Manager
4. Search for "esp32" and install "esp32 by Espressif Systems"
5. Select Board: "Arduino Nano ESP32"

**Using Arduino CLI:**
```bash
# Add ESP32 board support
arduino-cli config add board_manager.additional_urls https://espressif.github.io/arduino-esp32/package_esp32_index.json
arduino-cli core update-index
arduino-cli core install esp32:esp32
```

### Libraries
This project requires the AccelStepper library:

**Using Arduino IDE:**
1. Open Arduino IDE
2. Go to Sketch → Include Library → Manage Libraries
3. Search for "AccelStepper"
4. Install "AccelStepper" by Mike McCauley

**Using Arduino CLI:**
```bash
arduino-cli lib install AccelStepper
```

## Installation

1. Clone this repository or download the sketch
2. Install ESP32 board support (see above)
3. Install the AccelStepper library (see above)
4. Wire up all components according to wiring diagram
   - **Double-check all 3.3V connections! Do NOT use 5V!**
5. **Set DRV8825 current limit** (see section above - DO THIS FIRST!)
6. Connect Arduino Nano ESP32 via USB-C
7. Upload the sketch:

**Using Arduino IDE:**
- Select Board: "Arduino Nano ESP32"
- Select Port: Your USB port
- Click Upload

**Using Arduino CLI:**
```bash
# Compile and upload
arduino-cli compile --fqbn esp32:esp32:arduino_nano_esp32 neema_esp32.ino
arduino-cli upload -p /dev/ttyUSB0 --fqbn esp32:esp32:arduino_nano_esp32 neema_esp32.ino

# Monitor serial output
arduino-cli monitor -p /dev/ttyUSB0 -c baudrate=115200
```

## Configuration

You can adjust the following parameters in the sketch to customize for your specific blinds:

### Pin Configuration
```cpp
const int POT_PIN = A0;        // Potentiometer input (GPIO1)
const int STEP_PIN = 5;        // Step pulse pin (GPIO5)
const int DIR_PIN = 6;         // Direction pin (GPIO6)
const int ENABLE_PIN = 7;      // Enable pin (GPIO7)
```

### Motor Configuration
```cpp
const int STEPS_PER_REVOLUTION = 200;  // 17HS19-2004S1: 1.8° per step
const int MICROSTEPS = 32;             // DRV8825: 1/32 microstepping (M0,M1,M2 HIGH)
const int TOTAL_STEPS = 6400;          // 200 × 32 = 6400 steps per revolution
```

### Blinds-Specific Settings
```cpp
const int ROTATION_RANGE = TOTAL_STEPS / 2;  // 180° (half rotation)
// Adjust this value based on your blinds:
// - TOTAL_STEPS / 4 = 90° rotation
// - TOTAL_STEPS / 2 = 180° rotation  (typical for most blinds)
// - TOTAL_STEPS = 360° full rotation
```

### Motion Parameters
```cpp
const int MAX_SPEED = 2000;        // Maximum steps per second
const int ACCELERATION = 1000;     // Steps per second²
```

### Potentiometer Settings
```cpp
const int POT_DEADBAND = 10;       // Increase to reduce sensitivity/jitter
const int POT_MAX = 4095;          // ESP32 has 12-bit ADC (0-4095)
```

## Usage

1. Ensure DRV8825 current limit is set correctly
2. **Double-check all 3.3V connections (NOT 5V!)**
3. Power the system with 12V battery or USB-C
4. Upload the sketch to Arduino Nano ESP32
5. Open Serial Monitor (115200 baud) for debugging
6. Turn the potentiometer - the motor rotates the tilt wand
7. Blinds slats should open/close following the potentiometer position

### First-Time Setup Tips

1. **Test without blinds attached first**
   - Verify motor rotates smoothly
   - Check rotation direction (swap motor coil pairs if needed)
   - Ensure no skipping or stuttering

2. **Attach to blinds tilt wand**
   - You may need a coupling or adapter to connect motor shaft to wand
   - Common options: flexible shaft coupling, 3D printed adapter, or direct friction fit

3. **Calibrate rotation range**
   - Note pot positions for fully open and fully closed
   - Adjust `ROTATION_RANGE` if needed (typically 90-180°)

### Serial Output Example
```
Neema ESP32 - Potentiometer Stepper Controller
==============================================
Setup complete!
Board: Arduino Nano ESP32-S3 (3.3V logic)
ADC Resolution: 12-bit (0-4095)
Steps per revolution: 6400
Rotation range: 3200 steps (180 degrees)
Max speed: 2000
Acceleration: 1000

Pot: 2048 | Target: 1600 | Current: 1598
Pot: 3072 | Target: 2400 | Current: 2398
Pot: 1024 | Target: 800 | Current: 802
```

## Troubleshooting

### Motor doesn't move at all
- **Check DRV8825 current limit** - Must be set before motor will work properly
- **Verify 3.3V connections** - Ensure VDD, M0, M1, M2 are connected to 3.3V (NOT 5V!)
- Verify all wiring connections (especially SLEEP and RESET tied together or to 3.3V)
- Ensure 12V battery is charged and providing adequate voltage
- Check that ENABLE pin is LOW (sketch sets this automatically)
- Verify M0, M1, M2 pins are HIGH for 1/32 microstepping
- Measure VDD on DRV8825 should be 3.3V

### Motor whirs/vibrates but doesn't turn
- **MOST COMMON:** Missing common ground between power supplies
  - If using USB for ESP32 and separate 12V for motor, ALL grounds must be connected together
  - ESP32 GND, DRV8825 GND, and all power supply grounds must be common
  - Without common ground, control signals don't work properly
- **Current too low** - Increase Vref on DRV8825 (try 0.6-0.7V)
- **Speed too high** - Motor can't keep up, reduce MAX_SPEED to 400 for testing
- Motor shaft has mechanical resistance - check coupling/mounting

### Motor jitters or stutters
- **Current too low** - Increase Vref on DRV8825 slightly (don't exceed 0.8V)
- **Current too high** - Motor will get hot; decrease Vref
- Increase `POT_DEADBAND` to reduce sensitivity (try 15-20 for 12-bit ADC)
- Lower `MAX_SPEED` if motor is skipping steps (try 1000-1500)
- Ensure stable 12V power supply (weak battery will cause issues)
- Add 100µF capacitor across DRV8825 VMOT and GND

### Erratic movement or jumpy response
- Check potentiometer wiring (especially middle pin to A0)
- Verify potentiometer is good quality (cheap pots can be noisy)
- Increase `POT_DEADBAND` value to 15-20 (ESP32 has more sensitive 12-bit ADC)
- Add 0.1µF capacitor between A0 and GND for noise filtering
- Ensure common ground between ESP32 and DRV8825

### ESP32 won't program / upload fails
- Press and hold BOOT button while clicking Upload
- Try a different USB-C cable (some are charge-only)
- Ensure correct board selected: "Arduino Nano ESP32"
- Check COM port selection

### Motor gets too hot
- **Current limit too high** - Reduce Vref on DRV8825 (target 0.7V max)
- Add heatsink to DRV8825 chip
- Ensure adequate airflow
- Consider reducing holding current when motor is idle

### Wrong rotation direction
- Swap one motor coil pair (either A1/A2 or B1/B2, not both)
- Or modify code to invert direction

### Blinds-Specific Issues

**Motor doesn't have enough torque:**
- Increase current limit slightly (up to 0.8V Vref)
- Reduce `MAX_SPEED` for more torque
- Check tilt wand mechanism isn't binding
- Verify motor is properly coupled to wand

**Blinds don't fully open/close:**
- Adjust `ROTATION_RANGE` to match your blinds' actual travel
- Typical blinds use 90-180° of rotation
- Monitor serial output to see actual position values

**Battery drains too quickly:**
- Motor holds position using current even when stopped
- Consider adding auto-sleep after period of inactivity
- Use lower current limit if holding torque not critical

## Future Enhancements

Potential improvements leveraging ESP32-S3 capabilities:

- [ ] **WiFi control** - Web interface or MQTT for remote control
- [ ] **Bluetooth control** - Smartphone app integration
- [ ] **Auto-sleep mode** - Disable motor after inactivity to save battery
- [ ] **Position presets** - Buttons for "fully open", "fully closed", "50%" positions
- [ ] **NVS position storage** - Remember last position on power loss (using ESP32 flash)
- [ ] **Scheduled operation** - Built-in RTC for automatic open/close times
- [ ] **Light sensor integration** - Auto-close blinds when too bright
- [ ] **Multiple blinds control** - Single controller for several windows via WiFi
- [ ] **Manual override detection** - Detect if blinds moved manually and resync
- [ ] **Battery voltage monitoring** - Alert when battery needs charging (via ADC)
- [ ] **Current sensing** - Detect if motor stalls (blind jammed)
- [ ] **Over-the-air (OTA) updates** - Update firmware wirelessly
- [ ] **Home Assistant integration** - Smart home control

## License

This project is open source. Feel free to modify and use for your own projects.

## Author

Created for the Shed Computer Arduino projects collection.

## Bill of Materials

| Component | Part Number/Model | Approx. Cost |
|-----------|------------------|--------------|
| Arduino Nano ESP32 | ESP32-S3 | $15-20 |
| Stepper Motor | Stepperonline 17HS19-2004S1 | $10-15 |
| Stepper Driver | Pololu DRV8825 | $8-12 |
| Potentiometer | 10K linear taper | $1-3 |
| 12V Battery | Li-ion or SLA | $15-30 |
| USB-C Cable | Data + Power capable | $3-8 |
| Misc. | Wires, breadboard, heatsink | $5-10 |

**Total estimated cost:** $55-95 USD

## References

- [AccelStepper Library Documentation](http://www.airspayce.com/mikem/arduino/AccelStepper/)
- [Stepperonline 17HS19-2004S1 Product Page](https://www.omc-stepperonline.com/nema-17-bipolar-59ncm-84oz-in-2a-42x48mm-4-wires-w-1m-cable-connector-17hs19-2004s1)
- [Pololu DRV8825 Stepper Driver](https://www.pololu.com/product/2133)
- [DRV8825 Current Limiting Guide](https://www.pololu.com/product/2133#current-limiting-guide)
- [Arduino Nano ESP32 Documentation](https://docs.arduino.cc/hardware/nano-esp32)
- [ESP32-S3 Datasheet](https://www.espressif.com/sites/default/files/documentation/esp32-s3_datasheet_en.pdf)
