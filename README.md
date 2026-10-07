# IoT Smart Irrigation System

An ESP32 prototype that monitors a water reservoir, soil moisture, and battery voltage while exposing pump and servo controls through Blynk. The checked-in firmware combines automatic reservoir management with manual irrigation control.

> [!IMPORTANT]
> The current sketch is a development prototype. It contains two GPIO conflicts and has no loss-of-connection fail-safe. Resolve the issues in [Known limitations](#known-limitations) before connecting pumps or deploying the system unattended.

## Repository contents

| File | Purpose |
| --- | --- |
| `blynk.ino` | ESP32 firmware for sensing, Blynk telemetry, pump control, and one servo axis |

No circuit diagram, enclosure design, PCB files, or bill of materials is included, so the electrical implementation must be adapted and verified for the actual hardware.

## Intended hardware

- ESP32 development board
- HC-SR04-compatible ultrasonic distance sensor for reservoir level
- Analog soil-moisture sensor
- Battery-voltage divider suitable for a 3.3 V ADC
- Two correctly rated pump drivers or relay modules
- One or two servo motors
- Optional photoresistors; four LDR pins are declared, but the current firmware does not read them
- Separate, adequately rated power supplies for motors/pumps and logic, with a common reference where required

The project uses the Arduino ESP32 core together with the Blynk and ESP32Servo libraries.

## Current pin map

| Function | ESP32 GPIO | Notes |
| --- | ---: | --- |
| Ultrasonic trigger | 32 | Digital output |
| Ultrasonic echo | 33 | Digital input; level shifting may be required if the sensor outputs 5 V |
| Soil-moisture input | 34 | Input-only ADC pin |
| Battery-voltage input | 34 | **Conflicts with the soil sensor** |
| Reservoir pump output | 25 | Driver/relay control, not a direct pump connection |
| Irrigation pump output | 26 | Driver/relay control, not a direct pump connection |
| LDR inputs | 35, 36, 39, 27 | Declared but not sampled |
| X-axis servo | 14 | Controlled from Blynk |
| Y-axis servo | 27 | **Conflicts with LDR4** and is not otherwise controlled |

## Blynk channels

| Virtual pin | Direction | Value |
| --- | --- | --- |
| `V0` | Device to cloud | Calculated battery voltage |
| `V1` | Device to cloud | Raw soil-moisture ADC reading |
| `V2` | Device to cloud | X-axis servo position |
| `V3` | Device to cloud | Ultrasonic distance in centimetres |
| `V4` | Cloud to device | Manual irrigation-pump state |
| `V5` | Cloud to device | X-axis servo angle |

## Setup

1. Install Arduino IDE or PlatformIO with ESP32 board support.
2. Install `Blynk` and `ESP32Servo` through the library manager.
3. Create a Blynk template and device, then replace the placeholder template ID, template name, authentication token, Wi-Fi SSID, and password in `blynk.ino`.
4. Assign separate, valid GPIOs to `SOIL_SENSOR_PIN` and `BATTERY_PIN`. If the second servo is needed, also separate `LDR4` and the Y-axis servo pin; otherwise remove the unused declaration and attachment.
5. Verify the voltage divider, sensor voltage levels, relay polarity, pump driver ratings, and a common ground before applying power.
6. Select the correct ESP32 board and serial port, compile, upload, and open the Serial Monitor at 115200 baud.
7. Configure Blynk widgets for `V0` through `V5` using the channel table above.

## Runtime behavior

Every two seconds the firmware measures ultrasonic distance and both ADC values, then publishes telemetry to Blynk. It treats a larger ultrasonic distance as a lower water level:

- at 17 cm or more, the reservoir pump output is set `HIGH`;
- at 10 cm or less, the reservoir pump output is set `LOW`;
- between the two thresholds, the previous output state is retained to provide hysteresis.

The irrigation pump is controlled manually with `V4`. `V5` maps directly to the X-axis servo command; the dashboard should restrict that value to a safe 0-180 degree range.

Whether `HIGH` means a pump is on depends on the selected driver or relay module. Confirm this before testing.

## Calibration

- Record dry and wet soil-sensor readings in the installed medium. The declared `SOIL_THRESHOLD` value is currently unused.
- Measure the actual full and low reservoir distances before changing `LEVEL_HIGH` and `LEVEL_LOW`.
- Derive the battery conversion from the physical divider topology and resistor values. The current code assumes a 12-bit, 3.3 V ADC and applies a fixed divider ratio; ESP32 ADC readings are not perfectly linear and should be checked against a multimeter.
- Keep every ADC input at or below the board's permitted voltage.

## Known limitations

- GPIO 34 is assigned to both soil moisture and battery voltage, so the two reported values currently come from the same electrical input.
- GPIO 27 is assigned to both `LDR4` and the Y-axis servo.
- The battery calculation declares `R1 = 10 kOhm` and `R2 = 100 kOhm` but multiplies the ADC voltage by only 1.1. That may be correct only for a particular resistor placement; it is not the usual factor for a divider with 100 kOhm on the battery side and 10 kOhm on the ADC side. Verify the physical divider and correct the equation before trusting the reported voltage.
- LDR readings, the soil threshold, and Y-axis motion are not implemented.
- There is no pump shutoff for a stalled sensor, Wi-Fi/Blynk loss, maximum runtime, dry running, overcurrent, or overflow.
- `pulseIn()` has no project-specific timeout or validation; a missing echo can delay control and be interpreted as an invalid level.
- Pump outputs are not explicitly initialized to a documented safe state before network connection.
- Credentials are stored in source code. Use a local secrets file or provisioning mechanism before sharing deployable firmware.

## Safety

Never power a pump or servo directly from an ESP32 GPIO. Use isolated or protected drivers, flyback suppression for inductive loads, fusing, waterproof connectors, and an emergency disconnect. Test first with pumps disconnected, then with a low-energy bench setup. Water, batteries, and mains-powered supplies require suitable enclosure, grounding, strain relief, and supervision.

## Project status

The repository captures an early functional concept rather than a production-ready irrigation controller. A useful next milestone is to resolve the pin conflicts, add explicit safe-state and timeout handling, calibrate the sensors, and document a verified wiring diagram.
