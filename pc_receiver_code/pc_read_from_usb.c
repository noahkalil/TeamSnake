/* The original code is from: 
 *     http://git.nosuchfile.org/?p=uinput-sample.git;a=blob_plain;f=uinput-sample.c;hb=HEAD
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <linux/input.h>
#include <linux/uinput.h>
#include "keycodes.h"

#define die(str, args...) do { \
        perror(str); \
        exit(EXIT_FAILURE); \
    } while(0)

void check_opening(int fd, FILE* ttyUSB);
void create_uidev(int fd, struct uinput_user_dev* uidev, char* name);
uint8_t bitstring_to_key(char* bitstring);
void press_key(int fd, struct input_event* ev, int key);
uint8_t code2key(int code);

int main(int argc, char** argv) {
  struct uinput_user_dev  uidev;
  struct input_event      ev;

  int                     fd, key;
  size_t                  len;
  char*                   line = NULL;
  FILE*                   ttyUSB;

  // /dev/uinput doesn't explicitly say key/mouse/touch
  fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
  printf("Opening %s\n", argv[1]);
  ttyUSB = fopen(argv[1], "r");
  check_opening(fd, ttyUSB);

  create_uidev(fd, &uidev, "keyboard-input-injector");
  sleep(1);

  while( getline(&line, &len, ttyUSB) != -1 ) {
    key = atoi(line);
    printf("Code: %d\n", key);
    //key = code2key( key );
    key = 0x52u - key;
    printf("Key : %d\n", key);
    press_key(fd, &ev, key_codes[key]);
  }

  sleep(1);
  if(ioctl(fd, UI_DEV_DESTROY) < 0)
    die("error: ioctl");

  close(fd);
  fclose(ttyUSB);
  if (line)
    free(line);
  return 0;
}

void check_opening(int fd, FILE* ttyUSB) {
  int i = 0;

  if(fd < 0)
    die("error: open");

  if (ttyUSB == NULL)
    die("error: could not open tty");

  // says key
  if(ioctl(fd, UI_SET_EVBIT, EV_KEY) < 0)
    die("error: ioctl EV_KEY");

  // says mouse
  if(ioctl(fd, UI_SET_KEYBIT, BTN_LEFT) < 0)
    die("error: ioctl BTN_LEFT");

  if(ioctl(fd, UI_SET_EVBIT, EV_REL) < 0)
    die("error: ioctl EV_REL");

  if(ioctl(fd, UI_SET_RELBIT, REL_X) < 0)
    die("error: ioctl REL_X");

  if(ioctl(fd, UI_SET_RELBIT, REL_Y) < 0)
    die("error: ioctl REL_Y");

  for (i = 4; i < 101; ++i) {
    if(ioctl(fd, UI_SET_KEYBIT, key_codes[i]) < 0)
      die("error: ioctl BTN_LEFT");
  }
}

void create_uidev(int fd, struct uinput_user_dev* uidev, char* name) {
  memset(uidev, 0, sizeof(*uidev)); // clear mem here
  snprintf(uidev->name, UINPUT_MAX_NAME_SIZE, name);
  uidev->id.bustype = BUS_USB; // BUS_ISA, BUS_PCI are tested OK
  uidev->id.vendor  = 0x1;
  uidev->id.product = 0x1;
  uidev->id.version = 1;

  if(write(fd, uidev, sizeof(*uidev)) < 0)
    die("error: write");

  if(ioctl(fd, UI_DEV_CREATE) < 0)
    die("error: ioctl");
}

uint8_t bitstring_to_key(char* bitstring) {
  uint8_t code = (uint8_t) strtol(bitstring, NULL, 2);
  //if (code >> (sizeof(code)*8 - 1)) // if shift was sent
  //  return shift_key_codes[code-128];
  return key_codes[code];
}

uint8_t code2key(int code) {
  // aka 0x52 - code;

  if (code == 0) // up
    return 0x52;

  else if (code == 1) // down 
    return 0x51;

  else if (code == 2) // left
    return 0x50;

  else if (code == 3) // right
    return 0x4F;

  return 0;
}

void press_key(int fd, struct input_event* ev, int key) {
  if (key == 0) return;

  printf("Pressing %d\n", key);
  // press key
  memset(ev, 0, sizeof(struct input_event));
  ev->type = EV_KEY;
  ev->code = key;
  ev->value = 1;
  if(write(fd, ev, sizeof(struct input_event)) < 0)
    die("error: write REL_ENTER");

  // sync press
  memset(ev, 0, sizeof(struct input_event));
  ev->type = EV_SYN;
  ev->code = SYN_REPORT;
  ev->value = 0;
  if(write(fd, ev, sizeof(struct input_event)) < 0)
    die("error: write EV_SYN");

  // release key
  memset(ev, 0, sizeof(struct input_event));
  ev->type = EV_KEY;
  ev->code = key;
  ev->value = 0;
  if(write(fd, ev, sizeof(struct input_event)) < 0)
    die("error: write REL_ENTER");

  // sync release
  memset(ev, 0, sizeof(struct input_event));
  ev->type = EV_SYN;
  ev->code = SYN_REPORT;
  ev->value = 0;
  if(write(fd, ev, sizeof(struct input_event)) < 0)
    die("error: write EV_SYN");
}
