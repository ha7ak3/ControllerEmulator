#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#include <argp.h>
#include <errno.h>
#include <fcntl.h>
#include <gtk/gtk.h>
#include <linux/input.h>
#include <linux/uinput.h>
#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "keys.h"
#define GAMEPAD_NAME "Virtual Xbox Gamepad"
int BA[] = {KEY_K, KEY_K, KEY_K};
int BB[] = {KEY_L, KEY_L, KEY_LEFTALT};
int BX[] = {KEY_J, KEY_J, KEY_J};
int BY[] = {KEY_I, KEY_I, KEY_I};
int ST[] = {KEY_N, KEY_N, KEY_N};
int BK[] = {KEY_X, KEY_X, KEY_B};
int GD[] = {KEY_ENTER, KEY_ENTER, KEY_ENTER};
int LB[] = {KEY_Q, KEY_Q, KEY_R};
int RB[] = {KEY_E, KEY_Y, KEY_LEFTSHIFT};
int LT[] = {KEY_Z, KEY_4, KEY_SPACE};
int RT[] = {KEY_C, KEY_C, KEY_6};
int TL[] = {KEY_TAB, KEY_TAB, KEY_5};
int TR[] = {KEY_V, KEY_V, KEY_V};
int DU[] = {KEY_T, KEY_T, KEY_T};
int DD[] = {KEY_G, KEY_G, KEY_DOT};
int DL[] = {KEY_F, KEY_F, KEY_F};
int DR[] = {KEY_H, KEY_H, KEY_H};
int LU[] = {KEY_W, KEY_W, KEY_W};
int LD[] = {KEY_S, KEY_S, KEY_S};
int LL[] = {KEY_A, KEY_A, KEY_A};
int LR[] = {KEY_D, KEY_D, KEY_D};
int RU[] = {KEY_1, KEY_1, KEY_8};
int RD[] = {KEY_2, KEY_2, KEY_9};
int RL[] = {KEY_U, KEY_U, KEY_U};
int RR[] = {KEY_O, KEY_O, KEY_O};
char *tooltip = GAMEPAD_NAME;
char *start_icon = "applications-games-symbolic";
char pathKeyboard[256] = "???";
char xaxis = 0;
char yaxis = 0;
char rxaxis = 0;
char ryaxis = 0;
bool verbose = false;
bool paused = false;
bool altlay = false;
int rt_down = 0;
int grab = 1;
GtkStatusIcon *icon;

