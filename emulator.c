#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <argp.h>
#include <fcntl.h>
#include <linux/input.h>
#include <linux/uinput.h>
#include <time.h>
#include <errno.h>
#include <stdint.h>
#include <unistd.h>
#include <signal.h>
#include "keys.h"

#define GAMEPAD_NAME "Virtual XPAD"

#define MOUSE_SENSITIVITY 512
#define MOUSE_SENSITIVITY_NEGATIVE -512

char pathKeyboard[256] = "???";
char pathMouse[256] = "???";

bool verbose = false;

bool q_pressed = false;

int absMultiplier = 8;

static int parse_opt(int key, char *arg, struct argp_state *state)
{
    switch(key)
    {
    case 'm':
        if(strlen(arg)<256) strcpy(pathMouse,arg);
        break;
    case 'k':
        if(strlen(arg)<256) strcpy(pathKeyboard,arg);
        break;
    case 'v':
        verbose = true;
        break;
    }

    return 0;
}

int parse_arguments(int argc, char**argv)
{
    struct argp_option options[] =
    {
        { "mouse", 'm', "PATH", 0, "The path to mouse (should look something like /dev/input/eventX)" },
        { "keyboard", 'k', "PATH", 0, "The path to keyboard (should look something like /dev/input/eventX)" },
        { "verbose", 'v', 0, OPTION_ARG_OPTIONAL, "Show more info" },
        { 0 }
    };
    struct argp argp = { options, parse_opt };

    return argp_parse (&argp, argc, argv, 0, 0, 0);
}

void exitFunc(int keyboard_fd, int mouse_fd, int gamepad_fd)
{
    close(keyboard_fd);
    close(mouse_fd);
    if (ioctl(gamepad_fd, UI_DEV_DESTROY) < 0)
    {
        printf("Error destroying gamepad! \n");
    }
    close(gamepad_fd);
}

void send_sync_event(int gamepad_fd, struct input_event gamepad_event)
{
    memset(&gamepad_event, 0, sizeof(struct input_event));
    gamepad_event.type = EV_SYN;
    gamepad_event.code = 0;
    gamepad_event.value = 0;

    if(write(gamepad_fd, &gamepad_event, sizeof(struct input_event)) < 0)
    {
        printf("Error writing sync event\n");
    }
}

// TYPE Is The event to write to the gamepad and CODE is an integer value for the button on the gamepad
void send_event(int gamepad_fd, struct input_event gamepad_event, int TYPE, int CODE, int VALUE)
{
    memset(&gamepad_event, 0, sizeof(struct input_event));
    gamepad_event.type = TYPE;
    gamepad_event.code = CODE;
    gamepad_event.value = VALUE;

    if(write(gamepad_fd, &gamepad_event, sizeof(struct input_event)) < 0)
    {
        printf("Error writing event to gamepad!\n");
    }
    else if(verbose)
    {
        printf("-> Gamepad: type %d code %d value %d ", gamepad_event.type, gamepad_event.code, gamepad_event.value);
    }
}

void send_event_and_sync(int gamepad_fd, struct input_event gamepad_event, int TYPE, int CODE, int VALUE)
{
    send_event(gamepad_fd, gamepad_event, TYPE, CODE, VALUE);
    send_sync_event(gamepad_fd, gamepad_event);
}

