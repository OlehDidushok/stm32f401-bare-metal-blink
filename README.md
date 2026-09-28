# STM32F401 bare-metal: LED blink, push-pull vs open-drain

Learning exercise: register-level GPIO on an STM32F401 board, no HAL, no CubeMX-generated code.
The goal is to read the reference manual and write the bit manipulation by hand.

## Hardware

| Item | Details |
|---|---|
| Board | "STM32 Mini F401" (Black Pill form factor), STM32F401CCU6, Cortex-M4, 84 MHz max, 25 MHz HSE |
| User LED | On **PC13**, wired as a sink (LED lights when the pin is driven low) |
| Programmer | ST-Link V2 clone, firmware V2J37S7, connected over SWD |
| SWD wiring | SWCLK to PA14, SWDIO to PA13, GND to GND (SWIM pin left unconnected, it is for STM8) |
| Host | macOS, VS Code + PlatformIO (`framework = stm32cube`, used only for CMSIS headers) |

Board pinout and schematic: [stm32-base.org, STM32 Mini F401](https://stm32-base.org/boards/STM32F401CCU6-STM32-Mini-F401.html)

## What the code does

`src/main.c` blinks the on-board LED with a busy-wait delay and provides two init functions:

- `gpio_led_init_pushpull()`: `OTYPER = 0`, the pin drives both high and low.
- `gpio_led_init_opendrain()`: `OTYPER = 1`, the pin only pulls low; the internal pull-up (`PUPDR = 01`) provides the high level.

Registers touched: `RCC->AHB1ENR` (GPIOC clock), `GPIOC->MODER`, `OTYPER`, `OSPEEDR`, `PUPDR`, `BSRR`.
Every multi-bit field is written with the clear-then-set pattern: `reg &= ~(mask << pos); reg |= (value << pos);`.

Note: because the on-board LED is a sink, the visible blink looks the same in both modes.
The difference is electrical (pin level and drive), and can be observed with a logic analyzer or multimeter.

## Build and flash

```ini
[env:genericSTM32F401CC]
platform = ststm32
board = genericSTM32F401CC
framework = stm32cube
upload_protocol = custom
extra_scripts = pre:upload_override.py
```

```bash
pio run                      # build
pio run --target upload      # build + flash via ST-Link
```

### Known issue: default ST-Link upload fails

With `upload_protocol = stlink`, PlatformIO's bundled OpenOCD 0.12.0 (`tool-openocd 3.1200.7`)
failed with this ST-Link (V2J37S7):

```
Debug adapter doesn't support 'hla_swd' transport
```

Workaround: `upload_override.py` replaces the upload command with a direct OpenOCD call that uses
`interface/stlink.cfg`, `transport select swd` and `target/stm32f4x.cfg`.
Related discussion: [PlatformIO community thread](https://community.platformio.org/t/stlink-v3pwr-debug-adapter-doesnt-support-hla-swd-transport/46648)
(that thread is about an STLINK-V3PWR, so a different adapter, but the same `hla_swd` transport mismatch).

## Bring-up notes

- The first board was unreachable over SWD (`Unable to get core ID`, also in STM32CubeProgrammer) and was replaced.
- The replacement was checked with STM32CubeProgrammer **before soldering the headers**: connection succeeded and flash was readable.
- CubeProgrammer reported Device ID `0x433`, Rev Z. Per RM0368, `0x433` is STM32F401x**D/E**;
  an F401CCU6 is the xC variant and should report `0x423`. So the die does not match the package
  marking, which is common on these clone boards. The `256 KB` flash figure printed by `pio run`
  does not confirm the part either, it comes from the `genericSTM32F401CC` linker script rather
  than from reading the chip.

## Verification checklist

Checked against documentation, not by eye:

- [x] LED on PC13, sink mode (stm32-base.org schematic)
- [ ] `MODER`, `OTYPER`, `OSPEEDR`, `PUPDR`, `BSRR` bit fields (RM0368, GPIO chapter)
- [ ] `RCC_AHB1ENR` GPIOCEN bit position (RM0368, RCC chapter)
- [x] Device ID `0x433` vs the F401CC marking on the chip (RM0368 DBGMCU_IDCODE, datasheet DS9716):
      mismatch, `0x433` is xD/E while the marking says xC. See bring-up notes.

## References

- RM0368, STM32F401xB/C and xD/E reference manual
- DS9716, STM32F401xB/C datasheet
- [stm32-base.org board page and original schematic](https://stm32-base.org/boards/STM32F401CCU6-STM32-Mini-F401.html)
