#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#include <argp.h>
#include <errno.h>
#include <fcntl.h>
#include <gtk/gtk.h>
#include <linux/input.h>
#include <linux/uinput.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "keys.h"
#define GAMEPAD_NAME "Virtual Xinput Gamepad"
#define KEYUK KEY_UNKNOWN
#define KEYLS KEY_LEFTSHIFT
#define KEYLA KEY_LEFTALT
#define KEYSP KEY_SPACE
#define KEYEN KEY_ENTER
#define KEYTB KEY_TAB
#define KEYDT KEY_DOT
int BA[3], BB[3], BX[3], BY[3], ST[3], BK[3], GD[3], LB[3], RB[3], LT[3], RT[3], TL[3], TR[3];
int DU[3], DD[3], DL[3], DR[3], LU[3], LD[3], LL[3], LR[3], RU[3], RD[3], RL[3], RR[3];
char* tooltip = GAMEPAD_NAME;
char* start_icon = "applications-games-symbolic";
char pathKeyboard[256] = "???";
char xaxis = 0;
char yaxis = 0;
char rxaxis = 0;
char ryaxis = 0;
bool verbose = false;
bool paused = false;
bool altlay = false;
int rt_down = 0;
int lt_down = 0;
int grab = 1;
GtkStatusIcon* icon;

static int parse_opt(int key, char* arg, struct argp_state* state) {
  switch (key) {
    case 'k':
      if (strlen(arg) < 256) strcpy(pathKeyboard, arg);
      break;
    case 'v':
      verbose = true;
      break;
    case 'l':
      altlay = true;
      break;
  }

  return 0;
}

int parse_arguments(int argc, char** argv) {
  struct argp_option options[] = {{"keyboard", 'k', "PATH", 0, "The path to keyboard (should look something like /dev/input/eventX)"},
                                  {"verbose", 'v', 0, OPTION_ARG_OPTIONAL, "Show more info"},
                                  {"altlay", 'l', 0, OPTION_ARG_OPTIONAL, "Use alternate buttons layout."},
                                  {0}};
  struct argp argp = {options, parse_opt};

  return argp_parse(&argp, argc, argv, 0, 0, 0);
}

void exitFunc(int keyboard_fd, int gamepad_fd) {
  printf("\nExiting.\n");
  ioctl(keyboard_fd, EVIOCGRAB, 0);  // Disable exclusive access
  close(gamepad_fd);                 // Close gamepad
  close(keyboard_fd);                // Close keyboard
  exit(EXIT_SUCCESS);
}

void send_sync_event(int gamepad_fd, struct input_event gamepad_event) {
  memset(&gamepad_event, 0, sizeof(struct input_event));
  gamepad_event.type = EV_SYN;
  gamepad_event.code = 0;
  gamepad_event.value = 0;

  if (write(gamepad_fd, &gamepad_event, sizeof(struct input_event)) < 0) {
    printf("Error writing sync event\n");
  }
}

// TYPE Is The event to write to the gamepad and CODE is an integer value for
// the button on the gamepad
void send_event(int gamepad_fd, struct input_event gamepad_event, int TYPE, int CODE, int VALUE) {
  memset(&gamepad_event, 0, sizeof(struct input_event));
  gamepad_event.type = TYPE;
  gamepad_event.code = CODE;
  gamepad_event.value = VALUE;

  if (write(gamepad_fd, &gamepad_event, sizeof(struct input_event)) < 0) {
    printf("Error writing event to gamepad!\n");
  } else if (verbose) {
    printf("> Gamepad: type %d code %d value %d \n", gamepad_event.type, gamepad_event.code, gamepad_event.value);
  }
}

void send_event_and_sync(int gamepad_fd, struct input_event gamepad_event, int TYPE, int CODE, int VALUE) {
  send_event(gamepad_fd, gamepad_event, TYPE, CODE, VALUE);
  send_sync_event(gamepad_fd, gamepad_event);
}

