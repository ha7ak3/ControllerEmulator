# ControllerEmulator
An Xbox One Controller emulator for linux. Mouse not supported yet.

### Build

gcc profiles.c -o emulator

### Usage

sudo ./emulator -k "DEVICE" [-v]

"DEVICE" -> The path to the keyboard (should look something like /dev/input/eventX)
            run "sudo evtest" to get your device path.

   -v    -> To show more output. Useful for debugging.
