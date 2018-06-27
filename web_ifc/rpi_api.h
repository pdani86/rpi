#ifndef MY_RPI_WEBAPI
#define MY_RPI_WEBAPI

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 

#include <time.h>
#include <signal.h>

#include <pthread.h>

//#define DEBUG_WITHOUT_RPI

class rpiApi;

rpiApi* rpiApiInstance = 0;

#ifndef DEBUG_WITHOUT_RPI
#include <wiringPi.h>
#include "rpi_pca9685.h"
#include "rpi_AD7705.h"



const int INPUT_PIN = 4;
const int OUTPUT_PIN = 17;

int fan_tacho_cnt = 0;

void fan_tacho_interrupt();

#endif

class rpiApi
{
	pthread_mutex_t mutex;
	
	PCA9685 pca9685_ifc;
	AD7705 adc;
	
	int fan0_pc;
public:
	volatile int fan0_rpm;
	inline rpiApi() {
			fan0_pc = 0; fan0_rpm = 0; init(); rpiApiInstance=this;
			pthread_mutex_init(&mutex, NULL);
		}
	inline ~rpiApi() {pthread_mutex_destroy(&mutex);}
	void init();
	
	inline void lock() {pthread_mutex_lock(&mutex);}
	inline void unlock() {pthread_mutex_unlock(&mutex);}
	
	void setFanSpeed(int speed);
	void handleRequest(HttpConnection* client,HttpRequest* request);
};
#ifndef DEBUG_WITHOUT_RPI
long long last_tacho_ms = 0;

void fan_tacho_interrupt()
{
	fan_tacho_cnt++;
	//if(0 == fan_tacho_cnt%250) printf("tacho cnt: %d\n",fan_tacho_cnt);
	
	int nTacho = 80;
	int tachoPerRev = 2;
	int rpm = 0;
	if(0 == (fan_tacho_cnt%nTacho))
	{
		struct timespec now;
		clock_gettime(CLOCK_MONOTONIC,&now);
		long long now_ms = now.tv_sec*1000+now.tv_nsec/1000000;
		rpm = nTacho/tachoPerRev/((now_ms - last_tacho_ms)*(1.0/60000.0));
		last_tacho_ms = now_ms;
		//printf("rpm: %d\n", rpm);
		rpiApiInstance->fan0_rpm = rpm;
	}
}
#endif

void rpiApi::setFanSpeed(int speed)
{
	fan0_pc = speed;
	#ifndef DEBUG_WITHOUT_RPI
	if(0==speed) 
	{
		digitalWrite(OUTPUT_PIN,0);
	} else {
		digitalWrite(OUTPUT_PIN,1);
	}
	#endif
}

void rpiApi::init()
{
	#ifndef DEBUG_WITHOUT_RPI
	//if(wiringPiSetup () == -1)
	if(wiringPiSetupGpio () == -1)
	{
		printf("Couldn't initialize 'wiringPi'!\n");
		//return -1;
	}
	
	pca9685_ifc.init();
	
	pinMode(INPUT_PIN, INPUT);
	pullUpDnControl(INPUT_PIN, PUD_DOWN);
	//pullUpDnControl(INPUT_PIN, PUD_UP);
	wiringPiISR(INPUT_PIN,INT_EDGE_RISING,fan_tacho_interrupt);
	
	pinMode(OUTPUT_PIN, OUTPUT);
	pullUpDnControl(OUTPUT_PIN, PUD_OFF);
	#endif
}


const char* pca_outputNames[] = {
	"pca9685_out_0_off",
	"pca9685_out_1_off",
	"pca9685_out_2_off",
	"pca9685_out_3_off",
	"pca9685_out_4_off",
	"pca9685_out_5_off",
	"pca9685_out_6_off",
	"pca9685_out_7_off",
	"pca9685_out_8_off",
	"pca9685_out_9_off",
	"pca9685_out_10_off",
	"pca9685_out_11_off",
	"pca9685_out_12_off",
	"pca9685_out_13_off",
	"pca9685_out_14_off",
	"pca9685_out_15_off"
};

void rpiApi::handleRequest(HttpConnection* client,HttpRequest* request)
{
	lock();
	
	if(request->params.count("fan0_pc")==1) {
		int pc = atoi(request->params["fan0_pc"].c_str());
		if(pc>=0 && pc<=100)
		{
			setFanSpeed(pc);
		}
	}
	
	for(int i=0;i<16;i++)
	{
		const char* outputName = pca_outputNames[i];
		//printf("iterating output names [%d]: %s\n",i,outputName);
		if(request->params.count(outputName)==1) {
			int off_cnt = atoi(request->params[outputName].c_str());
			//printf("Set[%d]: %d\n",i,off_cnt);
			pca9685_ifc.setOutput(i,off_cnt);
		}
	}
	if(request->params.count("pca9685_prescale")==1) {
		int prescale = atoi(request->params["pca9685_prescale"].c_str());
		pca9685_ifc.setPwmPrescale(prescale);
	}
	
	
	int _fan0_pc = fan0_pc;
	int _fan0_rpm = fan0_rpm;
	
	int pca_out_0 = pca9685_ifc.getOutput(0);
	int pca_prescale = pca9685_ifc.getPwmPrescale();
	
	int output_values[16];
	for(int i=0;i<16;i++) output_values[i] = -1;
	bool getall = request->params.count("getall")==1;
	if(getall) {
		for(int i=0;i<16;i++) {
			int out = pca9685_ifc.getOutput(i);
			output_values[i] = out;
		}
	}
	
	int adc_value = adc.getADC();
	
	unlock();
	
	
	
	char buffer[1024];
	//snprintf(buffer,255,"fan0_pc=%d\r\nfan0_rpm=?\r\n\r\n",fan0_pc);
	if(!getall) {
		snprintf(buffer,1023,"fan0_pc=%d\r\nfan0_rpm=%d\r\npca9685_out_0_off=%d\r\npca9685_prescale=%d\r\n\r\n",
			_fan0_pc,_fan0_rpm,pca_out_0,pca_prescale);
	} else {
		snprintf(buffer,1023,
			"fan0_pc=%d\r\nfan0_rpm=%d\r\npca9685_prescale=%d\r\n"
			"adc_value=%d\r\n"
			"pca9685_out_0_off=%d\r\n"
			"pca9685_out_1_off=%d\r\n"
			"pca9685_out_2_off=%d\r\n"
			"pca9685_out_3_off=%d\r\n"
			"pca9685_out_4_off=%d\r\n"
			"pca9685_out_5_off=%d\r\n"
			"pca9685_out_6_off=%d\r\n"
			"pca9685_out_7_off=%d\r\n"
			"pca9685_out_8_off=%d\r\n"
			"pca9685_out_9_off=%d\r\n"
			"pca9685_out_10_off=%d\r\n"
			"pca9685_out_11_off=%d\r\n"
			"pca9685_out_12_off=%d\r\n"
			"pca9685_out_13_off=%d\r\n"
			"pca9685_out_14_off=%d\r\n"
			"pca9685_out_15_off=%d\r\n"
			"\r\n"
			,_fan0_pc,_fan0_rpm,pca_prescale,
			adc_value,
			output_values[0],
			output_values[1],
			output_values[2],
			output_values[3],
			output_values[4],
			output_values[5],
			output_values[6],
			output_values[7],
			output_values[8],
			output_values[9],
			output_values[10],
			output_values[11],
			output_values[12],
			output_values[13],
			output_values[14],
			output_values[15]
			);
		
	}
	int size = strlen(buffer);
	client->sendResponseHeader(200,size);
	client->send(buffer,size);
}

#endif