# MIDIFAM Bootloader and Firmware

In order to allow software updates over MIDI so that users do not need an AVR programmer,
the MCU runs a bootloader program.

## Build Process

You must build both the bootloader and the MIDIFAM firmware.

### Bootloader

The following commands are issued from inside the ``software/`` directory.

**Produce a ``.hex`` file for the bootloader:**

```bash
make -f bootloader/makefile 
```

**Burn the fuses to allow Read-While-Write and bootloader support:**

```bash
make -f bootloader/makefile fuses
```

**Flash the bootloader ``.hex`` to the MCU:**

```bash
make -f bootloader/makefile upload
```

### Firmware

The firmware can be flashed directly to the MCU, but it is better to have the bootloader in place
so that software updates do not require an AVR programmer.

The following commands are issued from inside the ``software/`` directory.

**Compile the firmware producing a ``.hex`` file and a ``.mid`` file:**

```bash
make -f firmware/makefile
```

Once you have the ``.mid`` file you can play it into the MIDI input to write the firmware to the MCU.
In Linux, something like:

```bash
aplaymidi -p 24:0 build/midifam/midifam.mid
```