# ControllerEmulator
An xbox controller emulator for linux.

### Build

gcc emulator.c -o emulator

### Usage

sudo ./emulator --keyboard="PATH" --mouse="PATH"

"PATH" -> The path to the keyboard and mouse (should look something like /dev/input/eventX)
