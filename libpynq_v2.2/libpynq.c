#include "libpynq.h"
#define LIBPYNQC_VERSION "v2.2"

/* Choose implementation: #define PYNQ or VM on ONCOURSE (default) */

#if !defined(PYNQ) && !defined(VM) && !defined(ONCOURSE)
#define ONCOURSE
#endif

/*********** PYNQ emulation in Ubuntu VM ***********/
/*********** PYNQ emulation in Ubuntu VM ***********/
/*********** PYNQ emulation in Ubuntu VM ***********/


#ifdef VM

#include <gtk/gtk.h>
#include <cairo.h>
#include <stdlib.h>
#include <string.h>

// PYNQ board image
#include "pynqz2.h"

static int vm_initialised = 0;

static volatile struct {
  // UI State
  GThread *ui_thread;
  GtkWidget *ui_window;
  GtkWidget *ui_eventbox;
  cairo_surface_t *ui_surface;
  guint update_timeout_handle;
  int running;
  // Led status. (read by UI)
  int led_status[4];
  // Button status. (written by UI)
  int button_status[4];
  int switch_status[2]; // (Written by UI)
  int color_status[2];
} PynqBoardStatus = {
  .ui_eventbox = NULL,
  .running = 0,
};


static volatile int *led_status = PynqBoardStatus.led_status;
static volatile int *color_status = PynqBoardStatus.color_status;
static volatile int *button_status = PynqBoardStatus.button_status;
static volatile int *switch_status = PynqBoardStatus.switch_status;

static gboolean pynq_board_int_draw(G_GNUC_UNUSED GtkWidget *wid, cairo_t *cr, G_GNUC_UNUSED gpointer user_data)
{
  // Paint background.
  cairo_set_source_surface(cr,PynqBoardStatus.ui_surface, 0,0);
  cairo_paint(cr);
  // Red.
  cairo_set_source_rgb ( cr, 1, 0, 0);
  // Draw  LED state.
  if ( PynqBoardStatus.led_status[3] ) {
    cairo_rectangle ( cr, 430+0*46, 378, 15, 7);
    cairo_fill ( cr );
  }
  if ( PynqBoardStatus.led_status[2] ) {
    cairo_rectangle ( cr, 430+1*46, 378, 15, 7);
    cairo_fill ( cr );
  }
  if ( PynqBoardStatus.led_status[1] ) {
    cairo_rectangle ( cr, 430+2*46, 378, 15, 7);
    cairo_fill ( cr );
  }
  if ( PynqBoardStatus.led_status[0] ) {
    cairo_rectangle ( cr, 430+3*42, 378, 15, 7);
    cairo_fill ( cr );
  }
  // Buttons
  if ( PynqBoardStatus.button_status[3] ) {
    cairo_rectangle ( cr, 427, 400, 23, 25);
    cairo_fill ( cr );
  }
  if ( PynqBoardStatus.button_status[2] ) {
    cairo_rectangle ( cr, 470, 400, 23, 25);
    cairo_fill ( cr );
  }
  if ( PynqBoardStatus.button_status[1] ) {
    cairo_rectangle ( cr, 517, 400, 23, 25);
    cairo_fill ( cr );
  }
  if ( PynqBoardStatus.button_status[0] ) {
    cairo_rectangle ( cr, 560, 400, 23, 25);
    cairo_fill ( cr );
  }

  // Switch
  if ( PynqBoardStatus.switch_status[0] ) {
    cairo_set_source_rgb ( cr, 0, 1, 0);
    cairo_rectangle ( cr, 170, 380, 15, 15);
  } else {
    cairo_set_source_rgb ( cr, 1, 0, 0);
    cairo_rectangle ( cr, 170, 400, 15, 15);
  }
  cairo_fill ( cr );
  if ( PynqBoardStatus.switch_status[1] ) {
    cairo_set_source_rgb ( cr, 0, 1, 0);
    cairo_rectangle ( cr, 203, 380, 15, 15);
  } else {
    cairo_set_source_rgb ( cr, 1, 0, 0);
    cairo_rectangle ( cr, 203, 400, 15, 15);
  }

  cairo_fill ( cr );

  // Color leds.
  cairo_set_source_rgb ( cr,
      (color_status[1]&1) == 1,
      (color_status[1]&2) == 2,
      (color_status[1]&4) == 4);
  cairo_arc ( cr, 175, 345, 10, 0, 2*3.14);
  cairo_fill ( cr );
  cairo_set_source_rgb ( cr,
      (color_status[0]&1) == 1,
      (color_status[0]&2) == 2,
      (color_status[0]&4) == 4);
  cairo_arc ( cr, 205, 345, 10, 0, 2*3.14);
  cairo_fill ( cr );


  cairo_set_source_rgb ( cr, 1, 1, 1);
  cairo_arc ( cr, 175, 345, 10, 0, 2*3.14);
  cairo_stroke ( cr );
  cairo_arc ( cr, 205, 345, 10, 0, 2*3.14);
  cairo_stroke ( cr );


  return FALSE;
}

