# BeagleBone Black Sensor and Control Project

This project integrates multiple peripherals with a BeagleBone Black (BBB) running Debian Bullseye IoT to perform sensing, control, and cloud logging. It uses plain C with no third-party libraries beyond those included in the Debian image.

## 📖 Overview

The project interfaces the following components:

- 🌡️ **DHT22 Sensor**: Measures temperature and humidity.
- 🖥️ **SSD1306 OLED (I2C)**: Displays live sensor data.
- 📡 **ThingSpeak**: Logs sensor data to the cloud.
- 💡 **LED (PWM via 2N2222)**: Controls an external LED with PWM.
- 🌬️ **Fan (PWM via 2N2222)**: Controls a DC fan with PWM and a flyback diode.
- 🌞 **LDR (ADC)**: Measures ambient light levels via a voltage divider.

The code is written in plain C, compiles directly on the BBB, and leverages sysfs for GPIO, PWM, and ADC interactions.

## 🖥️ Board & Kernel Info

Tested and verified on:

- **Kernel**: `5.10.168-ti-r68`
- **Image**: BeagleBoard.org Debian Bullseye IoT Image 2023-08-05 (`/etc/dogtag`)

## ✅ Pin Mapping
```bash

| Component | BBB Pin(s) | Mode | Sysfs/Device Path | Notes |
|-----------|------------|------|-------------------|-------|
| **DHT22** | P8_11 | GPIO | `/sys/class/gpio/gpio45/` | Uses gpiochip1 line 13 |
| **OLED** | P9_19 (SCL), P9_20 (SDA) | I2C2 | `/dev/i2c-2` | Address 0x3C |
| **LED** | P9_16 → 2N2222 base | PWM | `/sys/class/pwm/pwmchipX/pwm0/` | LED(+) to external VCC, LED(–) to transistor collector |
| **Fan** | P9_14 → 2N2222 base | PWM | `/sys/class/pwm/pwmchipY/pwm0/` | Fan(–) to collector, Fan(+) to external VCC, flyback diode required |
| **LDR** | P9_39 (AIN0), P9_32 (3.3V) | ADC | `/sys/bus/iio/devices/iio:device0/in_voltage0_raw` | Voltage divider with 10kΩ resistor |
| **UART** | P9_24 (TX), P9_26 (RX) | UART | `/dev/ttyS4` | Optional debugging |
```
### ⚠️ Verify Pinmux

Before running, confirm pin configurations using `config-pin`:

```bash
sudo config-pin -q P9.14  # pwm
sudo config-pin -q P9.16  # pwm
sudo config-pin -q P9.19  # i2c
sudo config-pin -q P9.20  # i2c
sudo config-pin -q P9.24  # uart
sudo config-pin -q P9.26  # uart
```

## 🪛 Wiring Details

### 🔹 DHT22 → BeagleBone Black
```bash
| DHT22 Pin | BBB Pin (P8/P9 Header) |
|-----------|------------------------|
| VCC (3.3V) | P9_3 (3.3V) |
| DATA | P8_11 (GPIO1_13, gpiochip1 line 13) |
| GND | P9_1 (GND) |
```

*Note*: Add a 4.7kΩ pull-up resistor between DATA and VCC.

### 🔹 SSD1306 OLED (I2C) → BeagleBone Black
```bash
| SSD1306 Pin | BBB Pin (P9 Header, I2C2) |
|-------------|---------------------------|
| VCC (3.3V) | P9_3 (3.3V) |
| GND | P9_1 (GND) |
| SDA | P9_20 (I2C2_SDA) |
| SCL | P9_19 (I2C2_SCL) |
```

### 🔹 LED (via 2N2222 + PWM P9_16)
- LED(+) → External VCC
- LED(–) → Collector of 2N2222
- P9_16 → Base of 2N2222 (through 1kΩ resistor)
- Emitter → GND (common ground with BBB)

### 🔹 Fan (via 2N2222 + PWM P9_14)
- Fan(+) → External VCC
- Fan(–) → Collector of 2N2222
- P9_14 → Base of 2N2222 (through 1kΩ resistor)
- Emitter → GND (common ground with BBB)
- Flyback diode (1N4007): Cathode → Fan(+), Anode → Fan(–)

### 🔹 LDR → ADC (P9_39 / AIN0)
- P9_32 (3.3V) → LDR → P9_39 (AIN0)
- 10kΩ resistor from LDR-P9_39 junction to GND

## ⚙️ Software Requirements

The following are preinstalled in Debian Bullseye IoT:

- `gcc`
- `make`
- I2C tools (`i2c-tools`)
- Sysfs access for GPIO, PWM, and ADC

Install missing packages if necessary:

```bash
sudo apt update
sudo apt install -y build-essential i2c-tools
```

## 🚀 Build & Run

1. **Compile**:
   ```bash
   gcc -o smart-env-monitor src/*.c -Iinclude -lgpiod -lcurl -lpthread
   ```

2. **Run**:
   ```bash
   ./smart-env-monitor
   ```

## ▶️ Expected Behavior

- **DHT22**: Reads temperature and humidity every ~2 seconds.
- **OLED**: Displays:
  ```
  Temp: XX.X °C
  Hum:  YY.Y %
  Light: ZZZ
  ```
- **ThingSpeak**: Pushes sensor data to the cloud (if API key is set).
- **LED**: Controlled via PWM with a brightness pattern.
- **Fan**: Runs under PWM duty cycle control.
- **LDR**: Adjusts based on light intensity (ADC value 0–4095).

## 📡 ThingSpeak Setup

1. Create a [ThingSpeak](https://thingspeak.com/) account.
2. Create a new channel with fields for Temperature, Humidity, and Light.
3. Copy your **Write API Key**.
4. Update the `API_KEY` in `main_merged.c`:
   ```c
   const char *API_KEY = "YOUR_API_KEY_HERE";
   ```
5. Rebuild and run the program.

## 🔧 Troubleshooting

- **DHT22 not reading**: Verify the 4.7kΩ pull-up resistor and correct GPIO line (`gpiochip1 line 13`).
- **OLED not displaying**: Run `i2cdetect -y -r 2` to confirm address `0x3C`.
- **PWM not working**: Ensure `config-pin` has enabled PWM on P9.14 and P9.16.
- **Fan always on**: Check transistor orientation and flyback diode placement.
- **LDR stuck values**: Verify voltage divider wiring, use only 3.3V.
- **ThingSpeak not updating**: Confirm internet connectivity and correct API key.

## 📜 License

This project is licensed under the [MIT License](LICENSE) – free to use, modify, and distribute.