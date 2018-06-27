/* A simple server in the internet domain using TCP
   The port number is passed as an argument */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
//#include <sys/socket.h>
//#include <netinet/in.h>
//#include <netinet/tcp.h>
#include <time.h>
#include <sys/time.h>
#include <wiringPi.h>
#include <signal.h>

const int ECHO_PIN = 27;
const int TRIGGER_PIN = 17;
const int TOO_CLOSE_PIN = 22;

void send_trigger()
{
printf("trigger\n");
	digitalWrite(TRIGGER_PIN,1);
	usleep(20);
	digitalWrite(TRIGGER_PIN,0);
}

bool exit_flag = false;
void loop()
{
	while(!exit_flag)
	{
	    send_trigger();
	    usleep(200000);
	}
}

void my_sig_handler(int s)
{
	printf("SIGINT received\n");
	exit_flag = true;
	exit(-1);
}

void echo_rising_edge()
{
    printf("echo rising edge\n");
}

void echo_falling_edge()
{
    printf("echo falling edge\n");
}

timeval last_rise_time;
timeval last_fall_time;


const int TOO_CLOSE_THRESHOLD_CM = 150;

void echo_isr()
{
    timeval t;
    bool on = digitalRead(ECHO_PIN);
//    if(on) printf("echo isr RISING\n");
//    else printf("echo isr FALLING\n");

    gettimeofday(&t,0);
    if(on) {
	int d_s = t.tv_sec - last_fall_time.tv_sec;
	int d_us = t.tv_usec - last_fall_time.tv_usec;
//	printf("ds: %d; dus: %d\n",d_s,d_us);
	last_rise_time = t;
    } else {
	int d_s = t.tv_sec - last_rise_time.tv_sec;
	int d_us = t.tv_usec - last_rise_time.tv_usec;
	//printf("ds: %d; dus: %d\n",d_s,d_us);

	if(d_s==0) {
	    int dist_cm = d_us/58;
	    printf("DISTANCE: %d cm\n",dist_cm);
	    digitalWrite(TOO_CLOSE_PIN,(dist_cm<=TOO_CLOSE_THRESHOLD_CM)?1:0);
	}

	last_fall_time = t;
    }
}

int my_init()
{
	memset(&last_rise_time,0,sizeof(last_rise_time));
	memset(&last_fall_time,0,sizeof(last_fall_time));
	signal(SIGINT,my_sig_handler);
	//if(wiringPiSetup () == -1)
	if(wiringPiSetupGpio () == -1)
	{
		printf("Couldn't initialize 'wiringPi'!\n");
		return -1;
	}
	
	pinMode(ECHO_PIN, INPUT);
	pullUpDnControl(ECHO_PIN, PUD_DOWN);
	//pullUpDnControl(INPUT_PIN, PUD_UP);
	//wiringPiISR(ECHO_PIN,INT_EDGE_RISING,echo_rising_edge);
	//wiringPiISR(ECHO_PIN,INT_EDGE_FALLING,echo_falling_edge);
	wiringPiISR(ECHO_PIN,INT_EDGE_BOTH,echo_isr);
	pinMode(TRIGGER_PIN, OUTPUT);
	pullUpDnControl(TRIGGER_PIN, PUD_OFF);
	pinMode(TOO_CLOSE_PIN, OUTPUT);
	pullUpDnControl(TRIGGER_PIN, PUD_DOWN);

	digitalWrite(TRIGGER_PIN,0);
	digitalWrite(TOO_CLOSE_PIN,0);
	
	loop();
	
	return 0;
}

int main(int argc, char *argv[])
{
	if(my_init()<0) return -1;
    return 0; 
}