# ControllerEmulator
An Xinput (Xbox 360) Controller emulator for linux. (Keyboard only)

### Build

```
make
```

It will be installed in $HOME/.local/bin

### Usage

```
sudo xipade -k "DEVICE" [-v]
```

"DEVICE" -> The path to the keyboard (should look something like /dev/input/eventX)
            run "sudo evtest" to get your device path.

   -v    -> To show more output. Useful for debugging.

   -l    -> Alternate buttons layout.

### Default Layout (US Keyboard Layout)

| Button       | Key                             | Alternate Key |
|:------------:|:-------------------------------:|:-------------:|
| A            | K                               |               |
| B            | L                               | LALT          |
| X            | J                               |               |
| Y            | I                               |               |
| BACK         | X                               | B             |
| START        | N                               |               |
| LB           | Q                               | R             |
| RB           | E                               | Y,LSHIFT      |
| LT           | Z                               | SPACE,4       |
| RT           | C                               | 6             |
| LS Click     | TAB                             | 5             |
| RS Click     | V                               |               |
| DPAD_UP      | T                               |               |
| DPAD_DOWN    | G                               | .             |
| DPAD_LEFT    | F                               |               |
| DPAD_RIGHT   | H                               |               |
| LSTICK_UP    | W                               |               |
| LSTICK_DOWN  | S                               |               |
| LSTICK_LEFT  | A                               |               |
| LSTICK_RIGHT | D                               |               |
| RSTICK_UP    | 1                               | 8             |
| RSTICK_DOWN  | 2                               | 9             |
| RSTICK_LEFT  | U                               |               |
| RSTICK_RIGHT | O                               |               |
| GUIDE        | ENTER                           |               |

### Alternate Layout (US Keyboard Layout)

| Button       | Key                             | Alternate Key |
|:------------:|:-------------------------------:|:-------------:|
| A            | K                               |               |
| B            | L                               |               |
| X            | J                               |               |
| Y            | I                               | LALT          |
| BACK         | X                               |               |
| START        | N                               |               |
| LB           | Q                               | R             |
| RB           | E                               | Y             |
| LT           | LSHIFT                          | 4             |
| RT           | SPACE                           | 6             |
| LS Click     | C                               |               |
| RS Click     | Z                               |               |
| DPAD_UP      | T                               | TAB           |
| DPAD_DOWN    | G                               | .             |
| DPAD_LEFT    | F                               |               |
| DPAD_RIGHT   | H                               |               |
| LSTICK_UP    | W                               |               |
| LSTICK_DOWN  | S                               |               |
| LSTICK_LEFT  | A                               |               |
| LSTICK_RIGHT | D                               |               |
| RSTICK_UP    | 1                               | 8             |
| RSTICK_DOWN  | 2                               | 9             |
| RSTICK_LEFT  | U                               |               |
| RSTICK_RIGHT | O                               |               |
| GUIDE        | ENTER                           |               |

### Control Keys

| KEY          | USAGE                           | ALTERNATE     |
|:------------:|:-------------------------------:|:-------------:|
| F2           | Toggle Pause Controller to Type |               |
| F12          | Close Program                   | Can be closed by clicking the tray icon |

### Fix Wrong Inputs
```
export SDL_GAMECONTROLLERCONFIG="030000005e0400008e02000010010000,Xbox 360 Controller,platform:Linux,a:b0,b:b1,x:b2,y:b3,back:b8,guide:b10,start:b9,leftstick:b11,rightstick:b12,leftshoulder:b4,rightshoulder:b5,dpup:b13,dpdown:b14,dpleft:b15,dpright:b16,leftx:a0,lefty:a1,rightx:a2,righty:a3,lefttrigger:b6,righttrigger:b7,"
```

### Run without Root (Linux Mint)

```
sudo usermod -a -G input $USER
sudo groupadd -f uinput
sudo gpasswd -a $USER uinput
```

In /etc/udev/rules.d/ create a new rule file (99-uinput.rules) and within put this line:
```
KERNEL=="uinput", GROUP="uinput", MODE="0660"
```

Save and Reboot.

### Credits

Thanks to the original project:

https://github.com/niehoff90/ControllerEmulator

And this project for the analog sticks code:

https://github.com/duzda/TitanSoulsLinuxEmulator
