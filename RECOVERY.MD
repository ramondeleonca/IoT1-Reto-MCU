# ESP32 Recovery Log

**Last updated:** 2026-10-15  
**Status:** Blink firmware builds, but has **not** been uploaded. The board's serial bootloader has not responded reliably.

## Goal and current firmware

The board is reported to be running older firmware that drives a servo signal on
GPIO15. The intended recovery firmware is the project's blink sketch.

The active entry point, `src/main.cpp`, includes `src/impl/blink.hpp`. That
sketch sets `LED_BUILTIN` as an output and alternates it HIGH/LOW every 500 ms.
It compiled successfully for the configured DOIT ESP32 DevKit V1 target.

The build succeeding does not establish that the board was flashed. None of the
upload attempts below completed, and no flash erase was performed. Assume the
old servo firmware is still on the board.

## Board and host details

- PlatformIO environment: `esp32`
- Board: `esp32doit-devkit-v1` (DOIT ESP32 DevKit V1)
- Framework: Arduino
- PlatformIO Core: 6.2.0
- Espressif platform: 55.3.39
- Detected hardware: ESP32, 4 MB flash
- USB-to-serial bridge: Silicon Labs CP2102, VID:PID `10C4:EA60`, serial `0001`
- Initial serial port: `/dev/cu.usbserial-0001` (also available as
  `/dev/tty.usbserial-0001`)
- After changing cable/USB port, the same CP2102 serial number was detected at
  a different USB location. The serial device name remained
  `/dev/cu.usbserial-0001`.
- PlatformIO is installed at
  `$HOME/.platformio/penv/bin/pio`; esptool is at
  `$HOME/.platformio/penv/bin/esptool`.

## Build result

Command:

```sh
$HOME/.platformio/penv/bin/pio run -e esp32
```

Result: **success**. The generated firmware is:

```text
.pio/build/esp32/firmware.bin
```

Its observed file size was 320,400 bytes. PlatformIO reported the image using
about 24.4% of the configured 1,310,720-byte application space and 23,548 bytes
of RAM.

## Upload and connection attempts

### PlatformIO upload using the original cable

Command:

```sh
$HOME/.platformio/penv/bin/pio run -e esp32 -t upload \
  --upload-port /dev/cu.usbserial-0001
```

The upload started with esptool at 460800 baud, but failed while connecting:

```text
Failed to connect to ESP32: Invalid head of packet (0xB7):
Possible serial noise or corruption.
```

### Manual BOOT-button attempts

The servo signal wire was disconnected from GPIO15. BOOT was held during an
upload retry; that attempt failed with invalid packet `0xC1`. A subsequent
verbose PlatformIO attempt reported:

```text
Failed to connect to ESP32: Wrong boot mode detected (0x3)!
The chip needs to be in download mode.
```

The servo was then fully disconnected (signal, power, and ground), leaving USB
as the only connection. BOOT/IO0 was held while EN/RESET was pressed and
released. Repeated direct esptool `chip-id` probes at 115200 baud, with
`--before no-reset --after no-reset`, still failed with invalid packet data
(`0xC1` or `0xB7`).

### Cold USB reconnect and serial monitor checks

The board was unplugged and reconnected while BOOT was held. The CP2102
reappeared, but a no-reset `chip-id` probe again failed with invalid packet
`0xB7`. A direct-reset `chip-id` probe failed with invalid packet `0xC1`.

PlatformIO's device monitor could not be used because it requires an
interactive terminal. A direct read from the serial device returned only one
replacement/invalid character; it did not produce a readable ROM boot message.

An esptool probe was also started before manually holding BOOT and tapping
EN/RESET, to synchronize during the connection window. It failed with invalid
packet `0xC1`.

### Cable and USB-port substitution

A different known-good USB data cable and a different computer USB port were
tried, with the servo still disconnected. The same CP2102 serial number
reappeared. A direct-reset probe reported no serial data; a no-reset probe
during a manual BOOT/EN reset again failed with invalid packet `0xC1`.

### GPIO0 strap and macOS serial endpoints

As a check for a faulty BOOT button, GPIO0/IO0 was connected directly to GND,
then EN/RESET was pressed and released. With the jumper in place, `chip-id`
probes at 115200 baud failed on both macOS serial paths:

| Port | Result |
| --- | --- |
| `/dev/cu.usbserial-0001` | Invalid head of packet `0x74` |
| `/dev/tty.usbserial-0001` | Invalid head of packet `0x30` |

The GPIO0-to-GND jumper was **not confirmed removed** at the end of this
session. Remove it before powering or using the board again. Keep the servo
disconnected until the blink firmware has been successfully flashed and
verified.

## Current assessment

The blink firmware and PlatformIO build are working. The failure is at the
hardware serial/bootloader connection stage. The symptoms persisted with the
servo fully disconnected, a different cable and USB port, manual BOOT/EN
sequences, and GPIO0 forcibly strapped low.

The onboard CP2102 appears on USB, but that only confirms USB enumeration of
the bridge; it does not prove that UART data reaches the ESP32. The remaining
likely causes include a faulty onboard CP2102-to-ESP32 UART path, damaged
ESP32 UART/boot circuitry, or a board/connection issue. An external 3.3 V
USB-to-UART adapter is the next useful isolation test. The user has one
available but not at this time.

## Next recovery steps

When an external adapter is available:

1. Remove the GPIO0-to-GND jumper from the previous test. Leave the servo and
   all other peripherals disconnected.
2. Connect the external adapter to the ESP32 UART0 pins:
   - Adapter **TXD** to ESP32 **U0R/RX0 (GPIO3)**.
   - Adapter **RXD** to ESP32 **U0T/TX0 (GPIO1)**.
   - Adapter **GND** to ESP32 **GND**.
3. Use **3.3 V UART logic levels**. Do not connect the adapter's VCC/5 V pin
   while the ESP32 is powered over its USB connector; avoid powering the board
   from two sources at once.
4. Determine the external adapter's serial port with:

   ```sh
   $HOME/.platformio/penv/bin/pio device list
   ```

5. Force download mode: connect GPIO0/IO0 to GND, press and release EN/RESET,
   and keep GPIO0 low for the bootloader connection.
6. First test the connection without writing flash (replace `<PORT>`):

   ```sh
   $HOME/.platformio/penv/bin/esptool \
     --chip esp32 --port <PORT> --baud 115200 \
     --before no-reset --after no-reset chip-id
   ```

7. If `chip-id` succeeds, flash the already-built blink firmware using the
   matching artifacts and the same port:

   ```sh
   $HOME/.platformio/penv/bin/esptool \
     --chip esp32 --port <PORT> --baud 115200 \
     --before no-reset --after hard-reset write-flash -z \
     --flash-mode dio --flash-freq 40m --flash-size detect \
     0x1000 .pio/build/esp32/bootloader.bin \
     0x8000 .pio/build/esp32/partitions.bin \
     0xe000 "$HOME/.platformio/packages/framework-arduinoespressif32/tools/partitions/boot_app0.bin" \
     0x10000 .pio/build/esp32/firmware.bin
   ```

8. After esptool reports a successful write and verification, remove the
   GPIO0-to-GND jumper and reset the board. Confirm the built-in LED blinks,
   then reconnect the servo only if needed.

If the external adapter also cannot read `chip-id`, stop repeated flash
attempts and inspect/test the ESP32 board itself (including UART0, EN, GPIO0,
power, and ground) or replace it. Do not erase flash unless there is a separate
reason to do so; an erase is not needed to replace the running application.
