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
 
 int led = 3;
 while(true){
	 led_onoff(led,off);
	 led ++;
	 if(led > 3){
		 led = 0;
	}
	  led_onoff(led,on);
	 sleep_msec(sleep);
 }
 return 1;
}