static gboolean pynq_board_int_delete_event ( G_GNUC_UNUSED gpointer user_data )
{
  //fprintf(stderr,"PynqBoard U I: Close requested.\n");
  g_source_remove ( PynqBoardStatus.update_timeout_handle );

  PynqBoardStatus.running = FALSE;
  return FALSE;
}

static void pynq_board_calculate_click ( int x, int y, int press )
{
  if ( !press && x > 25 && x < (25+55) && y > 312 && y < (312+35) ){
    fprintf(stderr,"libpynq GUI: switched off board, exiting\n");
    PynqBoardStatus.running = FALSE;
  }
  if ( press && y > 400 && y < 425 )
  {
    if ( x > 427 && x < 450 ) {
      PynqBoardStatus.button_status[3] = press;
    } else if ( x > 470 && x < 493 ) {
      PynqBoardStatus.button_status[2] = press;
    } else if ( x > 517 && x < 540 ) {
      PynqBoardStatus.button_status[1] = press;
    } else if ( x > 560 && x < 583 ) {
      PynqBoardStatus.button_status[0] = press;
    }
  } else {
    PynqBoardStatus.button_status[0] = 0;
    PynqBoardStatus.button_status[1] = 0;
    PynqBoardStatus.button_status[2] = 0;
    PynqBoardStatus.button_status[3] = 0;
  }
  if ( !press && y > 370 && y < 425 ){
    if ( x > 160 && x < 195) {
      PynqBoardStatus.switch_status[0] = ! PynqBoardStatus.switch_status[0];
    } else if ( x > 195 && x < 230 ) {
      PynqBoardStatus.switch_status[1] = ! PynqBoardStatus.switch_status[1];
    }
  }
}

static gboolean pynq_board_int_button_release_event ( G_GNUC_UNUSED GtkWidget *widget, GdkEvent *event, G_GNUC_UNUSED gpointer user_data )
{
  GdkEventButton *eb = (GdkEventButton*)event;
  pynq_board_calculate_click ( eb->x, eb->y, FALSE );
  gtk_widget_queue_draw ( PynqBoardStatus.ui_eventbox );

  return G_SOURCE_CONTINUE;
}

static gboolean pynq_board_int_button_press_event ( G_GNUC_UNUSED GtkWidget *widget, GdkEvent *event, G_GNUC_UNUSED gpointer user_data )
{
  GdkEventButton *eb = (GdkEventButton*)event;

  pynq_board_calculate_click ( eb->x, eb->y, TRUE );
  gtk_widget_queue_draw ( PynqBoardStatus.ui_eventbox );    
  return G_SOURCE_CONTINUE;
}

