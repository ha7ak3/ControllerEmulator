# ControllerEmulator
An xbox controller emulator for linux.

### Build

gcc emulator.c -o emulator

### Usage

sudo ./emulator --keyboard="PATH" --mouse="PATH"

"PATH" -> The path to the keyboard and mouse (should look something like /dev/input/eventX)

##### if an application cannot recognize the controller:

sudo ./mimic-xpad.sh "PATH"

"PATH" -> The path to the emulated controller (should look something like /dev/input/eventX)

This may be desirable for games that support Xbox 360 controllers out of the box, but have trouble detecting or working with other gamepads.
click here for details: https://wiki.archlinux.org/title/Gamepad#Mimic_Xbox_360_controller
