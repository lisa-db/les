#include <stdio.h>
#include <stdbool.h>
#include "libpynq.h"

int main(void){
int sleep;
 printf("Wait for how many milliseconds? ");
 scanf("%i", &sleep);
 while(sleep < 0){
	printf("The number of milliseconds should be at least 0 ");
	scanf("%i", &sleep);
 }
 
 while(true){
	 led_onoff(0,on);
	 sleep_msec(sleep);
	 led_onoff(0,off);
	 sleep_msec(sleep);
 }
 return 1;
}