static void pynq_board_calculate_key ( GdkEventKey *event )
{
  if ( event->type == GDK_KEY_PRESS ) {
    if ( event->keyval == GDK_KEY_h) {
      PynqBoardStatus.button_status[3] = 1;
    }
    if ( event->keyval == GDK_KEY_j) {
      PynqBoardStatus.button_status[2] = 1;
    }
    if ( event->keyval == GDK_KEY_k) {
      PynqBoardStatus.button_status[1] = 1;
    }
    if ( event->keyval == GDK_KEY_l) {
      PynqBoardStatus.button_status[0] = 1;
    }
  } else if ( event->type == GDK_KEY_RELEASE ) {
    if ( event->keyval == GDK_KEY_q ) {
      PynqBoardStatus.running = FALSE;
    }
    if ( event->keyval == GDK_KEY_w ) {
      PynqBoardStatus.switch_status[0] = !PynqBoardStatus.switch_status[0];
    }
    if ( event->keyval == GDK_KEY_e ) {
      PynqBoardStatus.switch_status[1] = !PynqBoardStatus.switch_status[1];
    }
    if ( event->keyval == GDK_KEY_h) {
      PynqBoardStatus.button_status[3] = 0;
    }
    if ( event->keyval == GDK_KEY_j) {
      PynqBoardStatus.button_status[2] = 0;
    }
    if ( event->keyval == GDK_KEY_k) {
      PynqBoardStatus.button_status[1] = 0;
    }
    if ( event->keyval == GDK_KEY_l) {
      PynqBoardStatus.button_status[0] = 0;
    }
  }
}
static gboolean pynq_board_int_key_release_event ( G_GNUC_UNUSED GtkWidget *widget, GdkEvent *event, G_GNUC_UNUSED gpointer user_data )
{
  GdkEventKey *eb = (GdkEventKey *)event;
  pynq_board_calculate_key ( eb );
  gtk_widget_queue_draw ( PynqBoardStatus.ui_eventbox );
  return G_SOURCE_CONTINUE;
}

static gboolean pynq_board_int_key_press_event ( G_GNUC_UNUSED GtkWidget *widget, GdkEvent *event, G_GNUC_UNUSED gpointer user_data )
{
  GdkEventKey *eb = (GdkEventKey *)event;
  pynq_board_calculate_key ( eb );
  gtk_widget_queue_draw ( PynqBoardStatus.ui_eventbox );    
  return G_SOURCE_CONTINUE;
}


static cairo_status_t pynq_board_int_cairo_read_func ( G_GNUC_UNUSED void *closure, unsigned char *data, unsigned int length )
{
  static int start = 0;
  memcpy ( data, &(pynqz2_png[start]),length);
  start+= length;
  return CAIRO_STATUS_SUCCESS;
}

static gpointer pynq_board_int_init ( G_GNUC_UNUSED gpointer data )
{
  // Clear everything.
  //fprintf(stderr,"PynqBoard UI: Initialize\n");
  gtk_init ( NULL, NULL );

  PynqBoardStatus.ui_window = gtk_window_new ( GTK_WINDOW_TOPLEVEL );
  gtk_window_set_title(GTK_WINDOW(PynqBoardStatus.ui_window), "Pynqboard UI emulation");

  gtk_window_resize ( GTK_WINDOW(PynqBoardStatus.ui_window), 640, 480 );

  PynqBoardStatus.ui_eventbox = gtk_event_box_new ();
  // Set the widget to be paintable.
  gtk_widget_set_app_paintable ( PynqBoardStatus.ui_eventbox, TRUE );

  PynqBoardStatus.ui_surface = cairo_image_surface_create_from_png_stream ( pynq_board_int_cairo_read_func, NULL );

  // Event handler: Window content needs to be drawn.
  g_signal_connect ( G_OBJECT ( PynqBoardStatus.ui_eventbox ), "draw",
      G_CALLBACK( pynq_board_int_draw), NULL);

  // Event hander: Closing of the window.
  g_signal_connect ( G_OBJECT ( PynqBoardStatus.ui_window ), "delete-event",
      G_CALLBACK( pynq_board_int_delete_event ), NULL);

  // Event handler: Mouse button press event.
  g_signal_connect ( G_OBJECT ( PynqBoardStatus.ui_window ), "key-press-event",
      G_CALLBACK ( pynq_board_int_key_press_event ) , NULL );
  g_signal_connect ( G_OBJECT ( PynqBoardStatus.ui_window ), "key-release-event",
      G_CALLBACK ( pynq_board_int_key_release_event ) , NULL );
  g_signal_connect ( G_OBJECT ( PynqBoardStatus.ui_window ), "button-press-event",
      G_CALLBACK ( pynq_board_int_button_press_event ), NULL);

  g_signal_connect ( G_OBJECT ( PynqBoardStatus.ui_window ), "button-release-event",
      G_CALLBACK ( pynq_board_int_button_release_event ), NULL);

  // Add the eventbox to the window.
  gtk_container_add ( GTK_CONTAINER ( PynqBoardStatus.ui_window ), PynqBoardStatus.ui_eventbox );

  //PynqBoardStatus.update_timeout_handle = g_timeout_add ( 1000/30, pynq_board_int_timeout, NULL );
  gtk_widget_queue_draw ( PynqBoardStatus.ui_eventbox );
  gtk_widget_show_all ( PynqBoardStatus.ui_window );
  gtk_main ( );
  return 0;
}