void tray_icon_on_click(int keyboard_fd, int gamepad_fd) { exitFunc(keyboard_fd, gamepad_fd); }

static GtkStatusIcon* create_tray_icon(char* start_icon, char* tooltip) {
  GtkStatusIcon* tray_icon;
  tray_icon = gtk_status_icon_new_from_icon_name(start_icon);
  g_signal_connect(G_OBJECT(tray_icon), "activate", G_CALLBACK(tray_icon_on_click), NULL);
  gtk_status_icon_set_visible(tray_icon, TRUE);
  gtk_status_icon_set_tooltip_text(tray_icon, tooltip);
  return tray_icon;
}

void setKeyButtons(int BTN[], int KEYa, int KEYb, int KEYc) {
  BTN[2] = KEYc;
  BTN[1] = KEYb;
  BTN[0] = KEYa;
}

void setAltLayout() {
  printf("Using alternate layout.\n");  // for Monster Hunter Games
  setKeyButtons(BA, KEY_K, KEYUK, KEYUK);
  setKeyButtons(BB, KEY_L, KEYLA, KEYUK);
  setKeyButtons(BX, KEY_J, KEYUK, KEYUK);
  setKeyButtons(BY, KEY_I, KEYUK, KEYUK);
  setKeyButtons(ST, KEY_N, KEYUK, KEYUK);
  setKeyButtons(BK, KEY_X, KEY_B, KEYUK);
  setKeyButtons(GD, KEYEN, KEYUK, KEYUK);
  setKeyButtons(LB, KEY_Q, KEY_R, KEYUK);
  setKeyButtons(RB, KEY_E, KEY_Y, KEYLS);
  setKeyButtons(LT, KEY_Z, KEY_4, KEYSP);
  setKeyButtons(RT, KEY_C, KEY_6, KEYUK);
  setKeyButtons(TL, KEY_V, KEY_5, KEYUK);
  setKeyButtons(TR, KEYTB, KEYUK, KEYUK);
  setKeyButtons(DU, KEY_T, KEYUK, KEYUK);
  setKeyButtons(DD, KEY_G, KEYDT, KEYUK);
  setKeyButtons(DL, KEY_F, KEYUK, KEYUK);
  setKeyButtons(DR, KEY_H, KEYUK, KEYUK);
  setKeyButtons(LU, KEY_W, KEYUK, KEYUK);
  setKeyButtons(LD, KEY_S, KEYUK, KEYUK);
  setKeyButtons(LL, KEY_A, KEYUK, KEYUK);
  setKeyButtons(LR, KEY_D, KEYUK, KEYUK);
  setKeyButtons(RU, KEY_1, KEY_8, KEYUK);
  setKeyButtons(RD, KEY_2, KEY_9, KEYUK);
  setKeyButtons(RL, KEY_U, KEYUK, KEYUK);
  setKeyButtons(RR, KEY_O, KEYUK, KEYUK);
}

