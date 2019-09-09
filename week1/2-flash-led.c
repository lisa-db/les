#include <stdio.h>
#include "libpynq.h"

int main(void){
 while(true){
	 led_onoff(0,on);
	 sleep_msec(500);
	 led_onoff(0,off);
	 sleep_msec(500);
 }
 return 1;
}
