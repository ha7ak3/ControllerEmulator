# ControllerEmulator
An xbox controller emulator for linux.

### Build ControllerEmulator

gcc emulator.c -o emulator

### ControllerEmulator Usage

sudo ./emulator --keyboard="PATH" --mouse="PATH"

"PATH" -> The path to the keyboard and mouse (should look something like /dev/input/eventX)