void setLayout() {
  printf("Using default layout.\n");  // for Any Game
  setKeyButtons(BA, KEY_K, KEYUK, KEYUK);
  setKeyButtons(BB, KEY_L, KEYUK, KEYUK);
  setKeyButtons(BX, KEY_J, KEYUK, KEYUK);
  setKeyButtons(BY, KEY_I, KEYLA, KEYUK);
  setKeyButtons(ST, KEY_N, KEYUK, KEYUK);
  setKeyButtons(BK, KEY_X, KEYUK, KEYUK);
  setKeyButtons(GD, KEYEN, KEYUK, KEYUK);
  setKeyButtons(LB, KEY_Q, KEY_R, KEYUK);
  setKeyButtons(RB, KEY_E, KEY_Y, KEYUK);
  setKeyButtons(LT, KEYLS, KEY_4, KEYUK);
  setKeyButtons(RT, KEYSP, KEY_6, KEYUK);
  setKeyButtons(TL, KEY_V, KEYUK, KEYUK);
  setKeyButtons(TR, KEY_B, KEYUK, KEYUK);
  setKeyButtons(DU, KEY_T, KEYTB, KEYUK);
  setKeyButtons(DD, KEY_G, KEYDT, KEYUK);
  setKeyButtons(DL, KEY_F, KEY_Z, KEYUK);
  setKeyButtons(DR, KEY_H, KEY_C, KEYUK);
  setKeyButtons(LU, KEY_W, KEYUK, KEYUK);
  setKeyButtons(LD, KEY_S, KEYUK, KEYUK);
  setKeyButtons(LL, KEY_A, KEYUK, KEYUK);
  setKeyButtons(LR, KEY_D, KEYUK, KEYUK);
  setKeyButtons(RU, KEY_1, KEY_8, KEYUK);
  setKeyButtons(RD, KEY_2, KEY_9, KEYUK);
  setKeyButtons(RL, KEY_U, KEYUK, KEYUK);
  setKeyButtons(RR, KEY_O, KEYUK, KEYUK);
}

bool checkButton(int BTNS[], int KEY) {
  bool match = false;
  for (int i = 0; i < 3; i++) {
    if (BTNS[i] == KEY) {
      match = true;
      break;
    }
  }
  return match;
}