static void pynq_board_int_atexit ( void )
{
  fprintf(stderr,"libpynq GUI: closed window, exiting\n");
  gtk_main_quit();
}

void pynq_board_init ( void )
{
  memset ( (struct PynqBoard* )&PynqBoardStatus, 0, sizeof (PynqBoardStatus)  );
  PynqBoardStatus.running = 1;
  PynqBoardStatus.ui_thread = g_thread_new ( "board-ui", pynq_board_int_init, NULL );
  atexit(pynq_board_int_atexit);
  vm_initialised = 1;
}

int pynq_board_running ( void )
{
  if (!vm_initialised) pynq_board_init();
  return (vm_initialised && PynqBoardStatus.running);
}

void pynq_board_update ( void )
{
  if ( PynqBoardStatus.ui_eventbox == NULL ) return ;
  if (pynq_board_running()) gtk_widget_queue_draw ( PynqBoardStatus.ui_eventbox );
}

#endif


/*********** library to create display windows ***********/
/*********** library to create display windows ***********/
/*********** library to create display windows ***********/

#ifdef ONCOURSE

void init_display(int height, int width, int scale, pixel displaybuffer[height][width]) {}
void draw_display() {}
void clear_display() {}
void set_pixel(int y, int x, unsigned char r, unsigned char g, unsigned char b) {}
int pixel_is_black(int y, int x) { return 1; }

#else

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <stdint.h>
#include <X11/Xlib.h>

static int display_width = -1;
static int display_height = -1;
static int display_scale = -1;
static pixel *display_buffer; // alloc'd by user
static pthread_t display_thread;
static int must_close;
static int successful_start;
static Window window;
static Display *display;
static XImage image;
static int screen;

// Nearest neighbour image scaling
static void resize_ximage(XImage *dst, XImage *src)
{
  uint32_t *src_ptr = (uint32_t *)src->data;
  uint32_t *dst_ptr = (uint32_t *)dst->data;
  for(int y = 0; y < dst->height; y++)
  {
    int src_y = y * src->height / dst->height;
    for(int x = 0; x < dst->width; x++)
    {
      int src_x = x * src->width / dst->width;
      dst_ptr[y * dst->width + x] = src_ptr[src_y * src->width + src_x];
    }
  }
}

static void display_draw_callback(XEvent *event)
{
  unsigned int width, height, border_width, depth;
  int window_x, window_y;
  Window root_window;
  XGetGeometry(display, window, &root_window, &window_x, &window_y, 
      &width, &height, &border_width, &depth);

  // Create new image
  XImage display_image = {
    .width = width,
    .height = height,
    .format = ZPixmap,
    .data = malloc(width * height * 4),
    .byte_order = LSBFirst,
    .bitmap_unit = 32,
    .bitmap_bit_order = LSBFirst,
    .bitmap_pad = 32,
    .depth = 24,
    .bytes_per_line = width * 4,
    .bits_per_pixel = 32,
    .red_mask = 0xFF000000,
    .green_mask = 0x00FF0000,
    .blue_mask = 0x0000ff00
  };
  if(!XInitImage(&display_image)) {
    fprintf(stderr, "libpynq: graphics window: could not initialize XImage\n");
    free(display_image.data);
    must_close = 1;
    return;
  }
  resize_ximage(&display_image, &image);

  GC gc = XCreateGC(display, window, 0, NULL);
  XPutImage(display, window, gc, &display_image, 0, 0, 0, 0, 
      display_image.width, display_image.height);
  XFreeGC(display, gc);
  free(display_image.data);
}

