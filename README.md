# ESP32 Thermostat – Smart Heating System

Complete project (firmware + web UI) for controlling heating with an ESP32, a relay and ATC-Mi Bluetooth thermometers.

## Main features
* Operating modes: **AUTO**, **MANUAL**, **OFF**
* Weekly schedule in 30 min slots (48 slots/day) with **HOME**, **AWAY**, **NIGHT**, **ANTIFREEZE**
* “Smart” directives that pre-heat based on average heating rate
* Manual relay control
* OTA updates, NTP sync and fallback AP mode
* Persistence on LittleFS (rooms, schedule, history, heating mode)
* Responsive UI (HTML/CSS/JS) – SweetAlert2 & flatpickr

## Hardware requirements
* ESP32-S3 (16 MB flash, PSRAM)  
* **or generic ESP32-WROOM / DevKitC** (4-8 MB flash)
* Relay on pin 26 (HIGH = OFF, LOW = ON)
* ATC-Mi thermometers with custom firmware (NimBLE)
* 5 V power supply for the relay

## Installation
1. Clone the repo and open it with PlatformIO.  
2. **Copy _or rename_ `src/wifi_credentials_example.h` to `src/wifi_credentials.h`, then add your
   real SSID / password (this file is git-ignored).**  
3. Select the appropriate environment (`esp32-s3` or `esp32`) in PlatformIO.  
4. Connect the relay to pin 26.  
5. Upload firmware (`pio run -e <env> -t upload`) then upload UI (`pio run -e <env> -t uploadfs`).  
6. At first boot the device tries Wi-Fi; if it fails it starts an AP named **Thermostat**.

## Web interface
Open `http://<esp_ip>/` to:
* Toggle Light/Dark theme (☀️/🌙)
* Add rooms and thermometers
* View or edit the weekly schedule
* Trigger “Home now / Leave now” or schedule at a specific time
* In MANUAL mode you can turn heating ON/OFF directly

## Local REST API (main endpoints)

| Method | Endpoint            | Description                    |
|--------|---------------------|--------------------------------|
| GET    | `/rooms`            | List rooms                     |
| POST   | `/rooms`            | Create room                    |
| PUT    | `/rooms`            | Update room                    |
| DELETE | `/rooms`            | Delete room                    |
| GET/PUT| `/heating/mode`     | Get / set heating mode         |
| GET/PUT| `/heating/manual`   | Get / set manual mode          |
| GET    | `/heating`          | Relay state (`isHeating`)      |
| GET/PUT| `/schedule`         | Get / modify a schedule slot   |

Responses are JSON; errors return proper 4xx/5xx codes.

## Build & Flash
```bash
# For ESP32-S3
pio run -e esp32-s3                 # compile
pio run -e esp32-s3 -t upload       # flash firmware
pio run -e esp32-s3 -t uploadfs     # upload LittleFS (UI)

# For generic ESP32
pio run -e esp32                    # compile
pio run -e esp32 -t upload          # flash firmware
pio run -e esp32 -t uploadfs        # upload LittleFS (UI)

pio device monitor                  # serial monitor
```

## OTA update
Enabled by default; use PlatformIO “Upload OTA” or any `arduinoOTA` client.

## Contributing
Pull requests are welcome. Keep the existing coding style and open issues for bugs/ideas.

## License
MIT License

## Configuration tips
Before your first build you may tweak a few constants in `src/WiFiConnection.h / .cpp` and `src/globalSettings.*`:

| File | Constant | Default | Meaning |
|------|----------|---------|---------|
| **wifi_credentials.h** | `WIFI_SSID` / `WIFI_PASSWORD` | "" | Home Wi-Fi credentials |
| WiFiConnection.h | `GMT_OFFSET_SEC` / `DAYLIGHT_OFFSET_SEC` | +7200 / +3600 | Time-zone for NTP |
| globalSettings.cpp | `heatingMode` | `AUTO` | Power-on default mode |
| HeatingControl.cpp | `RELAY_PIN` | 26 | GPIO used for the relay |

Re-flash after changing any of the above.

## Quick wiring diagram
1. ESP32-S3 / ESP32-DevKitC  
2. 5 V relay module – IN ➜ GPIO 26, Vcc ➜ 5 V, GND ➜ GND  
3. ATC-Mi sensors broadcast over BLE – no wiring required  
4. Optional 3.3 V-to-5 V level shifter if your relay is 5 V logic only.

## Example API usage

```bash
# List rooms
curl http://<esp_ip>/rooms

# Create a new room
curl -X POST http://<esp_ip>/rooms \
     -H "Content-Type: application/json" \
     -d '{"room_name":"Living"}'

# Set schedule slot: Monday (day 1) 07:00-07:30 to HOME
curl -X PUT http://<esp_ip>/schedule \
     -H "Content-Type: application/json" \
     -d '{"day":1,"hour":14,"mode":"HOME"}'
```

All endpoints are HTTP (not HTTPS) inside the LAN; use a firewall/NAT if you need remote access.

## Troubleshooting
| Symptom | Possible cause / fix |
|---------|----------------------|
| Device boots but no Wi-Fi | Wrong credentials, ensure 2.4 GHz only, check `platformio.ini` env |
| Relay never turns on | Inverted logic: adjust `digitalWrite(RELAY_PIN, LOW/HIGH)` in `HeatingControl.cpp` |
| BLE sensors not discovered | Flash ATC-Mi custom firmware, keep them in advertising mode |
| Web UI not loading | Make sure you ran `pio run -e <env> -t uploadfs` after changing files in `data/` |

Have fun building your DIY smart thermostat! Contributions and feedback are always welcome.
