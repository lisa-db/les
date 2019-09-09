#include <stdio.h>
#include "libpynq.h"

int main(void){
 led_onoff(0,on);
 sleep_msec(500);
 led_onoff(0,off);
 return 1;
}
