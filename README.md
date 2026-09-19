# RC Trash Sweeping Boat

A Wi-Fi-controlled floating trash collection boat built with a 3D-printed structure, an ESP32, two ESCs, and two motors.

The project is designed as a low-cost prototype for collecting floating plastic bottles and other lightweight waste from ponds, canals, and small water bodies.

## Features

- ESP32-based controller
- Two-motor differential thrust
- Wi-Fi Access Point mode
- Browser-based control panel
- Forward, reverse, and steering control
- Emergency stop button
- Automatic motor stop when the control signal is lost
- Compatible with a wide range of ESP32 boards that support the Arduino LEDC API

## Hardware

- ESP32 development board
- 2 × bidirectional ESCs
- 2 × DC brushed motors suitable for the boat
- Battery matched to the ESC and motor ratings
- 3D-printed boat body/pontoons
- Trash collection net or front sweeping mechanism
- Waterproof enclosure for electronics
- Fuse, power switch, wiring, and connectors
- Common ground between the ESP32 and ESC signal ground

## Wiring

| Component | ESP32 connection |
|---|---|
| Left ESC signal | GPIO 25 |
| Right ESC signal | GPIO 26 |
| Left ESC GND | ESP32 GND |
| Right ESC GND | ESP32 GND |
| ESC power input | Battery through suitable power wiring |
| ESP32 power | Regulated 5 V USB/VIN or suitable regulator |

### Important power note

Do not power the motors from the ESP32. The motors must receive power from the battery through their ESCs.

If the ESC has a BEC output, verify its voltage before connecting it to the ESP32. Use a suitable regulator if necessary.

## Software Requirements

- Arduino IDE
- ESP32 board package for Arduino IDE
- ESP32 Arduino Core 3.x recommended

## Installation

1. Install the ESP32 board package in Arduino IDE.
2. Open `rc_trash_boat_esp32.ino`.
3. Select your ESP32 board and COM port.
4. Confirm the ESC signal pins:
   - Left ESC: GPIO 25
   - Right ESC: GPIO 26
5. Upload the code.
6. Open Serial Monitor at `115200 baud`.
7. Connect your phone or computer to:

   **Wi-Fi name:** `TrashBoat-ESP32`  
   **Password:** `12345678`

8. Open the IP address shown in Serial Monitor. Usually it is:

   `http://192.168.4.1`

## ESC Calibration and Safety

This code assumes a bidirectional ESC with:

- 1000 µs = full reverse
- 1500 µs = neutral
- 2000 µs = full forward

Your ESC may use different values. Check its manual before operating.

### First test

1. Remove the propellers.
2. Place the boat on a stable stand.
3. Turn on the ESC and ESP32.
4. Confirm both motors remain stopped at neutral.
5. Test low throttle.
6. Check that forward and reverse directions are correct.
7. If one motor rotates in the wrong direction, reverse that motor's wiring or modify its command sign.

## Control Logic

The controller uses differential thrust:

- Throttle controls forward and reverse movement.
- Steering increases power on one side and decreases power on the other.
- If no command is received for 1 second, both motors automatically return to neutral.

## Customization

You can change:

```cpp
const int ESC_LEFT_PIN  = 25;
const int ESC_RIGHT_PIN = 26;
```

Wi-Fi settings:

```cpp
const char* AP_SSID = "TrashBoat-ESP32";
const char* AP_PASSWORD = "12345678";
```

ESC pulse limits:

```cpp
const int ESC_MIN_US = 1000;
const int ESC_NEUTRAL_US = 1500;
const int ESC_MAX_US = 2000;
```

## Limitations

- This version controls propulsion only.
- It does not include an automatic trash detection system.
- It does not include GPS, water-quality sensors, battery monitoring, or autonomous navigation.
- Waterproofing, buoyancy, motor sealing, and electrical isolation must be tested separately.
- The boat should be operated in controlled water and supervised at all times.

## Future Improvements

- Solar charging and battery voltage monitoring
- Ultrasonic or LiDAR obstacle detection
- Water-quality sensors
- GPS-based navigation
- Autonomous trash collection
- ESP32-CAM-based trash detection
- Live telemetry dashboard
- Motor current monitoring
- Emergency return-to-shore mode

## License

This project is released for educational and experimental use. Adapt the design and wiring according to your hardware specifications and local safety requirements.
