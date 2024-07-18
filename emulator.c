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
char *tooltip = GAMEPAD_NAME;
char *start_icon = "applications-games-symbolic";
char pathKeyboard[256] = "???";
char xaxis = 0;
char yaxis = 0;
char rxaxis = 0;
char ryaxis = 0;
bool verbose = false;
bool paused = false;
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
  }

  return 0;
}

int parse_arguments(int argc, char **argv) {
  struct argp_option options[] = {{"keyboard", 'k', "PATH", 0, "The path to keyboard (should look something like /dev/input/eventX)"},
                                  {"verbose", 'v', 0, OPTION_ARG_OPTIONAL, "Show more info"},
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
    printf("-> Gamepad: type %d code %d value %d ", gamepad_event.type, gamepad_event.code, gamepad_event.value);
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

int main(int argc, char *argv[]) {
  parse_arguments(argc, argv);
  gtk_init(&argc, &argv);
  icon = create_tray_icon(start_icon, tooltip);

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
        printf("Event: Keyboard: type %d code %d value %d ", keyboard_event.type, keyboard_event.code, keyboard_event.value);
      }

      // Pause Gamepad Toggle (F2)
      if (keyboard_event.code == KEY_F2 && keyboard_event.value == 0) {
        grab = !grab;
        paused = !paused;
        rcode = ioctl(keyboard_fd, EVIOCGRAB, grab);
        if (paused) {
          // Pause Icon
          gtk_status_icon_set_from_icon_name(icon, "media-playback-pause-symbolic");
        } else {
          // Gamepad Icon
          gtk_status_icon_set_from_icon_name(icon, "applications-games-symbolic");
        }
        send_sync_event(gamepad_fd, gamepad_ev);  // reset gamepad
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
          switch (keyboard_event.code) {
            case KEY_K:
              send_event_and_sync(gamepad_fd, gamepad_ev, EV_KEY, BTN_A, keyboard_event.value);
              break;
            case KEY_L:
              send_event_and_sync(gamepad_fd, gamepad_ev, EV_KEY, BTN_B, keyboard_event.value);
              break;
            case KEY_J:
              send_event_and_sync(gamepad_fd, gamepad_ev, EV_KEY, BTN_X, keyboard_event.value);
              break;
            case KEY_I:
              send_event_and_sync(gamepad_fd, gamepad_ev, EV_KEY, BTN_Y, keyboard_event.value);
              break;
            case KEY_N:
              send_event_and_sync(gamepad_fd, gamepad_ev, EV_KEY, BTN_START, keyboard_event.value);
              break;
            case KEY_X:
            case KEY_B:
              send_event_and_sync(gamepad_fd, gamepad_ev, EV_KEY, BTN_SELECT, keyboard_event.value);
              break;
            case KEY_ENTER:
              send_event_and_sync(gamepad_fd, gamepad_ev, EV_KEY, BTN_MODE, keyboard_event.value);
              break;
          }
        }

        /* Analog Sticks: Left & Right */
        bool pressedOrHold = keyboard_event.value == 4 || keyboard_event.value == 1;
        if (pressedOrHold) {
          switch (keyboard_event.code) {
            case KEY_W:
              yaxis -= 1;
              break;
            case KEY_S:
              yaxis += 1;
              break;
            case KEY_A:
              xaxis -= 1;
              break;
            case KEY_D:
              xaxis += 1;
              break;
            case KEY_1:
            case KEY_8:
              ryaxis -= 1;
              break;
            case KEY_2:
            case KEY_9:
              ryaxis += 1;
              break;
            case KEY_U:
              rxaxis -= 1;
              break;
            case KEY_O:
              rxaxis += 1;
              break;
          }
        } else if (keyboard_event.value == 0) {
          switch (keyboard_event.code) {
            case KEY_W:
              yaxis += 1;
              break;
            case KEY_S:
              yaxis -= 1;
              break;
            case KEY_A:
              xaxis += 1;
              break;
            case KEY_D:
              xaxis -= 1;
              break;
            case KEY_1:
            case KEY_8:
              ryaxis += 1;
              break;
            case KEY_2:
            case KEY_9:
              ryaxis -= 1;
              break;
            case KEY_U:
              rxaxis += 1;
              break;
            case KEY_O:
              rxaxis -= 1;
              break;
          }
        }
        /* Left */
        send_event_and_sync(gamepad_fd, gamepad_ev, EV_ABS, ABS_X, xaxis == 0 ? 0 : (xaxis == 1 ? 32767 : -32768));
        send_event_and_sync(gamepad_fd, gamepad_ev, EV_ABS, ABS_Y, yaxis == 0 ? 0 : (yaxis == 1 ? 32767 : -32768));
        /* Right */
        send_event_and_sync(gamepad_fd, gamepad_ev, EV_ABS, ABS_RX, rxaxis == 0 ? 0 : (rxaxis == 1 ? 32767 : -32768));
        send_event_and_sync(gamepad_fd, gamepad_ev, EV_ABS, ABS_RY, ryaxis == 0 ? 0 : (ryaxis == 1 ? 32767 : -32768));

        switch (keyboard_event.code) {
          /* TRIGGERS and BUMPERS */
          case KEY_Q:
          case KEY_R:
            send_event_and_sync(gamepad_fd, gamepad_ev, EV_KEY, BTN_TL, keyboard_event.value);
            break;
          case KEY_E:
          case KEY_Y:
          case KEY_LEFTSHIFT:
            send_event_and_sync(gamepad_fd, gamepad_ev, EV_KEY, BTN_TR, keyboard_event.value);
            break;
          case KEY_Z:
          case KEY_4:
          case KEY_SPACE:
            send_event_and_sync(gamepad_fd, gamepad_ev, EV_KEY, BTN_TL2, keyboard_event.value);
            break;
          case KEY_C:
          case KEY_6:
            send_event_and_sync(gamepad_fd, gamepad_ev, EV_KEY, BTN_TR2, keyboard_event.value);
            break;
          /* L3 and R3 */
          case KEY_5:
          case KEY_TAB:
            send_event_and_sync(gamepad_fd, gamepad_ev, EV_KEY, BTN_THUMBL, keyboard_event.value);
            break;
          case KEY_V:
            send_event_and_sync(gamepad_fd, gamepad_ev, EV_KEY, BTN_THUMBR, keyboard_event.value);
            break;
          /* DPAD */
          case KEY_T:
            send_event_and_sync(gamepad_fd, gamepad_ev, EV_KEY, BTN_DPAD_UP, keyboard_event.value);
            break;
          case KEY_G:
            send_event_and_sync(gamepad_fd, gamepad_ev, EV_KEY, BTN_DPAD_DOWN, keyboard_event.value);
            break;
          case KEY_F:
            send_event_and_sync(gamepad_fd, gamepad_ev, EV_KEY, BTN_DPAD_LEFT, keyboard_event.value);
            break;
          case KEY_H:
            send_event_and_sync(gamepad_fd, gamepad_ev, EV_KEY, BTN_DPAD_RIGHT, keyboard_event.value);
            break;
        }

        if (verbose) printf("\n");
      }  // paused
    }  // keyboard_fd
  }  // while
  return 0;
}  // main