static void *display_thread_func(void *params)
{
  XEvent event;
  int x11_fd;
  fd_set in_fds;
  struct timeval tv;

  if(!XInitThreads()) {
    fprintf(stderr, "libpynq: graphics windows: warning: could not initialize xlib multithreaded\n");
  }

  display = XOpenDisplay(NULL);
  if(display==NULL) {
    fprintf(stderr, "libpynq: graphics windows: could not open display\n");
    successful_start = -1;
    return NULL;
  }

  // Create the XImage
  image.width = display_width;
  image.height = display_height;
  image.format = ZPixmap;
  image.data = malloc(image.width * image.height * 4);
  memset(image.data, 0, image.width * image.height * 4);
  image.byte_order = LSBFirst;
  image.bitmap_unit = 32;
  image.bitmap_bit_order = LSBFirst;
  image.bitmap_pad = 32;
  image.depth = 24;
  image.bytes_per_line = image.width * 4;
  image.bits_per_pixel = 32;
  image.red_mask =   0xFF000000;
  image.green_mask = 0x00FF0000;
  image.blue_mask =  0x0000FF00;
  if(!XInitImage(&image)) {
    fprintf(stderr, "libpynq: graphics window: could not initialize XImage\n");
    successful_start = -1;
    return NULL;
  }

  screen = DefaultScreen(display);
  window = XCreateSimpleWindow(display, RootWindow(display, screen), 
      10, 10, display_scale *display_width, display_scale *display_height, 1, BlackPixel(display, screen), WhitePixel(display, screen));

  // Process the delete window in the event loop
  Atom delWindow = XInternAtom(display, "WM_DELETE_WINDOW", 0);
  XSetWMProtocols(display, window, &delWindow, 1);

  // Select the kind of events we are interested in
  XSelectInput(display, window, ExposureMask | KeyPressMask);

  // Show the window
  XMapWindow(display, window);
  XFlush(display);

  // Get the socket
  x11_fd = ConnectionNumber(display);

  successful_start = 1;
  while(must_close == 0) {
    FD_ZERO(&in_fds);
    FD_SET(x11_fd, &in_fds);

    tv.tv_usec = 0;
    tv.tv_sec = 1;

    if(select(x11_fd+1, &in_fds, 0, 0, &tv)) {
      // Event received
      XLockDisplay(display);
      while(XPending(display) && must_close == 0) {
	XNextEvent(display, &event);
	switch(event.type) {
	  case Expose:
	    display_draw_callback(&event);
	    break;
	  case ClientMessage:
	    if((Atom)event.xclient.data.l[0] == delWindow) {
	      must_close = 1;
	      break;
	    }
	    break;
	}
      }
      XUnlockDisplay(display);
    }
  }
  must_close = 1; 
  XDestroyWindow(display, window); 
  XCloseDisplay(display); 
  return NULL;
}

static void display_cleanup()
{
  must_close = 1; 
  pthread_join(display_thread, NULL); 
  free(image.data);
}

