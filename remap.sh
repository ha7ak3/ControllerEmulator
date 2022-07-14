#!/bin/bash
EVDEV="/dev/input/event22"

K00="BTN_EAST=B"
K01="BTN_SOUTH=A"
K02="BTN_NORTH=X"
K03="BTN_WEST=Y"

K04="BTN_TL=TL"
K05="BTN_TR=TR"
K06="BTN_TL2=LT"
K07="BTN_TR2=RT"

K08="BTN_SELECT=back"
K09="BTN_START=start"

K10="BTN_THUMBL=LB"
K11="BTN_THUMBR=RB"

K12="BTN_DPAD_UP=du"
K13="BTN_DPAD_DOWN=dd"
K14="BTN_DPAD_LEFT=dl"
K15="BTN_DPAD_RIGHT=dr"

# xboxdrv --evdev $EVDEV --evdev-absmap ABS_X=X1,ABS_Y=Y1,ABS_RX=X2,ABS_RY=Y2,ABS_HAT0X=dpad_x,ABS_HAT0Y=dpad_y --axismap -Y1=Y1,-Y2=Y2 --evdev-keymap BTN_SOUTH=a,BTN_EAST=b,BTN_NORTH=x,BTN_WEST=y,BTN_THUMBL=lt,BTN_THUMBR=rt --mimic-xpad

#,ABS_HAT0X=dpad_x,ABS_HAT0Y=dpad_y \
xboxdrv --evdev $EVDEV \
  --evdev-absmap ABS_X=X1,ABS_Y=Y1,ABS_RX=X2,ABS_RY=Y2 \
  --axismap -Y1=Y1,-Y2=Y2 \
  --evdev-keymap $K00,$K01,$K02,$K03,$K04,$K05,$K06,$K07,$K08,$K09,$K10,$K11,$K12,$K13,$K14,$K15,$K16 \
  --mimic-xpad