int main(int argc, char* argv[]) {
  parse_arguments(argc, argv);
  gtk_init(&argc, &argv);
  icon = create_tray_icon(start_icon, tooltip);

  if (altlay) {
    setAltLayout();
  } else {
    setLayout();
  }

  sleep(1);
  int rcode = 0;

  char keyboard_name[256] = "Unknown";
  int keyboard_fd = open(pathKeyboard, O_RDONLY | O_NONBLOCK);
  if (keyboard_fd == -1) {
    printf("Failed to open keyboard -> %s\n", pathKeyboard);
    exit(1);
  }
  rcode = ioctl(keyboard_fd, EVIOCGNAME(sizeof(keyboard_name)), keyboard_name);
  printf("Reading from: %s \n", keyboard_name);

  printf("Getting exclusive access: ");
  rcode = ioctl(keyboard_fd, EVIOCGRAB, 1);
  printf("%s\n", (rcode == 0) ? "SUCCESS" : "FAILURE");
  struct input_event keyboard_event;

  // Now, create gamepad
  int gamepad_fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
  if (gamepad_fd < 0) {
    printf("Opening of input failed! \n");
    return 1;
  }

  struct uinput_user_dev uidev;
  memset(&uidev, 0, sizeof(uidev));
  // Gamepad Name
  snprintf(uidev.name, UINPUT_MAX_NAME_SIZE, GAMEPAD_NAME);
  uidev.id.bustype = BUS_USB;
  uidev.id.vendor = 0x45e;
  uidev.id.product = 0x28e;
  uidev.id.version = 0x110;
  // Setting Gamepad Buttons
  ioctl(gamepad_fd, UI_SET_EVBIT, EV_KEY);
  ioctl(gamepad_fd, UI_SET_KEYBIT, BTN_SOUTH);  // A
  ioctl(gamepad_fd, UI_SET_KEYBIT, BTN_EAST);   // B
  ioctl(gamepad_fd, UI_SET_KEYBIT, BTN_NORTH);  // Y
  ioctl(gamepad_fd, UI_SET_KEYBIT, BTN_WEST);   // X

  ioctl(gamepad_fd, UI_SET_KEYBIT, BTN_TL);   // LB
  ioctl(gamepad_fd, UI_SET_KEYBIT, BTN_TR);   // RB
  ioctl(gamepad_fd, UI_SET_KEYBIT, BTN_TL2);  // LT
  ioctl(gamepad_fd, UI_SET_KEYBIT, BTN_TR2);  // RT

  ioctl(gamepad_fd, UI_SET_KEYBIT, BTN_START);   // Start
  ioctl(gamepad_fd, UI_SET_KEYBIT, BTN_SELECT);  // Back
  ioctl(gamepad_fd, UI_SET_KEYBIT, BTN_MODE);    // Guide

  ioctl(gamepad_fd, UI_SET_KEYBIT, BTN_THUMBL);  // Left Thumb
  ioctl(gamepad_fd, UI_SET_KEYBIT, BTN_THUMBR);  // Right Thumb

  ioctl(gamepad_fd, UI_SET_KEYBIT, BTN_DPAD_UP);     // Dpad Up
  ioctl(gamepad_fd, UI_SET_KEYBIT, BTN_DPAD_DOWN);   // Dpad Down
  ioctl(gamepad_fd, UI_SET_KEYBIT, BTN_DPAD_LEFT);   // Dpad Left
  ioctl(gamepad_fd, UI_SET_KEYBIT, BTN_DPAD_RIGHT);  // Dpad Right
  // Setting Gamepad Sticks
  ioctl(gamepad_fd, UI_SET_EVBIT, EV_ABS);
  ioctl(gamepad_fd, UI_SET_ABSBIT, ABS_X);
  ioctl(gamepad_fd, UI_SET_ABSBIT, ABS_Y);
  ioctl(gamepad_fd, UI_SET_ABSBIT, ABS_RX);
  ioctl(gamepad_fd, UI_SET_ABSBIT, ABS_RY);
  // Left Stick
  uidev.absmax[ABS_X] = 512;
  uidev.absmin[ABS_X] = -512;
  uidev.absfuzz[ABS_X] = 0;
  uidev.absflat[ABS_X] = 16;
  uidev.absmax[ABS_Y] = 512;
  uidev.absmin[ABS_Y] = -512;
  uidev.absfuzz[ABS_Y] = 0;
  uidev.absflat[ABS_Y] = 16;
  // Right Stick
  uidev.absmax[ABS_RX] = 512;
  uidev.absmin[ABS_RX] = -512;
  uidev.absfuzz[ABS_RX] = 0;
  uidev.absflat[ABS_RX] = 16;
  uidev.absmax[ABS_RY] = 512;
  uidev.absmin[ABS_RY] = -512;
  uidev.absfuzz[ABS_RY] = 0;
  uidev.absflat[ABS_RY] = 16;

  if (write(gamepad_fd, &uidev, sizeof(uidev)) < 0) {
    printf("Failed to write! \n");
    return 1;
  }
  if (ioctl(gamepad_fd, UI_DEV_CREATE) < 0) {
    printf("Failed to create gamepad! \n");
    return 1;
  }

  sleep(0.6);

  struct input_event gamepad_ev;

  while (1) {
    sleep(0.001);
    gtk_main_iteration_do(FALSE);

    if (read(keyboard_fd, &keyboard_event, sizeof(keyboard_event)) != -1) {
      if (verbose) {
        if (paused) printf("\n");
        printf("> Keyboard: type %d code %d value %d \n", keyboard_event.type, keyboard_event.code, keyboard_event.value);
      }

      // Pause Gamepad Toggle (F2)
      if (keyboard_event.code == KEY_F2 && keyboard_event.value == 0) {
        paused = !paused;
        grab = !grab;
        ioctl(keyboard_fd, EVIOCGRAB, grab);
        if (paused) {
          // Pause Icon
          gtk_status_icon_set_from_icon_name(icon, "media-playback-pause-symbolic");
          // Reset Joysticks (Fix joysticks getting stuck)
          xaxis = 0;
          yaxis = 0;
          rxaxis = 0;
          ryaxis = 0;
          send_event(gamepad_fd, gamepad_ev, EV_ABS, ABS_X, 0);
          send_event(gamepad_fd, gamepad_ev, EV_ABS, ABS_Y, 0);
          send_event(gamepad_fd, gamepad_ev, EV_ABS, ABS_RX, 0);
          send_event(gamepad_fd, gamepad_ev, EV_ABS, ABS_RY, 0);
          send_sync_event(gamepad_fd, gamepad_ev);
        } else {
          // Gamepad Icon
          gtk_status_icon_set_from_icon_name(icon, "applications-games-symbolic");
        }
      }

      // Exit with F12 Key
      if (keyboard_event.code == KEY_F12 && keyboard_event.value == 0) {
        exitFunc(keyboard_fd, gamepad_fd);
        break;
      }

      if (!paused) {
        /* Face Buttons */
        if (keyboard_event.value != 2)  // only care about button press and not hold
        {
          if (checkButton(BA, keyboard_event.code)) {
            send_event_and_sync(gamepad_fd, gamepad_ev, EV_KEY, BTN_SOUTH, keyboard_event.value);
          }
          if (checkButton(BB, keyboard_event.code)) {
            send_event_and_sync(gamepad_fd, gamepad_ev, EV_KEY, BTN_EAST, keyboard_event.value);
          }
          if (checkButton(BX, keyboard_event.code)) {
            send_event_and_sync(gamepad_fd, gamepad_ev, EV_KEY, BTN_WEST, keyboard_event.value);
          }
          if (checkButton(BY, keyboard_event.code)) {
            send_event_and_sync(gamepad_fd, gamepad_ev, EV_KEY, BTN_NORTH, keyboard_event.value);
          }
          if (checkButton(ST, keyboard_event.code)) {
            send_event_and_sync(gamepad_fd, gamepad_ev, EV_KEY, BTN_START, keyboard_event.value);
          }
          if (checkButton(BK, keyboard_event.code)) {
            send_event_and_sync(gamepad_fd, gamepad_ev, EV_KEY, BTN_SELECT, keyboard_event.value);
          }
          if (checkButton(GD, keyboard_event.code)) {
            send_event_and_sync(gamepad_fd, gamepad_ev, EV_KEY, BTN_MODE, keyboard_event.value);
          }
        }

        /* BUMPERS and TRIGGERS */
        if (checkButton(LB, keyboard_event.code)) {
          send_event_and_sync(gamepad_fd, gamepad_ev, EV_KEY, BTN_TL, keyboard_event.value);
        }
        if (checkButton(RB, keyboard_event.code)) {
          send_event_and_sync(gamepad_fd, gamepad_ev, EV_KEY, BTN_TR, keyboard_event.value);
        }
        if (checkButton(LT, keyboard_event.code)) {
          lt_down = keyboard_event.value == 2 ? 1 : 0;
          send_event_and_sync(gamepad_fd, gamepad_ev, EV_KEY, BTN_TL2, keyboard_event.value);
        }
        if (checkButton(RT, keyboard_event.code)) {
          rt_down = keyboard_event.value == 2 ? 1 : 0;
          send_event_and_sync(gamepad_fd, gamepad_ev, EV_KEY, BTN_TR2, keyboard_event.value);
        }

        /* Left and Right Thumb */
        if (checkButton(TL, keyboard_event.code)) {
          send_event_and_sync(gamepad_fd, gamepad_ev, EV_KEY, BTN_THUMBL, keyboard_event.value);
        }
        if (checkButton(TR, keyboard_event.code)) {
          send_event_and_sync(gamepad_fd, gamepad_ev, EV_KEY, BTN_THUMBR, keyboard_event.value);
        }

        /* DPAD */
        if (checkButton(DU, keyboard_event.code)) {
          send_event_and_sync(gamepad_fd, gamepad_ev, EV_KEY, BTN_DPAD_UP, keyboard_event.value);
        }
        if (checkButton(DD, keyboard_event.code)) {
          send_event_and_sync(gamepad_fd, gamepad_ev, EV_KEY, BTN_DPAD_DOWN, keyboard_event.value);
        }
        if (checkButton(DL, keyboard_event.code)) {
          send_event_and_sync(gamepad_fd, gamepad_ev, EV_KEY, BTN_DPAD_LEFT, keyboard_event.value);
        }
        if (checkButton(DR, keyboard_event.code)) {
          send_event_and_sync(gamepad_fd, gamepad_ev, EV_KEY, BTN_DPAD_RIGHT, keyboard_event.value);
        }

        /* Analog Sticks */
        bool pressedOrHold = keyboard_event.value == 4 || keyboard_event.value == 1;
        if (pressedOrHold) {
          // Left
          if (checkButton(LU, keyboard_event.code)) {
            yaxis -= 1;
          }
          if (checkButton(LD, keyboard_event.code)) {
            yaxis += 1;
          }
          if (checkButton(LL, keyboard_event.code)) {
            xaxis -= 1;
          }
          if (checkButton(LR, keyboard_event.code)) {
            xaxis += 1;
          }
          // Right
          if (checkButton(RU, keyboard_event.code)) {
            ryaxis -= 1;
          }
          if (checkButton(RD, keyboard_event.code)) {
            ryaxis += 1;
          }
          if (checkButton(RL, keyboard_event.code)) {
            rxaxis -= 1;
          }
          if (checkButton(RR, keyboard_event.code)) {
            rxaxis += 1;
          }
        } else if (keyboard_event.value == 0) {
          // Left
          if (checkButton(LU, keyboard_event.code)) {
            yaxis += 1;
          }
          if (checkButton(LD, keyboard_event.code)) {
            yaxis -= 1;
          }
          if (checkButton(LL, keyboard_event.code)) {
            xaxis += 1;
          }
          if (checkButton(LR, keyboard_event.code)) {
            xaxis -= 1;
          }
          // Right
          if (checkButton(RU, keyboard_event.code)) {
            ryaxis += 1;
          }
          if (checkButton(RD, keyboard_event.code)) {
            ryaxis -= 1;
          }
          if (checkButton(RL, keyboard_event.code)) {
            rxaxis += 1;
          }
          if (checkButton(RR, keyboard_event.code)) {
            rxaxis -= 1;
          }
        }
        /* Left Stick */
        send_event_and_sync(gamepad_fd, gamepad_ev, EV_ABS, ABS_X, xaxis == 0 ? 0 : (xaxis == 1 ? 512 : -512));
        send_event_and_sync(gamepad_fd, gamepad_ev, EV_ABS, ABS_Y, yaxis == 0 ? 0 : (yaxis == 1 ? 512 : -512));
        /* Right Stick */
        if (lt_down == 1 && altlay) {  // Lower sensitivity while holding LT in alternate layout
          send_event_and_sync(gamepad_fd, gamepad_ev, EV_ABS, ABS_RX, rxaxis == 0 ? 0 : (rxaxis == 1 ? 288 : -288));
          send_event_and_sync(gamepad_fd, gamepad_ev, EV_ABS, ABS_RY, ryaxis == 0 ? 0 : (ryaxis == 1 ? 288 : -288));
        } else if (rt_down == 1 && !altlay) {  // Lower sensitivity while holding RT in default layout
          send_event_and_sync(gamepad_fd, gamepad_ev, EV_ABS, ABS_RX, rxaxis == 0 ? 0 : (rxaxis == 1 ? 288 : -288));
          send_event_and_sync(gamepad_fd, gamepad_ev, EV_ABS, ABS_RY, ryaxis == 0 ? 0 : (ryaxis == 1 ? 288 : -288));
        } else {  // Default sensitivity
          send_event_and_sync(gamepad_fd, gamepad_ev, EV_ABS, ABS_RX, rxaxis == 0 ? 0 : (rxaxis == 1 ? 512 : -512));
          send_event_and_sync(gamepad_fd, gamepad_ev, EV_ABS, ABS_RY, ryaxis == 0 ? 0 : (ryaxis == 1 ? 512 : -512));
        }

        if (verbose) printf("\n");
      }  // paused
    }  // keyboard_fd
  }  // while
  return 0;
}  // main