void init_display(int height, int width, int scale, pixel displaybuffer[height][width])
{
  check_pynq_version();
  if(width <= 0) {
    fprintf(stderr, "libpynq: graphics display width must be positive\n");
    exit(1);
  }
  if(height <= 0) {
    fprintf(stderr, "libpynq: graphics display height must be positive\n");
    exit(1);
  }
  if(scale <= 0) {
    fprintf(stderr, "libpynq: graphics display scale must be positive\n");
    exit(1);
  }
  display_width = width;
  display_height = height;
  display_scale = scale;
  display_buffer = (pixel *)displaybuffer;
  clear_display();
  must_close = 0;
  successful_start = 0;

  pthread_create(&display_thread, NULL, display_thread_func, NULL);

  // Wait until thread has started or failed
  //fprintf(stderr, "libpynq: Graphics window opening\n");
  while(successful_start == 0);
  if(successful_start < 0) {
    // Failed
    pthread_join(display_thread, NULL);
    exit(1);
  }

  // Don't let students do the cleanup themselves. Register an atexit function.
  atexit(display_cleanup);
  //fprintf(stderr, "libpynq: Graphics window opened\n");
}

void update_display()
{
  if(must_close == 1) {
    fprintf(stderr, "libpynq: Graphics window closed\n");
    exit(1);
  }

  XExposeEvent event;
  event.type = Expose;
  event.window = window;
  event.display = display;
  event.send_event = 1;
  event.x = 0;
  event.y = 0;
  event.width = display_width*display_scale;
  event.height = display_height*display_scale;
  event.count = 0;

  XLockDisplay(display);
  if(!XSendEvent(display, window, 1, ExposureMask, (XEvent *)&event)) {
    fprintf(stderr, "libpynq: Graphics window: could not send redraw event\n");
  }
  XFlush(display);
  XUnlockDisplay(display);
}

void draw_display () {
  memcpy(image.data, display_buffer, display_width * display_height * sizeof(pixel));
  update_display();
#if VM
  pynq_board_update();
#endif
}

void clear_display()
{
  pixel black;
  black.r = black.g = black.b = 0;
  for (int i=0; i < display_height*display_width; i++) display_buffer[i]=black;
}

void set_pixel(int y, int x, unsigned char r, unsigned char g, unsigned char b)
{
  if (x < 0 || x >= display_width || y < 0 || y >= display_height || r < 0 || g < 0 || b < 0)
    fprintf(stderr,"libpynq: set_pixel(h=%d,w=%d,r=%d,g=%d,b=%d) argument out of range height=%d width=%d\n",
	y, x, r, g, b, display_height, display_width);
  else {
    pixel pxl = {b,g,r,0};
    display_buffer[y*display_width+x] = pxl;
  }
}

int pixel_is_black(int y, int x) 
{
  return (
      display_buffer[display_width*y+x].r == 0 &&
      display_buffer[display_width*y+x].g == 0 &&
      display_buffer[display_width*y+x].b == 0);
}

#endif



/*********** library to access LEDs ***********/
/*********** library to access LEDs ***********/
/*********** library to access LEDs ***********/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

const int nrleds = 4; // monochrome LEDs
const int on = 1;
const int off = 0;
const int nrcolourleds = 2;
const int nrcolourleds_pwr = 7; // internal use only
const int nrcolours = 3;
const char *colours[] = { "red", "green", "blue" };
const int red = 0;
const int green = 1;
const int blue = 2;

#ifdef PYNQ 

inline void sleep_msec (int msec) { if (msec >= 0) usleep ((unsigned) msec*1000); }

static int initialised = 0;
// open file pointers to all LEDs and keep them open
static FILE *ledfp[4+2*3] = {};
static void leds_initialise (void) {
  check_pynq_version();

  // monochrome LEDs
  const char file[] = "/sys/class/leds/LED%d/brightness";
  char filename[128];
  if (nrleds>4) {
    fprintf(stderr,"libpynq: leds_initialise: nrleds=%d+nrcolourleds=%d*nrcolours=%d>10\n",
	nrleds,nrcolourleds,nrcolours);
    exit(1);
  }
  for (int i=0; i < nrleds; i++) {
    sprintf(filename,file,i);
    ledfp[i] = fopen (filename,"w");
    if (!ledfp[i]) { fprintf(stderr,"libpynq: failed to initialise LED %d\n",i); exit(1); }
    fputc('0',ledfp[i]); // switch LED off
    fflush(ledfp[i]);
  }

  // colour LEDs, numbered from 0 for user but from nrleds in /sys/class/leds
  const char cfile[] = "/sys/class/leds/LED%d-%s/brightness";
  for (int i=0; i < nrcolourleds; i++) {
    for (int c=0; c < nrcolours; c++) {
      const int led = nrleds + i*nrcolours + c;
      sprintf(filename,cfile,nrleds+i,colours[c]);
      ledfp[led] = fopen (filename,"w");
      if (!ledfp[led]) {
	fprintf(stderr,"libpynq: failed to initialise colour LED %d-%s\n",i,colours[c]);
	exit(1);
      }
      fputc('0',ledfp[led]); // switch LED off
      fflush(ledfp[led]);
    }
  }
  initialised = 1;
}

