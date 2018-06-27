
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 

#include <time.h>
#include <signal.h>

#include <wiringPi.h>

#include "math.h"
#include "MPU6050.h"

MPU6050 mpu;

enum {
	HEATING_ENABLE_PIN = 17,
};

const bool HEATING_ACTIVE_LOW = true;

void setHeating(bool enable)
{
	digitalWrite(HEATING_ENABLE_PIN,(enable^HEATING_ACTIVE_LOW)?(1):(0));
	printf("*** HEATING SWITCHED %s ***\n",(enable)?("ON"):("OFF"));
}

bool isHeating()
{
	return HEATING_ACTIVE_LOW ^ (0!=digitalRead(HEATING_ENABLE_PIN));
}

void loop()
{
	float gx=0,gy=0,gz=0;
	float rx=0,ry=0,rz=0;
	float temp = 0.0;
	
	const float HEATING_ON_TEMP = 35.9;
	const float HEATING_OFF_TEMP = 36.1;
	
	while(1)
	{
		//mpu.readAccel(&gx,&gy,&gz);
		//mpu.readRot(&rx,&ry,&rz);
		temp = mpu.readTemp();
	
		bool heating = isHeating();
		
		printf("[%.1f,%.1f] %.2f °C, HEATING [%s]\n",HEATING_ON_TEMP,HEATING_OFF_TEMP,temp,(heating)?("ON"):("OFF"));
		
		if(!heating && temp<HEATING_ON_TEMP) setHeating(true);
		if(heating && temp>HEATING_OFF_TEMP) setHeating(false);
		
		usleep(1000000);
	}
}

int main()
{
	mpu.init();
	
	if(wiringPiSetupGpio () == -1)
	{
		printf("Couldn't initialize 'wiringPi'!\n");
		return -1;
	}
	
	pinMode(HEATING_ENABLE_PIN, OUTPUT);
	
	loop();
	return 0;
}