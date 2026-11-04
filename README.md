# ESP32 Smart Blinds (Arduino Cloud)

A DIY motorised blind controller. An ESP32 drives two DC gear motors through a TB6612FNG motor driver and is controlled from the **Arduino Cloud (IoT Cloud) dashboard**, so you can open and close the blinds from a phone or browser (and from voice assistants, via Arduino Cloud's Alexa/Google integrations, since the main property is a `CloudLight`).

The blinds are moved by pulling the bead (ball) chain with a drive wheel on each motor. Each motor is stopped when a magnet (reed) switch shows that its drive wheel has **stalled** at the end of the blind's travel. A maximum run time acts only as a safety timeout.

## How it works

### Cloud variables (Thing properties)

| Variable | Type | Access | Purpose |
|---|---|---|---|
| `blinds` | `CloudLight` | read/write | Main control. **On = open (up), Off = closed (down).** Triggers the full automatic move. |
| `blindSwitch` | `bool` | read/write | Manual mode only: selects which motor the Up/Down buttons drive (see below). |
| `up` | `bool` | read/write | Manual jog: hold to run the selected motor up, release to stop. |
| `down` | `bool` | read/write | Manual jog: hold to run the selected motor down, release to stop. |
| `uptime` | `int` | read only | Seconds since boot, updated every ~10 s. Handy as a "is it alive" indicator. |

Suggested dashboard widgets: a Switch/Light for `blinds`, a Switch for `blindSwitch`, two Push Buttons for `up`/`down`, and a Value/Chart for `uptime`.

### Automatic open/close

When `blinds` changes, the sketch moves the **two motors one after the other** (motor 1 first, then motor 2), at full speed for up to `BLIND_UP_SECONDS` / `BLIND_DOWN_SECONDS` (10 s in the current sketch), stopping early on a stall (see below). They deliberately run one at a time, never together (see [Why only one motor at a time](#why-only-one-motor-at-a-time)). Details:

- The sketch tracks a software state (`opened` / `closed` / `unknown`). Asking for the state it already thinks it is in does nothing.
- Commands received while the blinds are moving are ignored.
- The first change message received after boot is ignored (the cloud re-syncs stored values on connect, and this stops the blinds moving every time the board restarts). **The initial state is "unknown", so the first real command after power-up will move the blinds** regardless of where they physically are.
- The state is not stored in flash, so it is lost on reboot. If the blinds and the software state disagree, use the manual buttons to put the blinds where you want them.

### Stall detection (magnet sensors)

Each motor has a magnet reed sensor (`MAGR` for motor 1, `MAGL` for motor 2) that toggles as the drive wheel turns. While a motor runs in an automatic move, the sketch reads its sensor every 10 ms loop. If the sensor value has not changed for more than 30 reads, and the motor has been running for at least 30 loops, the motor is declared stalled ("Motor 1 Stall" / "Motor 2 Stall" in the serial log) and the sketch moves on to the next motor, or finishes. This is how the blinds stop at the top and bottom without limit switches. The fixed run time is only an upper bound if no stall is seen.

- Stall detection is used for automatic moves only, not the manual `up` / `down` buttons.
- The sketch notes that the check may be susceptible to switch bounce if the magnet rests close to the sensor.
- The sketch prints several serial lines every 10 ms while moving, which makes the serial log very noisy.

### Manual mode

`blindSwitch` chooses the motor, then `up` / `down` run it while held: `blindSwitch` = Off drives motor 2, On drives motor 1. This is for trimming each side individually (e.g. if the two sides drift out of level) and is ignored while an automatic move is in progress.

### Why only one motor at a time

Do not change the code to run both motors together without upgrading the hardware. Each motor is rated 0.92 A, so two need about 1.84 A at rated load (more at start-up or stall), which is above the 12 V / 1.7 A supply. The TB6612FNG breakout has no heatsink and cannot dissipate the heat of driving both. A driver was destroyed when this was tried. If you want simultaneous motion, use a larger supply, motors with lower current draw, and a driver with proper heatsinking (or a separate driver per motor).

### Tuning

Edit these `#define`s in `BlindsESP32_apr16a.ino`:

- `BLIND_UP_SECONDS`, `BLIND_DOWN_SECONDS` – maximum run time for each motor; set a little longer than a full travel so stall detection normally ends the move.
- `BLIND_UP` / `BLIND_DOWN` – motor direction (+1 / -1); the motor driver always runs at full speed.
- `offsetA`, `offsetB` – set to `-1` to flip a motor's direction if it spins the wrong way.

Mechanical note from the original build: drive wheel diameter is 30.6 mm (≈96 mm circumference), and the bead chain has 16 balls per wheel revolution.

## CAD

The motor mount is modelled in Tinkercad: [Blinds motor mount](https://www.tinkercad.com/things/eyJbsuSWBMM-blinds-motor-mount?sharecode=2IoUn7R2jkvfn2GjVcyLNe40DJcVV0ZCj15O-5fbk3I).

## Repository layout

| File | What it is |
|---|---|
| `BlindsESP32_apr16a.ino` | Main sketch (the name comes from Arduino Cloud, which does not allow renaming the main file): pin mapping, motion logic, cloud callbacks. |
| `thingProperties.h` | Auto-generated by Arduino Cloud: declares the cloud variables and connects them. Contains a placeholder device ID. |
| `arduino_secrets.h.example` | Template for WiFi / device credentials. Copy to `arduino_secrets.h` (git-ignored). |
| `motor_driver.h/.cpp` | Small driver for one TB6612FNG channel: direction pins, an on/off enable pin, standby. Full speed only (no PWM). |
| `sketch.json` | Arduino Cloud sketch metadata (board FQBN, secret names). Values are empty. |

## Hardware

| Part | Notes |
|---|---|
| ESP32 dev board | Built for "ESP32 Dev Module" (`esp32:esp32:esp32`), 4 MB flash. Any ESP32 supported by Arduino Cloud should work. |
| TB6612FNG dual motor driver | SparkFun TB6612FNG breakout board (ROB-14450). |
| 2 × DC gear motors | [12 V DC gearmotor, 100 RPM, eccentric D-shaft](https://www.amazon.com/dp/B0721T1PXQ) – one on each side of the blind, driving a bead-chain wheel. 1:47.5 reduction, rated 3.6 kg·cm and 0.92 A, 6×14 mm D-shaft, 37 mm gearbox, M3 mounting holes. Stall current not published. |
| 2 × bead-chain drive wheels | 30.6 mm diameter, 16 balls per revolution. |
| Motor power supply | 12 V DC wall adapter, 1.7 A (feeds TB6612 `VM`). |
| 5 V buck converter module | Steps the 12 V adapter down to 5 V for the ESP32. Fixed 5 V output, 3 A, 12-24 V input (Weewooday 10-pack, [Amazon](https://www.amazon.com/dp/B08JZ5FVLC)); any 12 V to 5 V buck module rated for at least 500 mA will do (see below). |
| 2 × magnetic reed switches | Normally-closed door/window-alarm style reed switch, one per motor, read on GPIO12 / GPIO14 (4-pack, [Amazon](https://www.amazon.com/dp/B07VQ1DY6T)). Closed when the magnet is near, open when it moves away. |
| 2 × 10 kΩ resistors | Pull-downs on the two reed switch inputs (value assumed). |
| Status LED (+ resistor) | Optional, on GPIO19; blinks every loop iteration as a liveness indicator. |

### Wiring

| ESP32 GPIO | TB6612FNG pin | Function |
|---|---|---|
| 16 | AIN1 | Motor 1 direction |
| 17 | AIN2 | Motor 1 direction |
| 5 | PWMA | Motor 1 speed |
| 4 | BIN1 | Motor 2 direction |
| 0 | BIN2 | Motor 2 direction |
| 2 | PWMB | Motor 2 speed |
| 18 | STBY | Standby / enable |
| 19 | – | Status LED (to GND via resistor) |
| 14 | magnet sensor, motor 1 (`MAGR`) | Digital input, 10 kΩ pull-down to GND |
| 12 | magnet sensor, motor 2 (`MAGL`) | Digital input, 10 kΩ pull-down to GND |
| 13 | – | Always-on 3.3 V supply for the motor 1 reed switch (set HIGH once at boot) |
| 27 | – | Always-on 3.3 V supply for the motor 2 reed switch (set HIGH once at boot) |
| 3V3 | VCC | Logic supply |
| GND | GND | Common ground with motor supply |
| – | VM | Motor supply (+) |
| – | A01/A02, B01/B02 | Motor 1 and motor 2 |

> GPIO0, GPIO2 and GPIO12 are ESP32 strapping pins. The sensor inputs use no internal pull-up/pull-down (`INPUT` only). A bare reed switch only connects the input to the supply (GPIO13 / GPIO27) when closed; when open the pin is not driven, so each input has an external 10 kΩ pull-down resistor to GND (value assumed from the original build; anything around 10 kΩ works) to read a clean LOW. A level on GPIO12 at boot can affect flash voltage selection on some boards; if the board will not boot with the sensor connected, move that input. They work here, but a connected device pulling a strapping pin to the wrong level at boot can stop flashing or booting. If you build this from scratch, consider other pins (and update the `#define`s). They work here, but a driver input pulling them to the wrong level at boot can prevent flashing or booting. If you build this from scratch, consider moving them to other pins (and updating the `#define`s).

Wiring diagram (derived from the pin definitions in the sketch):

```
                      12 V DC wall adapter
                         (+)         (-)
                          |           |
                          |           +-----------------------+
                          |                                   |
   +----------------------+-----+      +----------------------+---+
   |  SparkFun TB6612FNG        |      |        ESP32 Dev Module  |
   |  breakout (ROB-14450)      |      |                          |
   |                            |      |                          |
   |  VM   <--------------------+      |                          |
   |  VCC  <------------------------------ 3V3                    |
   |  GND  <------------------------+---- GND  (common ground)    |
   |                            |      |                          |
   |  AIN1 <------------------------------ GPIO16                 |
   |  AIN2 <------------------------------ GPIO17                 |
   |  PWMA <------------------------------ GPIO5                  |
   |  BIN1 <------------------------------ GPIO4                  |
   |  BIN2 <------------------------------ GPIO0                  |
   |  PWMB <------------------------------ GPIO2                  |
   |  STBY <------------------------------ GPIO18                 |
   |                            |      |                          |
   |  A01 / A02 ---> Motor 1    |      |  GPIO19 --[R]--|>|-- GND |
   |  B01 / B02 ---> Motor 2    |      |           (status LED)   |
   +----------------------------+      +--------------------------+
```

Notes:

- **Common ground:** the wall adapter's negative, the TB6612 `GND` and the ESP32 `GND` must all be connected together.
- **ESP32 current budget:** an ESP32 dev board typically draws about 80-100 mA while connected to WiFi, with short transmit peaks of roughly 250-500 mA. The TB6612 logic and the status LED add only a few mA. Espressif recommends a supply that can deliver at least 500 mA, so a 3 A module has plenty of margin. The motors run from the 12 V adapter through `VM`, not through the buck converter.
- **Powering the ESP32:** the 12 V adapter must not go straight to the ESP32. Feed the adapter into a 12 V to 5 V buck converter module and connect its 5 V output to the ESP32's `5V`/`VIN` pin (grounds common). Verify the output reads 5 V with a multimeter before connecting the ESP32.
- **Motor polarity:** if a motor runs the wrong way, swap its two wires or flip `offsetA` / `offsetB` in the sketch.
- Consider a bulk capacitor (e.g. 100 µF or more) across `VM` and `GND` near the driver to absorb motor switching spikes.

## Setup and deployment

You need an [Arduino Cloud](https://cloud.arduino.cc) account (the free tier is enough for one Thing).

### How Arduino Cloud fits together

Arduino Cloud has four pieces that you set up in order. The menu names below are from the current web interface and may change.

| Piece | What it is | In this project |
|---|---|---|
| **Device** | The physical board registered with the Cloud. Gives you a Device ID and a Secret Key. | One ESP32, created as a *third-party device*. |
| **Thing** | The device plus its cloud variables and WiFi settings. Generates `thingProperties.h` and the sketch skeleton. | One Thing with the five variables listed above. |
| **Sketch** | The code, edited, compiled and uploaded in the Cloud's web editor. Belongs to the Thing. | `BlindsESP32_apr16a.ino` plus `motor_driver.*`. |
| **Dashboard** | The web/phone control panel with widgets linked to the variables. | Switch, buttons and a value for the blinds. |

**Board selection:** this project is built for a generic ESP32. When you create the device, and again whenever the editor asks you to choose a board for compiling and uploading, pick **ESP32 Dev Module** (`esp32:esp32:esp32`). The original build used these board settings (from `sketch.json`); if the editor or your IDE exposes them, use the same values:

| Setting | Value |
|---|---|
| CPU frequency | 240 MHz |
| Flash size | 4 MB |
| Flash mode / frequency | QIO / 80 MHz |
| Partition scheme | Default |
| PSRAM | Disabled |
| Upload speed | 921600 |
| Core debug level | None |

### 1. Create the device

1. In Arduino Cloud go to **Devices → Add → Third party device → ESP32 → ESP32 Dev Module**.
2. Connect the ESP32 to your computer with a data-capable USB cable when prompted and select its serial port. This needs the **Arduino Cloud Agent** (the browser will offer to install it) and a Chrome-based browser.
3. Name the device (for example "Blinds"). When it finishes, **save the Device ID and Secret Key** – the secret key is shown only once and goes into the Secret tab later.

### 2. Create the Thing

1. **Things → Create**. Under *Associated Device* select the device from step 1. Under *Network* choose WiFi and enter your 2.4 GHz WiFi name and password (the ESP32 does not support 5 GHz).
2. Add the variables from the table above with exactly these names and types:
   - `blinds` – type **Smart Home → Light (`CloudLight`)**, Read & Write, *Update on change*
   - `blindSwitch`, `down`, `up` – Boolean, Read & Write, *Update on change*
   - `uptime` – Integer, Read only, *Update on change*
3. Open the **Sketch** tab. Arduino Cloud generates `thingProperties.h` (with your own device ID) and the `.ino` skeleton.

### 3. Add the code

Easiest path (Arduino Cloud web editor):

1. In the Sketch tab, replace the `.ino` contents with `BlindsESP32_apr16a.ino` from this repo (keep the `#include "arduino_secrets.h"` first line).
2. Add `motor_driver.h` and `motor_driver.cpp` as new files in the sketch (*▾ → New tab / Upload*).
3. Leave the Cloud-generated `thingProperties.h` as is – **do not** paste this repo's placeholder over it.
4. Fill in the **Secret** tab: `SECRET_SSID`, `SECRET_OPTIONAL_PASS`, `SECRET_DEVICE_KEY`.
5. At the top of the editor, select your device or choose board **ESP32 Dev Module** and the serial port of the connected ESP32.
6. Click **Verify** (compile), then **Upload**. If the upload stalls at "Connecting...", hold the ESP32's **BOOT** button until it starts.
7. Open the editor's **Serial Monitor** at 115200 baud to watch the log.

Alternative (local Arduino IDE / arduino-cli):

1. Install the ESP32 board package (`esp32` by Espressif) and the **ArduinoIoTCloud** and **Arduino_ConnectionHandler** libraries (they pull in their dependencies).
2. Put the files in a folder named `BlindsESP32_apr16a` (the folder name must match the `.ino`, so the cloned repo folder must be renamed).
3. `cp arduino_secrets.h.example arduino_secrets.h` and fill it in.
4. Replace the placeholder device ID in `thingProperties.h` with your Device ID (or copy the generated `thingProperties.h` from the Cloud).
5. Select board *ESP32 Dev Module*, 240 MHz, 4 MB flash, default partition scheme, and upload.

### 4. Build the dashboard

**Dashboards → Create → Build dashboard**, then *Add* widgets and link each to a variable of your Thing:

| Widget | Linked variable | Notes |
|---|---|---|
| Switch (or Light) | `blinds` | On = open, Off = closed. |
| Switch | `blindSwitch` | Chooses which motor the manual buttons drive. |
| Push button | `up` | Hold to run the selected motor up. A push button sends on while pressed and off when released, which is what the sketch expects. |
| Push button | `down` | Hold to run the selected motor down. |
| Value or Chart | `uptime` | Shows the board is alive. |

Optionally enable the Alexa or Google Home integration from the Cloud to control `blinds` by voice. Open the dashboard on your phone with the Arduino Cloud app.

### 5. Commission and calibrate

1. Power up with the blinds in a known position and open the Serial Monitor at 115200 baud. You should see `***Booted***`, then cloud connection messages and `Uptime=` every ~10 s.
2. Use the manual `up` / `down` buttons (with `blindSwitch` selecting the motor) to confirm each motor turns the right way. If not, flip its `offsetA` / `offsetB`.
3. Check the sensors: with the serial monitor open, run a motor and confirm the `Motor ... ticks` count resets as the wheel turns. If it never resets, check sensor power, wiring and magnet position.
4. Time a full travel of the blinds and set `BLIND_UP_SECONDS` / `BLIND_DOWN_SECONDS` a little above it. Re-upload.
5. Test `blinds` on/off. Keep a hand near the power supply for the first runs – stall detection is the only thing that stops the blinds at the ends before the timeout, so if a sensor fails the motor will keep pulling until the timeout.

## Known limitations

- End of travel is found by stall detection (a safety timeout backs it up), but there is no absolute position measurement. The software open/closed state is lost on reboot.
- Stall detection relies on the magnet sensors and fixed thresholds (30 loops); there is no electrical current protection.
- Fixed full-speed moves; motors run sequentially by design (see below).
- The manual `up` / `down` buttons do not set the "moving" flag, so changing `blinds` while a manual button is held (or changing `blindSwitch` mid-jog) can run both motors at once. See [Why only one motor at a time](#why-only-one-motor-at-a-time); avoid doing this.
- `uptime` increments by 10 per ~10 s but drifts slightly (it is counted in loop iterations).

## Security note

WiFi credentials and the Arduino Cloud device secret key belong only in `arduino_secrets.h` (git-ignored) or the Cloud's Secret tab. If you ever committed real values in a fork, regenerate the device's secret key in Arduino Cloud.

## License and credits

- The motor driver (`motor_driver.*`) is original code; it was written after starting from SparkFun's MIT-licensed [TB6612FNG library](https://github.com/sparkfun/SparkFun_TB6612FNG_Arduino_Library), which is no longer included.
- This project is released under the [MIT License](LICENSE).
