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

int set_interface_attribs (int fd, int speed, int parity);
void set_blocking (int fd, int should_block);
void check_opening(int fd, int ttyUSB);
void create_uidev(int fd, struct uinput_user_dev* uidev, char* name);
uint8_t bitstring_to_key(char* bitstring);
void press_key(int fd, struct input_event* ev, int key);

int main(void) {
  struct uinput_user_dev  uidev;
  struct input_event      ev;

  int                     fd, ttyUSB;
  size_t                  len = 1;
  char                    key[1];

  // /dev/uinput doesn't explicitly say key/mouse/touch
  fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
  ttyUSB = open("/dev/ttyUSB0", O_RDONLY | O_NOCTTY | O_SYNC);
  check_opening(fd, ttyUSB);

  set_interface_attribs (ttyUSB, B9600, 0);  // set speed to 115,200 bps, 8n1 (no parity)
  set_blocking (ttyUSB, 0);                 // set no blocking

  int n = read (fd, buf, sizeof buf);   // read up to 100 characters if ready to read

  create_uidev(fd, &uidev, "keyboard-input-injector");
  sleep(1);

  while( getline(&key, &len, ttyUSB) != -1 ) {
    //key = bitstring_to_key(line);
    //printf("Line: %s", line);
    //printf("Key : %d\n", key);
    //sleep(1);
    press_key(fd, &ev, key_codes[*key]);
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

void check_opening(int fd, int ttyUSB) {
  int i = 0;

  if(fd < 0)
    die("error: open");

  if (ttyUSB < 0)
    die("error: could not open USB0");

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

void press_key(int fd, struct input_event* ev, int key) {
  if (key == 0) return;
  //bool shift = false;

  //if (key >> (sizeof(key)*8 - 1)) {
  //  key -= 128;
  //  shift = true;
  //}

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

int set_interface_attribs (int fd, int speed, int parity) {
  struct termios tty;
  memset (&tty, 0, sizeof tty);
  if (tcgetattr (fd, &tty) != 0) {
    error_message ("error %d from tcgetattr", errno);
    return -1;
  }

  cfsetospeed (&tty, speed);
  cfsetispeed (&tty, speed);

  tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;     // 8-bit chars
  // disable IGNBRK for mismatched speed tests; otherwise receive break
  // as \000 chars
  tty.c_iflag &= ~IGNBRK;         // disable break processing
  tty.c_lflag = 0;                // no signaling chars, no echo,
                                  // no canonical processing
  tty.c_oflag = 0;                // no remapping, no delays
  tty.c_cc[VMIN]  = 0;            // read doesn't block
  tty.c_cc[VTIME] = 5;            // 0.5 seconds read timeout

  tty.c_iflag &= ~(IXON | IXOFF | IXANY); // shut off xon/xoff ctrl

  tty.c_cflag |= (CLOCAL | CREAD);// ignore modem controls,
                                  // enable reading
  tty.c_cflag &= ~(PARENB | PARODD);      // shut off parity
  tty.c_cflag |= parity;
  tty.c_cflag &= ~CSTOPB;
  tty.c_cflag &= ~CRTSCTS;

  if (tcsetattr (fd, TCSANOW, &tty) != 0) {
    error_message ("error %d from tcsetattr", errno);
    return -1;
  }
  return 0;
}

void set_blocking (int fd, int should_block) {
  struct termios tty;
  memset (&tty, 0, sizeof tty);
  if (tcgetattr (fd, &tty) != 0) {
    error_message ("error %d from tggetattr", errno);
    return;
  }

  tty.c_cc[VMIN]  = should_block ? 1 : 0;
  tty.c_cc[VTIME] = 5;            // 0.5 seconds read timeout

  if (tcsetattr (fd, TCSANOW, &tty) != 0)
    error_message ("error %d setting term attributes", errno);
}

