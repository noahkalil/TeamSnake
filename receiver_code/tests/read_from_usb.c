/* The original code is from: 
 *     http://git.nosuchfile.org/?p=uinput-sample.git;a=blob_plain;f=uinput-sample.c;hb=HEAD
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <linux/input.h>
#include <linux/uinput.h>

#define die(str, args...) do { \
        perror(str); \
        exit(EXIT_FAILURE); \
    } while(0)

void check_opening(int fd);
int line_to_key(char* line);
void press_key(int fd, struct input_event* ev, int key);

int main(void) {
  struct uinput_user_dev  uidev;
  struct input_event      ev;

  int                     fd, usbTTY, key;
  size_t                  len;
  char*                   line = NULL;

  //usbTTY = fopen("/dev/ttyUSB0", "r");
  //if ( ttyUSB < 0)
  //  die("error: could not open USB0");

  // /dev/uinput doesn't explicitly say key/mouse/touch
  fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
  check_opening(fd);

  memset(&uidev, 0, sizeof(uidev)); // clear mem here
  snprintf(uidev.name, UINPUT_MAX_NAME_SIZE, "keyboard-input-sample");
  uidev.id.bustype = BUS_USB; // BUS_ISA, BUS_PCI are tested OK
  uidev.id.vendor  = 0x1;
  uidev.id.product = 0x1;
  uidev.id.version = 1;

  if(write(fd, &uidev, sizeof(uidev)) < 0)
    die("error: write");

  if(ioctl(fd, UI_DEV_CREATE) < 0)
    die("error: ioctl");

  while( getline(&line, &len, stdin) != -1 ) {
    key = line_to_key(line);
    printf("Key: %d\n", key);
    //press_key(fd, &ev, key);
    //sleep(1);
  }

  sleep(1);
  if(ioctl(fd, UI_DEV_DESTROY) < 0)
    die("error: ioctl");

  close(fd);
  //close(ttyUSB);
  if (line)
    free(line);
  return 0;
}

void check_opening(int fd) {
  if(fd < 0)
    die("error: open");

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
}

int line_to_key(char* line) {
  int code = (int) strtol(line, NULL, 2);
  return key_codes[code];
}

void press_key(int fd, struct input_event* ev, int key) {
  if (key == 0) return;

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

  memset(ev, 0, sizeof(struct input_event));
  ev->type = EV_SYN;
  ev->code = SYN_REPORT;
  ev->value = 0;
  if(write(fd, ev, sizeof(struct input_event)) < 0)
    die("error: write EV_SYN");
}
