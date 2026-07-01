# tuya_daemon

Tuya integration stack for RUTOS/OpenWrt in this SDK tree. The project bridges Tuya IoT Cloud messages to local `ubus` methods, then forwards hardware requests to an ESP-based module over serial.

## Components

- `tuya_daemon_program/`
  - Builds package `tuya_daemon`.
  - Binary: `/usr/bin/tuya_daemon`.
  - Reads Tuya credentials from UCI config and connects to Tuya MQTT.
  - Translates cloud `actionCode` messages into `ubus` method calls on object `esp_module`.
- `esp_module_program/`
  - Builds package `esp_module`.
  - Binary: `/usr/bin/esp_module`.
  - Exposes `ubus` object `esp_module` with methods `on`, `off`, `get`, and `devices`.
  - Sends JSON requests to serial devices and returns responses over `ubus`.
- `libtuya_link/`
  - Builds package `libtuya_link` from `tuya-iot-core-sdk`.
  - Provides Tuya MQTT/link libraries used by `tuya_daemon`.
- `libserialport/`
  - Builds package `libserialport` from upstream `sigrok/libserialport`.
  - Used by `esp_module` for USB/serial communication.
- `vuci_tuya_daemon_api/`
  - VuCI API and ACL integration for managing `tuya_daemon` config.

## Runtime Flow

```mermaid
flowchart LR
    Cloud[Tuya Cloud] -->|MQTT actionCode| TD[tuya_daemon]
    TD -->|ubus invoke| UBUS[ubus object: esp_module]
    UBUS -->|serial JSON| ESP[ESP module over USB/serial]
    ESP -->|serial JSON reply| UBUS
    UBUS -->|ubus reply| TD
    TD -->|property report| Cloud
```

## Build (from SDK root)

Build each package directly:

```sh
make package/tuya_daemon/libserialport/compile V=s
make package/tuya_daemon/libtuya_link/compile V=s
make package/tuya_daemon/esp_module_program/compile V=s
make package/tuya_daemon/tuya_daemon_program/compile V=s
make package/tuya_daemon/vuci_tuya_daemon_api/compile V=s
```

Optional clean + rebuild example:

```sh
make package/tuya_daemon/tuya_daemon_program/clean
make package/tuya_daemon/tuya_daemon_program/compile V=s
```

## Installed Files on Target

- `/usr/bin/tuya_daemon`
- `/usr/bin/esp_module`
- `/etc/init.d/tuya_daemon`
- `/etc/init.d/esp_module`
- `/etc/config/tuya_daemon`
- VuCI API files under:
  - `/usr/lib/lua/api/services/config_tuya_daemon.lua`
  - `/usr/share/vuci/path.d/tuya_daemon.json`
  - `/usr/share/rpcd/acl.d/tuya_daemon.json`

## Configuration

UCI config file: `/etc/config/tuya_daemon`

Default structure:

```uci
config cert
    option enable '1'
    option product_id ''
    option device_id ''
    option device_secret ''
```

Notes:
- Init script reads sections of type `tuya_daemon` and passes values as:
  - `-p <product_id>`
  - `-d <device_id>`
  - `-s <device_secret>`
- Binary argument parser requires all three options (`device-id`, `device-secret`, `product-id`).

## Service Control

```sh
/etc/init.d/esp_module enable
/etc/init.d/esp_module start

/etc/init.d/tuya_daemon enable
/etc/init.d/tuya_daemon start

/etc/init.d/tuya_daemon restart
/etc/init.d/esp_module restart
```

Recommended startup order: start `esp_module` before `tuya_daemon`.

## ubus Interface (esp_module)

List object/methods:

```sh
ubus -v list esp_module
```

Method examples:

```sh
# Turn pin on
ubus call esp_module on '{"pin":13,"port":"/dev/ttyUSB0"}'

# Turn pin off
ubus call esp_module off '{"pin":13,"port":"/dev/ttyUSB0"}'

# Read sensor/data
ubus call esp_module get '{"pin":13,"port":"/dev/ttyUSB0","model":"dht11","sensor":"temperature"}'

# List detected serial devices
ubus call esp_module devices '{}'
```

## Tuya Action Mapping

`tuya_daemon` currently maps Tuya `actionCode` values to local methods:

- `action_get_sensor_data` -> `ubus call esp_module get ...`
- `action_get_devices` -> `ubus call esp_module devices ...`
- `action_toggle` -> toggles between `on` and `off` on each call (state kept in process memory)

Expected incoming payload shape:

```json
{
  "actionCode": "action_get_sensor_data",
  "inputParams": {
    "pin": 13,
    "port": "/dev/ttyUSB0",
    "model": "dht11",
    "sensor": "temperature"
  }
}
```

## Logs and Debugging

View service logs:

```sh
logread -e tuya_daemon
logread -e esp_module
```

Useful checks:

```sh
# Verify ubus object exists
ubus list | grep esp_module

# Verify config values
uci show tuya_daemon

# Verify binary starts manually
/usr/bin/tuya_daemon -p <PRODUCT_ID> -d <DEVICE_ID> -s <DEVICE_SECRET>
```

## Known Behavior Notes

- `action_toggle` does not track per-pin state; it flips a single daemon-level toggle variable.
- If serial reply is empty, handlers return `{}` fallback in some paths.
- MQTT host/port are currently hardcoded in source (`m1.tuyacn.com:8883`).