// monochrome LEDs
void led_onoff (int led, int onoff) { 
  if (!initialised) leds_initialise();
  if (led < 0 || led > nrleds-1) return;
  fputc('0'+(onoff != 0),ledfp[led]);
  fflush(ledfp[led]);
}

// colour LEDs
void led_colour (int led, int colour, int onoff) {
  if (!initialised) leds_initialise();
  if (led < 0 || led > nrcolourleds-1) return;
  if (colour < 0 || colour > nrcolours-1) return;
  if (onoff < 0 || onoff > 1) return;
  const int l = nrleds + led*nrcolours + colour;
  fputc('0'+(onoff != 0),ledfp[l]);
  fflush(ledfp[l]);
}

// colours is RGB coded in bits 0, 1, 2, e.g. 0b111 is white, 0b001 is red
// this allows (un)setting multiple colours in one go, e.g.:
// colours = (redonoff<<red) | (greenonoff<<green) | (blueonoff<<blue)
void led_colours (int led, int clours) {
  if (led < 0 || led > nrcolourleds-1) return;
  if (clours < 0 || clours > nrcolourleds_pwr) return;
  const int l = nrleds + led*nrcolours;
  for (int c=red; c <= blue; c++) {
    fputc('0'+ (clours & 1),ledfp[l+c]);
    fflush(ledfp[l+c]);
    //printf("colour LED %d-%s is %s\n",led,colours[c],(clours & 1 ? "on" : "off"));
    clours = clours >> 1;
  }
}

#endif
#ifdef VM

inline void sleep_msec (int msec) { if (msec >= 0) usleep ((unsigned) msec*1000); }

// monochrome LEDs
void led_onoff (int led, int onoff) { 
  if (!pynq_board_running()) exit(0);
  if (led < 0 || led > nrleds-1) return;
  led_status[led] = (onoff != 0);
  pynq_board_update();
}

// switch one colour at a time on or off
void led_colour (int led, int colour, int onoff) {
  if (!pynq_board_running()) exit(0);
  if (led < 0 || led > nrcolourleds-1) return;
  if (colour < 0 || colour > nrcolours-1) return;
  if (onoff < 0 || onoff > 1) return;
  if (onoff) color_status[led] |= 1 << colour;
  else color_status[led] &= ~(1 << colour);
  pynq_board_update();
}

// colours is RGB coded in bits 0, 1, 2, e.g. 0b111 is white, 0b001 is red
// this allows (un)setting multiple colours in one go, e.g.:
// colours = (redonoff<<red) | (greenonoff<<green) | (blueonoff<<blue)
void led_colours (int led, int colours) {
  if (!pynq_board_running()) exit(0);
  if (led < 0 || led > nrcolourleds-1) return;
  if (colours < 0 || colours > nrcolourleds_pwr) return;
  color_status[led] = colours;
  pynq_board_update();
}

#endif

#ifdef ONCOURSE

void led_onoff (int led, int onoff) {}
void led_colour (int led, int colour, int onoff) {}
void led_colours (int led, int clours) {}
inline void sleep_msec (int msec) {}

#endif

inline void led_on (int led) { led_onoff (led,on); }
inline void led_off (int led) { led_onoff (led,off); }


/*********** library to access buttons ***********/
/*********** library to access buttons ***********/
/*********** library to access buttons ***********/

