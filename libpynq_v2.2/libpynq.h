#ifndef _LIB_PYNQ_H_

#define LIBPYNQH_VERSION "v2.2"


/*********** library to access buttons ***********/

extern const int nrpushbuttons;
extern const int pushed;	// equal to 1
extern const int released;	// equal to 0
extern int button_states[4];	// static, set by sleep_msec_buttons_pushed

// return the state of the button at this very moment
extern int button_state (int button);
// next two functions return number of milliseconds that we waited
extern int wait_until_button_pushed (int button);
extern int wait_until_button_released (int button);
// sleep for msec milliseconds and return whether the button was pushed during that time
extern int sleep_msec_button_pushed (int msec, int button);
// sleep for msec milliseconds and indicate in button_states if buttons were pushed during that time
extern void sleep_msec_buttons_pushed (int msec);


/*********** library to create display windows ***********/

#include <stdint.h>

typedef struct px {
  unsigned char b;
  unsigned char g;
  unsigned char r;
  unsigned char a;
} pixel;

// user declares pixel array: pixel buf[height][width]
// then create display, update array, and draw to update on screen
void init_display_simple(int height, int width, int scale);
void init_display(int height, int width, int scale, pixel displaybuffer[height][width]);

// call draw_display when you want to update the screen
void draw_display();
void clear_display();
void set_pixel(int y, int x, unsigned char r, unsigned char g, unsigned char b);
int pixel_is_black(int y, int x);


/*********** library to access LEDs ***********/

extern const int nrleds; // monochrome LEDs
extern const int on;
extern const int off;

extern void led_onoff (int led, int onoff);
extern void led_on (int led);
extern void led_off (int led);

extern const int nrcolourleds;
extern const int nrcolours;
extern const char *colours[]; // { "red", "green", "blue" }
extern const int red;    // 0
extern const int green;  // 1
extern const int blue;   // 2

// switch one colour at a time on or off
extern void led_colour (int led, int colour, int onoff);
// colours is RGB coded in bits 0, 1, 2, e.g. 0b111 is white, 0b001 is red
// this allows (un)setting multiple colours in one go, e.g.:
// colours = (redonoff<<red) | (greenonoff<<green) | (blueonoff<<blue)
extern void led_colours (int led, int colours);

extern void sleep_msec (int msec); // delay in milliseconds


/*********** consistency checking ***********/

// check that you're running the right version of PYNQ SD card image and libpynq
extern int check_pynq_version (void);

#define _LIB_PYNQ_H_
#endif
