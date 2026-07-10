# esphome-cst3530

An [ESPHome](https://esphome.io) external component for the **Hynitron CST3530** capacitive touch controller.

This is the touch chip on the **V2** revision of the Waveshare **ESP32‑S3‑Touch‑LCD‑2.8** board
(V1 used the CST328; V2 ships an ST7789 display + CST3530 touch and answers on I²C address **`0x58`**).
No existing ESPHome touch component supports it — the CST328 drivers use 16‑bit register
addressing and the wrong data registers, so they return scrambled coordinates. This driver ports
Waveshare's own CST3530 read routine: 32‑bit register addressing, the `0xD0070000` data block, the
`0xD00002AB` end‑of‑read acknowledge, and the correct 12‑bit nibble decode.

## Usage

```yaml
external_components:
  - source: github://itconor/esphome-cst3530
    components: [cst3530]

i2c:
  sda: GPIO1
  scl: GPIO3

touchscreen:
  - platform: cst3530
    id: my_touchscreen
    display: my_display
    address: 0x58
    interrupt_pin: GPIO4
    reset_pin: GPIO2
```

The CST3530 reports coordinates **already scaled to the panel resolution**
(≈0–240 × 0–320 on the 2.8" board), so no `calibration:` block is needed — the
driver maps them 1:1 to the display. Add one only if your panel is mirrored or
rotated (`calibration:` / `transform:` / `swap_xy` from the base touchscreen schema).

### Options

| Option | Default | Notes |
|---|---|---|
| `address` | `0x58` | CST3530 I²C address |
| `interrupt_pin` | – | falling-edge touch interrupt (GPIO4 on the Waveshare 2.8) |
| `reset_pin` | – | active-low reset (GPIO2 on the Waveshare 2.8) |
| `calibration` | native display size | raw→screen mapping; also supports `swap_xy` / `transform` from the base touchscreen schema |

## Credit

Register map and decode ported from Waveshare's ESP‑IDF `CST3530.c` demo driver
(Apache‑2.0, © Espressif / Waveshare).

## Licence

MIT