static int parse_opt(int key, char *arg, struct argp_state *state) {
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

int parse_arguments(int argc, char **argv) {
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
  close(keyboard_fd);                // Close keyboard
  close(gamepad_fd);                 // Close gamepad
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

static GtkStatusIcon *create_tray_icon(char *start_icon, char *tooltip) {
  GtkStatusIcon *tray_icon;
  tray_icon = gtk_status_icon_new_from_icon_name(start_icon);
  g_signal_connect(G_OBJECT(tray_icon), "activate", G_CALLBACK(tray_icon_on_click), NULL);
  gtk_status_icon_set_visible(tray_icon, TRUE);
  gtk_status_icon_set_tooltip_text(tray_icon, tooltip);
  return tray_icon;
}

void waitReleaseAll(int fd) {
  struct input_event evt;
  unsigned char key_b[KEY_MAX / 8 + 1];
  int i, nothing;
  while (1) {
    memset(key_b, 0, sizeof(key_b));
    ioctl(fd, EVIOCGKEY(sizeof(key_b)), key_b);
    for (nothing = 1, i = 0; i < KEY_MAX / 8 + 1; i++) {
      if (key_b[i] != 0) {
        nothing = 0;
        break;
      }
    }
    if (nothing) break;
    read(fd, &evt, sizeof(evt));
  }
  if (verbose) printf("All keys are now released\n");
}

void setAltLayout() {
  printf("Using alternate layout.\n");
  BB[2] = KEY_L;
  BY[2] = KEY_LEFTALT;
  BK[2] = KEY_X;
  LB[2] = KEY_Q;
  RB[1] = KEY_E;
  RB[2] = KEY_H;
  LT[0] = KEY_LEFTSHIFT;
  LT[1] = KEY_LEFTSHIFT;
  LT[2] = KEY_LEFTSHIFT;
  RT[0] = KEY_SPACE;
  RT[1] = KEY_SPACE;
  RT[2] = KEY_SPACE;
  TL[0] = KEY_C;
  TL[1] = KEY_C;
  TL[2] = KEY_C;
  TR[0] = KEY_Z;
  TR[1] = KEY_Z;
  TR[2] = KEY_Z;
  DU[0] = KEY_R;
  DU[1] = KEY_R;
  DU[2] = KEY_R;
  DD[0] = KEY_B;
  DD[1] = KEY_B;
  DD[2] = KEY_B;
  DR[0] = KEY_V;
  DR[1] = KEY_V;
  DR[2] = KEY_V;
}

int main(int argc, char *argv[]) {
  parse_arguments(argc, argv);
  gtk_init(&argc, &argv);
  icon = create_tray_icon(start_icon, tooltip);

  if (altlay) setAltLayout();

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

  ioctl(gamepad_fd, UI_SET_EVBIT, EV_KEY);  // setting Gamepad keys
  ioctl(gamepad_fd, UI_SET_KEYBIT, BTN_A);
  ioctl(gamepad_fd, UI_SET_KEYBIT, BTN_B);
  ioctl(gamepad_fd, UI_SET_KEYBIT, BTN_X);
  ioctl(gamepad_fd, UI_SET_KEYBIT, BTN_Y);

  ioctl(gamepad_fd, UI_SET_KEYBIT, BTN_TL);
  ioctl(gamepad_fd, UI_SET_KEYBIT, BTN_TR);
  ioctl(gamepad_fd, UI_SET_KEYBIT, BTN_TL2);
  ioctl(gamepad_fd, UI_SET_KEYBIT, BTN_TR2);

  ioctl(gamepad_fd, UI_SET_KEYBIT, BTN_START);
  ioctl(gamepad_fd, UI_SET_KEYBIT, BTN_SELECT);
  ioctl(gamepad_fd, UI_SET_KEYBIT, BTN_MODE);

  ioctl(gamepad_fd, UI_SET_KEYBIT, BTN_THUMBL);
  ioctl(gamepad_fd, UI_SET_KEYBIT, BTN_THUMBR);

  ioctl(gamepad_fd, UI_SET_KEYBIT, BTN_DPAD_UP);
  ioctl(gamepad_fd, UI_SET_KEYBIT, BTN_DPAD_DOWN);
  ioctl(gamepad_fd, UI_SET_KEYBIT, BTN_DPAD_LEFT);
  ioctl(gamepad_fd, UI_SET_KEYBIT, BTN_DPAD_RIGHT);

  ioctl(gamepad_fd, UI_SET_EVBIT, EV_ABS);  // setting Gamepad thumbsticks
  ioctl(gamepad_fd, UI_SET_ABSBIT, ABS_X);
  ioctl(gamepad_fd, UI_SET_ABSBIT, ABS_Y);
  ioctl(gamepad_fd, UI_SET_ABSBIT, ABS_RX);
  ioctl(gamepad_fd, UI_SET_ABSBIT, ABS_RY);

  struct uinput_user_dev uidev;

  memset(&uidev, 0, sizeof(uidev));
  snprintf(uidev.name, UINPUT_MAX_NAME_SIZE, GAMEPAD_NAME);  // Name of Gamepad
  uidev.id.bustype = BUS_USB;
  uidev.id.vendor = 0x45e;
  uidev.id.product = 0x28e;
  uidev.id.version = 0x110;
  // Parameters of thumbsticks
  uidev.absmax[ABS_X] = 32767;
  uidev.absmin[ABS_X] = -32768;
  uidev.absfuzz[ABS_X] = 0;
  uidev.absflat[ABS_X] = 15;
  uidev.absmax[ABS_Y] = 32767;
  uidev.absmin[ABS_Y] = -32768;
  uidev.absfuzz[ABS_Y] = 0;
  uidev.absflat[ABS_Y] = 15;
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
        grab = !grab;
        paused = !paused;
        if (paused) {
          waitReleaseAll(gamepad_fd);
          // Pause Icon
          gtk_status_icon_set_from_icon_name(icon, "media-playback-pause-symbolic");
        } else {
          waitReleaseAll(keyboard_fd);
          // Gamepad Icon
          gtk_status_icon_set_from_icon_name(icon, "applications-games-symbolic");
        }
        ioctl(keyboard_fd, EVIOCGRAB, grab);
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
          if (keyboard_event.code == BA[0] || keyboard_event.code == BA[1] || keyboard_event.code == BA[2]) {
            send_event_and_sync(gamepad_fd, gamepad_ev, EV_KEY, BTN_A, keyboard_event.value);
          }
          if (keyboard_event.code == BB[0] || keyboard_event.code == BB[1] || keyboard_event.code == BB[2]) {
            send_event_and_sync(gamepad_fd, gamepad_ev, EV_KEY, BTN_B, keyboard_event.value);
          }
          if (keyboard_event.code == BX[0] || keyboard_event.code == BX[1] || keyboard_event.code == BX[2]) {
            send_event_and_sync(gamepad_fd, gamepad_ev, EV_KEY, BTN_X, keyboard_event.value);
          }
          if (keyboard_event.code == BY[0] || keyboard_event.code == BY[1] || keyboard_event.code == BY[2]) {
            send_event_and_sync(gamepad_fd, gamepad_ev, EV_KEY, BTN_Y, keyboard_event.value);
          }
          if (keyboard_event.code == ST[0] || keyboard_event.code == ST[1] || keyboard_event.code == ST[2]) {
            send_event_and_sync(gamepad_fd, gamepad_ev, EV_KEY, BTN_START, keyboard_event.value);
          }
          if (keyboard_event.code == BK[0] || keyboard_event.code == BK[1] || keyboard_event.code == BK[2]) {
            send_event_and_sync(gamepad_fd, gamepad_ev, EV_KEY, BTN_SELECT, keyboard_event.value);
          }
          if (keyboard_event.code == GD[0] || keyboard_event.code == GD[1] || keyboard_event.code == GD[2]) {
            send_event_and_sync(gamepad_fd, gamepad_ev, EV_KEY, BTN_MODE, keyboard_event.value);
          }
        }

        /* TRIGGERS and BUMPERS */
        if (keyboard_event.code == LB[0] || keyboard_event.code == LB[1] || keyboard_event.code == LB[2]) {
          send_event_and_sync(gamepad_fd, gamepad_ev, EV_KEY, BTN_TL, keyboard_event.value);
        }
        if (keyboard_event.code == RB[0] || keyboard_event.code == RB[1] || keyboard_event.code == RB[2]) {
          send_event_and_sync(gamepad_fd, gamepad_ev, EV_KEY, BTN_TR, keyboard_event.value);
        }
        if (keyboard_event.code == LT[0] || keyboard_event.code == LT[1] || keyboard_event.code == LT[2]) {
          send_event_and_sync(gamepad_fd, gamepad_ev, EV_KEY, BTN_TL2, keyboard_event.value);
        }
        if (keyboard_event.code == RT[0] || keyboard_event.code == RT[1] || keyboard_event.code == RT[2]) {
          rt_down = keyboard_event.value == 2 ? 1 : 0;
          send_event_and_sync(gamepad_fd, gamepad_ev, EV_KEY, BTN_TR2, keyboard_event.value);
        }

        /* L3 and R3 */
        if (keyboard_event.code == TL[0] || keyboard_event.code == TL[1] || keyboard_event.code == TL[2]) {
          send_event_and_sync(gamepad_fd, gamepad_ev, EV_KEY, BTN_THUMBL, keyboard_event.value);
        }
        if (keyboard_event.code == TR[0] || keyboard_event.code == TR[1] || keyboard_event.code == TR[2]) {
          send_event_and_sync(gamepad_fd, gamepad_ev, EV_KEY, BTN_THUMBR, keyboard_event.value);
        }

        /* DPAD */
        if (keyboard_event.code == DU[0] || keyboard_event.code == DU[1] || keyboard_event.code == DU[2]) {
          send_event_and_sync(gamepad_fd, gamepad_ev, EV_KEY, BTN_DPAD_UP, keyboard_event.value);
        }
        if (keyboard_event.code == DD[0] || keyboard_event.code == DD[1] || keyboard_event.code == DD[2]) {
          send_event_and_sync(gamepad_fd, gamepad_ev, EV_KEY, BTN_DPAD_DOWN, keyboard_event.value);
        }
        if (keyboard_event.code == DL[0] || keyboard_event.code == DL[1] || keyboard_event.code == DL[2]) {
          send_event_and_sync(gamepad_fd, gamepad_ev, EV_KEY, BTN_DPAD_LEFT, keyboard_event.value);
        }
        if (keyboard_event.code == DR[0] || keyboard_event.code == DR[1] || keyboard_event.code == DR[2]) {
          send_event_and_sync(gamepad_fd, gamepad_ev, EV_KEY, BTN_DPAD_RIGHT, keyboard_event.value);
        }

        /* Analog Sticks */
        bool pressedOrHold = keyboard_event.value == 4 || keyboard_event.value == 1;
        if (pressedOrHold) {
          // Left
          if (keyboard_event.code == LU[0] || keyboard_event.code == LU[1] || keyboard_event.code == LU[2]) {
            yaxis -= 1;
          }
          if (keyboard_event.code == LD[0] || keyboard_event.code == LD[1] || keyboard_event.code == LD[2]) {
            yaxis += 1;
          }
          if (keyboard_event.code == LL[0] || keyboard_event.code == LL[1] || keyboard_event.code == LL[2]) {
            xaxis -= 1;
          }
          if (keyboard_event.code == LR[0] || keyboard_event.code == LR[1] || keyboard_event.code == LR[2]) {
            xaxis += 1;
          }
          // Right
          if (keyboard_event.code == RU[0] || keyboard_event.code == RU[1] || keyboard_event.code == RU[2]) {
            ryaxis -= 1;
          }
          if (keyboard_event.code == RD[0] || keyboard_event.code == RD[1] || keyboard_event.code == RD[2]) {
            ryaxis += 1;
          }
          if (keyboard_event.code == RL[0] || keyboard_event.code == RL[1] || keyboard_event.code == RL[2]) {
            rxaxis -= 1;
          }
          if (keyboard_event.code == RR[0] || keyboard_event.code == RR[1] || keyboard_event.code == RR[2]) {
            rxaxis += 1;
          }
        } else if (keyboard_event.value == 0) {
          // Left
          if (keyboard_event.code == LU[0] || keyboard_event.code == LU[1] || keyboard_event.code == LU[2]) {
            yaxis += 1;
          }
          if (keyboard_event.code == LD[0] || keyboard_event.code == LD[1] || keyboard_event.code == LD[2]) {
            yaxis -= 1;
          }
          if (keyboard_event.code == LL[0] || keyboard_event.code == LL[1] || keyboard_event.code == LL[2]) {
            xaxis += 1;
          }
          if (keyboard_event.code == LR[0] || keyboard_event.code == LR[1] || keyboard_event.code == LR[2]) {
            xaxis -= 1;
          }
          // Right
          if (keyboard_event.code == RU[0] || keyboard_event.code == RU[1] || keyboard_event.code == RU[2]) {
            ryaxis += 1;
          }
          if (keyboard_event.code == RD[0] || keyboard_event.code == RD[1] || keyboard_event.code == RD[2]) {
            ryaxis -= 1;
          }
          if (keyboard_event.code == RL[0] || keyboard_event.code == RL[1] || keyboard_event.code == RL[2]) {
            rxaxis += 1;
          }
          if (keyboard_event.code == RR[0] || keyboard_event.code == RR[1] || keyboard_event.code == RR[2]) {
            rxaxis -= 1;
          }
        }
        /* Left Stick */
        send_event_and_sync(gamepad_fd, gamepad_ev, EV_ABS, ABS_X, xaxis == 0 ? 0 : (xaxis == 1 ? 32767 : -32768));
        send_event_and_sync(gamepad_fd, gamepad_ev, EV_ABS, ABS_Y, yaxis == 0 ? 0 : (yaxis == 1 ? 32767 : -32768));
        /* Right Stick */
        if (rt_down == 1 && altlay) {
          send_event_and_sync(gamepad_fd, gamepad_ev, EV_ABS, ABS_RX, rxaxis == 0 ? 0 : (rxaxis == 1 ? 288 : -288));
          send_event_and_sync(gamepad_fd, gamepad_ev, EV_ABS, ABS_RY, ryaxis == 0 ? 0 : (ryaxis == 1 ? 288 : -288));
        } else {
          send_event_and_sync(gamepad_fd, gamepad_ev, EV_ABS, ABS_RX, rxaxis == 0 ? 0 : (rxaxis == 1 ? 512 : -512));
          send_event_and_sync(gamepad_fd, gamepad_ev, EV_ABS, ABS_RY, ryaxis == 0 ? 0 : (ryaxis == 1 ? 512 : -512));
        }

        if (verbose) printf("\n");
      }  // paused
    }  // keyboard_fd
  }  // while
  return 0;
}  // main