const int nrpushbuttons = 4;
const int pushed = 1;
const int released = 0;
int button_states[4];

#ifdef ONCOURSE

int button_state (int button) { return released; }
int wait_until_button_pushed (int button) { return 0; }
int wait_until_button_released (int button) { return 0; }
int sleep_msec_button_pushed (int msec, int button) { return 0; }
void sleep_msec_buttons_pushed (int msec) {}

#else

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/time.h>

int button_state (int button) {
  if (button < 0 || button > nrpushbuttons-1) return released;
  // do this every time; keeping the FILE open doesn't work reliably
#ifdef PYNQ
  check_pynq_version();
  char *ge = getenv("GPIO_BTN_BASE");
  if (!ge) { fprintf(stderr,"libpynq: error: cannot initialise buttons\n"); exit(1); }
  int gpiooffset = atoi(ge);
  const char file[] = "/sys/class/gpio/gpio%d/value";
  char filename[128];
  sprintf (filename,file,gpiooffset+button);
  FILE *buttonfp = fopen (filename,"r");
  if (!buttonfp) { fprintf(stderr,"libpynq: error: cannot initialise buttons %s\n",filename); exit(1); }
  int s = fgetc (buttonfp);
  fclose (buttonfp);
  return (s == '1' ? pushed : released);
#endif
#ifdef VM
  if (!pynq_board_running()) exit(0);
  return button_status[button];
#endif
}

// return number of milliseconds waited
int wait_until_button_state (int button, int state) {
  struct timeval then, now;
  long seconds, useconds;    
  (void) gettimeofday (&then, NULL);
  while (button_state (button) != state) ;
  (void) gettimeofday (&now, NULL);
  seconds  = now.tv_sec  - then.tv_sec;
  useconds = now.tv_usec - then.tv_usec;
  return seconds*1e3 + useconds/1000;
}

inline int wait_until_button_pushed (int button) {
  return wait_until_button_state (button, pushed);
}

inline int wait_until_button_released (int button) { 
  return wait_until_button_state (button, released);
}

int sleep_msec_button_pushed (int msec, int button) {
  int state = released; 
  struct timeval then, now;
  long seconds, useconds;    
  (void) gettimeofday (&then, NULL);
  do {
    (void) gettimeofday (&now, NULL);
    seconds  = now.tv_sec  - then.tv_sec;
    useconds = now.tv_usec - then.tv_usec;
    if (state != pushed) state = button_state (button);
  } while (seconds*1e3+useconds/1000 < msec);
  return state;
}

void sleep_msec_buttons_pushed (int msec) {
  struct timeval then, now;
  long seconds, useconds;    
  for (int b=0; b < nrpushbuttons; b++) button_states[b] = 0;
  (void) gettimeofday (&then, NULL);
  do {
    (void) gettimeofday (&now, NULL);
    seconds  = now.tv_sec  - then.tv_sec;
    useconds = now.tv_usec - then.tv_usec;
    for (int b=0; b < nrpushbuttons; b++) 
      button_states[b] += button_state (b);
  } while (seconds*1e3+useconds/1000 < msec);
}

#endif


/*********** consistency checking ***********/

// check that you're running the right version of PYNQ SD card image and libpynq
#ifdef PYNQ
int check_pynq_version (void)
{
  // SD card image:
  FILE *fp = fopen ("/etc/netplan/default.yaml","r");
  // error, ignore
  if (!fp) return 0;
  char line[100];
  if (fscanf(fp,"%8s",line) == 0) {
    // empty file -> v2.0.0
    fclose(fp);
    return 1;
  } else if (!strncmp(line,"network:",8)) {
    fprintf(stderr,"ERROR: you are using the wrong PYNQ SD card image v1.0.0 of 2018/19\n"); 
    fprintf(stderr,"ERROR: flash your SD card with the PYNQ SD card image v2.0.0 of 2019/20\n"); 
    //exit(1);
    fclose(fp);
    return 0;
  }
  else {
    fclose(fp);
    return 0; // error, ignore
  }
}
#else
int check_pynq_version (void) { return 1; }
#endif
