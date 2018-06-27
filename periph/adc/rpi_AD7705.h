#ifndef RPI_AD7705_H
#define RPI_AD7705_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 

#include <time.h>
#include <signal.h>

//  #include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>
#include <wiringPi.h>
#include <wiringPiSPI.h>

class AD7705
{
	int spi_fd;
	int CHANNEL;
public:
	inline AD7705();
	inline ~AD7705();
	inline void init();
	
	inline int getADC();
	inline int writeByte(unsigned char b);
};

AD7705::AD7705()
{
	spi_fd = -1;
	CHANNEL = 0;
	init();
}

AD7705::~AD7705()
{
	if(spi_fd>=0) close(spi_fd);
}

int AD7705::writeByte(unsigned char b)
{
	unsigned char buf[1];
	buf[0] = b;
	int res = wiringPiSPIDataRW(CHANNEL, buf, 1);
	printf("0x%02X -> spi_buf[0]: 0x%02X\n",(int)b,(int)buf[0]);
	return res;
}

void AD7705::init()
{
	spi_fd = wiringPiSPISetup(CHANNEL, 500000);
	if(spi_fd==-1) printf("Couldn't initialize SPI\n");
	int result;
	
	int spiMode = 3;
	ioctl(spi_fd, SPI_IOC_WR_MODE, &spiMode);
	if(errno != 0) {printf("errno: %d\n",errno);}
	
	result = writeByte(0xff);
	result = writeByte(0xff);
	result = writeByte(0xff);
	result = writeByte(0xff);
	
	result = writeByte(0xff);
	result = writeByte(0xff);
	result = writeByte(0xff);
	result = writeByte(0xff);
	
	
	result = writeByte(0x20); // channel Ain1, next write: clock reg.
	result = writeByte(0x0C); // master clock, output rate
	result = writeByte(0x10); // channel Ain1, next write: setup reg.
	result = writeByte(0x40); //* gain = 1, bipolar mode, buffer off, clear FSYNC and perform a Self Calibration*/
}

int AD7705::getADC()
{
	unsigned char buf[16];
	buf[0] = 0x00;
	buf[1] = 0x00;
	int result;
	/*int cnt = 10;
	while(cnt-->0)
	{
		result = writeByte(0x08);
		buf[0] = 0x00;
		result = wiringPiSPIDataRW(CHANNEL, buf, 1);
		printf("spi_buf[0]: 0x%02X\n",(int)buf[0]);
		if(buf[0]&0x80) break;
		usleep(500000);
	}*/
	//usleep(150*1000);
	
	/*result = writeByte(0x08);
	usleep(1000);
	result = wiringPiSPIDataRW(CHANNEL, buf, 1);
	printf("Comm Reg.: %02X\n",buf[0]);
	usleep(10000);*/
	result = writeByte(0x38); /*set the next operation for 16 bit read from the data register */
	buf[0] = 0x00;
	buf[1] = 0x00;
	usleep(10000);
	result = wiringPiSPIDataRW(CHANNEL, buf, 2);
	printf("spi_res: %d\n",result);
	printf("spi_buf[0]: 0x%02X\n",(int)buf[0]);
	printf("spi_buf[1]: 0x%02X\n",(int)buf[1]);
	return buf[1]+buf[0]*256;
}

#endif
