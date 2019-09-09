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
 int heen = 1;
 int i = 0;
 while(i < 20){
	 led_onoff(led,off);
	 if (heen%2 == 1){
		 led ++;
	 }
	 else {
		 led --;
	 }
	 if(led > 3){
		 led = 2;
		 heen++;
	}
	if(led < 0){
		led = 1;
		heen++;
	}
	 led_onoff(led,on);
	 sleep_msec(sleep);
	 i ++;
 }
 return 1;
}
