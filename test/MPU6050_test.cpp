
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 

#include <time.h>
#include <signal.h>

#include "math.h"
#include "MPU6050.h"

MPU6050 mpu;

void loop()
{
	float gx=0,gy=0,gz=0;
	float rx=0,ry=0,rz=0;
	float temp = 0.0;
	
	while(1)
	{
		mpu.readAccel(&gx,&gy,&gz);
		mpu.readRot(&rx,&ry,&rz);
		temp = mpu.readTemp();
		
		printf("G: [%.2f, %.2f, %.2f] [%.2f] R: [%.2f, %.2f, %.2f] T: %.2f\n",gx,gy,gz,(float)sqrt(gx*gx+gy*gy+gz*gz),rx,ry,rz,temp);
		usleep(50 * 1000);
	}
}

int main()
{
	mpu.init();
	loop();
	return 0;
}