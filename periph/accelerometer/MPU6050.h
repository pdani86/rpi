#ifndef RPI_MPU6050_H
#define RPI_MPU6050_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 

#include <time.h>
#include <signal.h>

#include <wiringPi.h>
#include <wiringPiI2C.h>

enum MPU6050_REG_ADDR {
	SLEEP_REG = 0x6B,
	GYRO_CONFIG = 0x1B,
	ACC_CONFIG = 0x1C,
	
	ACCEL_X_H = 0x3B,
	ACCEL_X_L = 0x3C,
	ACCEL_Y_H = 0x3D,
	ACCEL_Y_L = 0x3E,
	ACCEL_Z_H = 0x3F,
	ACCEL_Z_L = 0x40,
	
	TEMP_H = 0x41,
	TEMP_L = 0x42,
	
	GYRO_X_H = 0x43,
	GYRO_X_L = 0x44,
	GYRO_Y_H = 0x45,
	GYRO_Y_L = 0x46,
	GYRO_Z_H = 0x47,
	GYRO_Z_L = 0x48,
};

class MPU6050
{
	int i2c_fd;
	unsigned char i2c_address;
	
	//int accLSBperG;
	//int gyrLSBperDegPerSec;
public: 
	inline MPU6050(unsigned char addr = 0b1101000);
	inline ~MPU6050();
	inline void init();
	
	//void selectAccRange(...);
	//void selectGyrRange(...);
	
	inline void readAccel(float* gx,float* gy,float* gz);
	inline void readRot(float* rx,float* ry,float* rz);
	inline float readTemp();
};

MPU6050::MPU6050(unsigned char addr)
{
	i2c_fd = wiringPiI2CSetup(addr);
	i2c_address = addr;
}

MPU6050::~MPU6050()
{

}

inline void MPU6050::init()
{
	wiringPiI2CWriteReg8(i2c_fd,SLEEP_REG,0x00);
	
	wiringPiI2CWriteReg8(i2c_fd,GYRO_CONFIG,0x00); // 0: +/- 250 deg/sec
	wiringPiI2CWriteReg8(i2c_fd,ACC_CONFIG,0x00); // 0: +/- 2g
		
}

inline void MPU6050::readAccel(float* gx,float* gy,float* gz)
{
	const int G = 16384; // assuming +/- 2g config
	int x = (wiringPiI2CReadReg8(i2c_fd,ACCEL_X_H)<<8) | wiringPiI2CReadReg8(i2c_fd,ACCEL_X_L);
	int y = (wiringPiI2CReadReg8(i2c_fd,ACCEL_Y_H)<<8) | wiringPiI2CReadReg8(i2c_fd,ACCEL_Y_L);
	int z = (wiringPiI2CReadReg8(i2c_fd,ACCEL_Z_H)<<8) | wiringPiI2CReadReg8(i2c_fd,ACCEL_Z_L);
	short xx = *(short*)&x;
	short yy = *(short*)&y;
	short zz = *(short*)&z;
	//printf("--- %06d %06d %06d ---\n",x,y,z);
	*gx = xx/(float)G;
	*gy = yy/(float)G;
	*gz = zz/(float)G;
}

inline void MPU6050::readRot(float* rx,float* ry,float* rz)
{
	const int DEG = 131; // assuming +/- 250 deg/s config
	int x = (wiringPiI2CReadReg8(i2c_fd,GYRO_X_H)<<8) | wiringPiI2CReadReg8(i2c_fd,GYRO_X_L);
	int y = (wiringPiI2CReadReg8(i2c_fd,GYRO_Y_H)<<8) | wiringPiI2CReadReg8(i2c_fd,GYRO_Y_L);
	int z = (wiringPiI2CReadReg8(i2c_fd,GYRO_Z_H)<<8) | wiringPiI2CReadReg8(i2c_fd,GYRO_Z_L);
	short xx = *(short*)&x;
	short yy = *(short*)&y;
	short zz = *(short*)&z;
	//printf("R--- %06d %06d %06d ---\n",x,y,z);
	*rx = xx/(float)DEG;
	*ry = yy/(float)DEG;
	*rz = zz/(float)DEG;
}

inline float MPU6050::readTemp()
{
	int temp = 0;
	temp = (wiringPiI2CReadReg8(i2c_fd,TEMP_H)<<8) | wiringPiI2CReadReg8(i2c_fd,TEMP_L);
	short _temp = *(short*)&temp;
	return _temp/(float)340+36.53;
}

#endif
