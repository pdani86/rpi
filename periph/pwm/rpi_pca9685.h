#ifndef RPI_PCA9685_H
#define RPI_PCA9685_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 

#include <time.h>
#include <signal.h>

#include <wiringPi.h>
#include <wiringPiI2C.h>

enum PCA9685_REG_ADDR {
	MODE1 = 0x00,
	LED0_ON_L = 0x06,
	//LED0_OFF_L = 0x08,
	PRESCALE = 0xfe,
};

// PCA9685 I2C interface
class PCA9685
{
	int i2c_fd;
	unsigned char i2c_address;
public:
	inline PCA9685(unsigned char addr = 0x40);
	inline ~PCA9685();
	/*inline int setPwmFrequency(int hz);
	inline int getPwmFrequency();*/
	inline void init();
	inline void setPwmPrescale(int prescale);
	inline int getPwmPrescale();
	inline void setOutput(int ix,int val);
	inline int getOutput(int ix);
};

PCA9685::PCA9685(unsigned char addr)
{
	i2c_fd = wiringPiI2CSetup(addr);
	i2c_address = addr;
}

PCA9685::~PCA9685()
{

}

inline void PCA9685::init()
{
	wiringPiI2CWriteReg8(i2c_fd,0x00,0x00);
}

/*int PCA9685::setPwmFrequency(int hz)
{
	return 0;
}*/

inline void PCA9685::setPwmPrescale(int prescale)
{
	if(prescale<0) prescale = 0;
	if(prescale>255) prescale = 255;
	int mode1 = wiringPiI2CReadReg8(i2c_fd,0x00);
	int mode1_sleep = mode1 | 0x10;
	wiringPiI2CWriteReg8(i2c_fd,0x00,mode1_sleep);
	usleep(1000);
	wiringPiI2CWriteReg8(i2c_fd,PRESCALE,prescale);
	usleep(1000);
	wiringPiI2CWriteReg8(i2c_fd,0x00,mode1);
}

inline int PCA9685::getPwmPrescale()
{
	int prescale = wiringPiI2CReadReg8(i2c_fd,PRESCALE);
	return prescale;
}

void PCA9685::setOutput(int ix,int val)
{
	if(ix<0 || ix>=16) return;
	
	int addr_led_off_low = LED0_ON_L + ix*4+2;
	int addr_led_off_high = addr_led_off_low + 1;
	//if(val>4095) val = 4095;
	if(val>4096) val = 4095;
	if(val<0) val = 0;
	int valH = val >> 8;
	int valL = val & 0xff;
	wiringPiI2CWriteReg8(i2c_fd,addr_led_off_low,valL);
	wiringPiI2CWriteReg8(i2c_fd,addr_led_off_high,valH);
}

int PCA9685::getOutput(int ix)
{
	if(ix<0 || ix>=16) return 0;
	
	int addr_led_off_low = LED0_ON_L + ix*4+2;
	int addr_led_off_high = addr_led_off_low + 1;
	int valL = wiringPiI2CReadReg8(i2c_fd,addr_led_off_low);
	int valH = wiringPiI2CReadReg8(i2c_fd,addr_led_off_high);
	
	int val = valH<<8 | valL;
	return val;
}

#endif
