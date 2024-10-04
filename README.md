# ControllerEmulator
An Xbox 360 Controller emulator for linux. (Keyboard only)

### Build

```
make
```

It will be installed in $HOME/.local/bin

### Usage

sudo ./xipad -k "DEVICE" [-v]

"DEVICE" -> The path to the keyboard (should look something like /dev/input/eventX)
            run "sudo evtest" to get your device path.

   -v    -> To show more output. Useful for debugging.

   -l    -> Alternate buttons layout.
   <details>
       <summary>
       ```
       | Button       | Key                             | Alternate Key |
       |:------------:|:-------------------------------:|:-------------:|
       | A            | K                               |               |
       | B            | L                               |               |
       | X            | J                               |               |
       | Y            | I                               | LALT          |
       | BACK         | X                               |               |
       | START        | N                               |               |
       | LB           | Q                               | H             |
       | RB           | E                               |               |
       | LT           | LSHIFT                          |               |
       | RT           | SPACE                           |               |
       | LS Click     | C                               |               |
       | RS Click     | Z                               |               |
       | DPAD_UP      | R                               |               |
       | DPAD_DOWN    | B                               |               |
       | DPAD_LEFT    | F                               |               |
       | DPAD_RIGHT   | V                               |               |
       | LSTICK_UP    | W                               |               |
       | LSTICK_DOWN  | S                               |               |
       | LSTICK_LEFT  | A                               |               |
       | LSTICK_RIGHT | D                               |               |
       | RSTICK_UP    | 1                               | 8             |
       | RSTICK_DOWN  | 2                               | 9             |
       | RSTICK_LEFT  | U                               |               |
       | RSTICK_RIGHT | O                               |               |
       | GUIDE        | ENTER                           |               |
       ```
       </summary>
   </details>

###Default Buttons (US Keyboard Layout)

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

### Control Keys

| KEY          | USAGE                           | ALTERNATE     |
|:------------:|:-------------------------------:|:-------------:|
| F2           | Toggle Pause Controller to Type |               |
| F12          | Close Program                   | Can be closed by clicking the tray icon |

### Credits

Thanks to the original project:

https://github.com/niehoff90/ControllerEmulator

And this project for the analog sticks code:

https://github.com/duzda/TitanSoulsLinuxEmulator