int main(int argc, char *argv[])
{
    parse_arguments(argc, argv);

    sleep(1);
    int rcode = 0;

    char keyboard_name[256] = "Unknown";
    int keyboard_fd = open(pathKeyboard, O_RDONLY | O_NONBLOCK);
    if (keyboard_fd == -1)
    {
        printf("Failed to open keyboard -> %s\n",pathKeyboard);
        exit(1);
    }
    rcode = ioctl(keyboard_fd, EVIOCGNAME(sizeof(keyboard_name)), keyboard_name);
    printf("Reading from : %s \n", keyboard_name);

    //   printf("Getting exclusive access: ");
    //   rcode = ioctl(keyboard_fd, EVIOCGRAB, 1);
    //   printf("%s\n", (rcode == 0) ? "SUCCESS" : "FAILURE");>>
    struct input_event keyboard_event;

    char mouse_name[256] = "Unknown";
    int mouse_fd = open(pathMouse, O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (mouse_fd == -1)
    {
        printf("Failed to open mouse -> %s\n",pathMouse);
        exit(1);
    }

    rcode = ioctl(mouse_fd, EVIOCGNAME(sizeof(mouse_name)), mouse_name);
    printf("Reading from : %s \n", mouse_name);

    struct input_event mouse_event;

    // printf("Getting exclusive access: ");
    // rcode = ioctl(mouse_fd, EVIOCGRAB, 1);
    // printf("%s\n", (rcode == 0) ? "SUCCESS" : "FAILURE");

    // Now, create gamepad

    int gamepad_fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
    if (gamepad_fd < 0)
    {
        printf("Opening of input failed! \n");
        return 1;
    }

    ioctl(gamepad_fd, UI_SET_EVBIT, EV_KEY); // setting Gamepad keys
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

    ioctl(gamepad_fd, UI_SET_KEYBIT, BTN_THUMBL);
    ioctl(gamepad_fd, UI_SET_KEYBIT, BTN_THUMBR);

    ioctl(gamepad_fd, UI_SET_KEYBIT, BTN_DPAD_UP);
    ioctl(gamepad_fd, UI_SET_KEYBIT, BTN_DPAD_DOWN);
    ioctl(gamepad_fd, UI_SET_KEYBIT, BTN_DPAD_LEFT);
    ioctl(gamepad_fd, UI_SET_KEYBIT, BTN_DPAD_RIGHT);

    ioctl(gamepad_fd, UI_SET_EVBIT, EV_ABS); // setting Gamepad thumbsticks
    ioctl(gamepad_fd, UI_SET_ABSBIT, ABS_X);
    ioctl(gamepad_fd, UI_SET_ABSBIT, ABS_Y);
    ioctl(gamepad_fd, UI_SET_ABSBIT, ABS_RX);
    ioctl(gamepad_fd, UI_SET_ABSBIT, ABS_RY);
    ioctl(gamepad_fd, UI_SET_ABSBIT, ABS_TILT_X);
    ioctl(gamepad_fd, UI_SET_ABSBIT, ABS_TILT_Y);


    struct uinput_user_dev uidev;

    memset(&uidev, 0, sizeof(uidev));
    snprintf(uidev.name, UINPUT_MAX_NAME_SIZE, GAMEPAD_NAME); // Name of Gamepad
    uidev.id.bustype = BUS_USB;
    uidev.id.vendor = 0x3;
    uidev.id.product = 0x3;
    uidev.id.version = 2;
    uidev.absmax[ABS_X] = 32767; // Parameters of thumbsticks
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

    if (write(gamepad_fd, &uidev, sizeof(uidev)) < 0)
    {
        printf("Failed to write! \n");
        return 1;
    }
    if (ioctl(gamepad_fd, UI_DEV_CREATE) < 0)
    {
        printf("Failed to create gamepad! \n");
        return 1;
    }

    sleep(0.6);

    struct input_event gamepad_ev;

    while (1)
    {
        sleep(0.001);

        if (read(keyboard_fd, &keyboard_event, sizeof(keyboard_event)) != -1)
        {
            if(verbose)
            {
                printf("Event: Keyboard: type %d code %d value %d ", keyboard_event.type, keyboard_event.code, keyboard_event.value);
            }

            if (keyboard_event.code == KEY_Q && keyboard_event.value == 1)
            {
                q_pressed = !q_pressed;
            }

            if (keyboard_event.code == KEY_ENTER && keyboard_event.value == 1 && q_pressed == true)
            {
                exitFunc(keyboard_fd, mouse_fd, gamepad_fd);
                break;
            }

            if (keyboard_event.value != 2) // only care about button press and not hold
            {
                switch (keyboard_event.code)
                {
                case KEY_C:
                    send_event_and_sync(gamepad_fd, gamepad_ev, EV_KEY, BTN_Y, keyboard_event.value);
                    break;
                case KEY_X:
                    send_event_and_sync(gamepad_fd, gamepad_ev, EV_KEY, BTN_X, keyboard_event.value);
                    break;
                case KEY_SPACE:
                    send_event_and_sync(gamepad_fd, gamepad_ev, EV_KEY, BTN_A, keyboard_event.value);
                    break;
                case KEY_LEFTALT:
                    send_event_and_sync(gamepad_fd, gamepad_ev, EV_KEY, BTN_B, keyboard_event.value);
                    break;
                case KEY_ENTER:
                    send_event_and_sync(gamepad_fd, gamepad_ev, EV_KEY, BTN_START, keyboard_event.value);
                    break;
                case KEY_TAB:
                    send_event_and_sync(gamepad_fd, gamepad_ev, EV_KEY, BTN_SELECT, keyboard_event.value);
                    break;
                }
            }

            bool pressedOrHold = keyboard_event.value == 2 || keyboard_event.value == 1;
            switch (keyboard_event.code)
            {
            case KEY_W:
                send_event_and_sync(gamepad_fd, gamepad_ev, EV_ABS, ABS_Y, pressedOrHold ? -2000 * absMultiplier : 0);
                break;
            case KEY_S:
                send_event_and_sync(gamepad_fd, gamepad_ev, EV_ABS, ABS_Y, pressedOrHold ? 2000 * absMultiplier : 0);
                break;
            case KEY_A:
                send_event_and_sync(gamepad_fd, gamepad_ev, EV_ABS, ABS_X, pressedOrHold ? -2000 * absMultiplier : 0);
                break;
            case KEY_D:
                send_event_and_sync(gamepad_fd, gamepad_ev, EV_ABS, ABS_X, pressedOrHold ? 2000 * absMultiplier : 0);
                break;
            }

            switch (keyboard_event.code)
            {
            case KEY_LEFTSHIFT:
                send_event_and_sync(gamepad_fd, gamepad_ev, EV_KEY, BTN_TL, keyboard_event.value);
                break;
            case KEY_RIGHTSHIFT:
                send_event_and_sync(gamepad_fd, gamepad_ev, EV_KEY, BTN_TR, keyboard_event.value);
                break;
            case KEY_Q:
                send_event_and_sync(gamepad_fd, gamepad_ev, EV_KEY, BTN_TL2, keyboard_event.value);
                break;
            case KEY_E:
                send_event_and_sync(gamepad_fd, gamepad_ev, EV_KEY, BTN_TR2, keyboard_event.value);
                break;

            case KEY_UP:
                send_event_and_sync(gamepad_fd, gamepad_ev, EV_KEY, BTN_DPAD_UP, keyboard_event.value);
                break;
            case KEY_DOWN:
                send_event_and_sync(gamepad_fd, gamepad_ev, EV_KEY, BTN_DPAD_DOWN, keyboard_event.value);
                break;
            case KEY_LEFT:
                send_event_and_sync(gamepad_fd, gamepad_ev, EV_KEY, BTN_DPAD_LEFT, keyboard_event.value);
                break;
            case KEY_RIGHT:
                send_event_and_sync(gamepad_fd, gamepad_ev, EV_KEY, BTN_DPAD_RIGHT, keyboard_event.value);
                break;

            case KEY_PAGEUP:
                absMultiplier >= 15 ? absMultiplier = 15 : absMultiplier++;
                break;
            case KEY_PAGEDOWN:
                absMultiplier <= 1 ? absMultiplier = 1 : absMultiplier--;
                break;

            // reset view joystick on left control
            case KEY_LEFTCTRL:
            {
                send_event(gamepad_fd, gamepad_ev, EV_ABS, ABS_RX, 0);
                send_event(gamepad_fd, gamepad_ev, EV_ABS, ABS_RY, 0);

                // send one sync event for both axes
                send_sync_event(gamepad_fd, gamepad_ev);
            }
            }

            if(verbose) printf("\n");
        }

        if (read(mouse_fd, &mouse_event, sizeof(struct input_event)) != -1)
        {
            if(verbose)
            {
                printf("Event: Mouse: type %d code %d value %d ", mouse_event.type, mouse_event.code, mouse_event.value);
            }

            switch (mouse_event.type)
            {
            case EV_REL:
                if (mouse_event.code == REL_X)
                {
                    int toWrite = mouse_event.value * 32;
                    if (mouse_event.value > 0 && toWrite > MOUSE_SENSITIVITY) toWrite = MOUSE_SENSITIVITY;
                    if (mouse_event.value < 0 && toWrite < MOUSE_SENSITIVITY_NEGATIVE) toWrite = MOUSE_SENSITIVITY_NEGATIVE;
                    if (mouse_event.value == 0) toWrite = 0;

                    send_event_and_sync(gamepad_fd, gamepad_ev, EV_ABS, ABS_RX, toWrite);
                }
                if (mouse_event.code == REL_Y)
                {
                    int toWrite = mouse_event.value * 32;
                    if (mouse_event.value > 0 && toWrite > MOUSE_SENSITIVITY) toWrite = MOUSE_SENSITIVITY;
                    if (mouse_event.value < 0 && toWrite < MOUSE_SENSITIVITY_NEGATIVE) toWrite = MOUSE_SENSITIVITY_NEGATIVE;
                    if (mouse_event.value == 0) toWrite = 0;

                    send_event_and_sync(gamepad_fd, gamepad_ev, EV_ABS, ABS_RY, toWrite);
                }
                if (mouse_event.code == REL_WHEEL)
                {
                    absMultiplier += mouse_event.value;
                    if (absMultiplier >= 15) absMultiplier = 15;
                    if (absMultiplier <= 1) absMultiplier = 1;
                }
                break;
            case EV_KEY:
                if (mouse_event.code == BTN_LEFT) send_event_and_sync(gamepad_fd, gamepad_ev, EV_KEY, BTN_THUMBL, mouse_event.value);
                if (mouse_event.code == BTN_RIGHT) send_event_and_sync(gamepad_fd, gamepad_ev, EV_KEY, BTN_THUMBR, mouse_event.value);

                // reset controller state
                if (mouse_event.code == BTN_MIDDLE)
                {
                    send_event(gamepad_fd, gamepad_ev, EV_ABS, ABS_RX, 0);
                    send_event(gamepad_fd, gamepad_ev, EV_ABS, ABS_RY, 0);

                    // send one sync event for both axes
                    send_sync_event(gamepad_fd, gamepad_ev);
                }
                break;
            }

            if(verbose) printf("\n");
        }
    }

    printf("Exiting.\n");
    rcode = ioctl(keyboard_fd, EVIOCGRAB, 1);
    close(keyboard_fd);
    rcode = ioctl(mouse_fd, EVIOCGRAB, 1);
    close(mouse_fd);
    return 0;
}
