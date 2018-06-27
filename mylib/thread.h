#ifndef DANI_THREAD_H
#define DANI_THREAD_H

#include <pthread.h>

class Thread
{
	pthread_t handle;
	
	Thread(const Thread&);
	Thread& operator=(const Thread&);
	
protected:
	static void* _run(void*);
	virtual int run();
public:
	Thread();
	virtual ~Thread();
	void start();
	inline void join() {if(handle) pthread_join(handle,NULL);}
};


void* Thread::_run(void* instance)
{
	((Thread*)instance)->run();
	return 0;
}

Thread::Thread()
{
	handle=0;
}

Thread::~Thread()
{
	if(handle) pthread_join(handle,NULL);
}

int Thread::run()
{
	printf("Thread: I'm running!\n");
	return 0;
}

void Thread::start()
{
	if(handle!=0) return;
	int ret = pthread_create(&handle,0,Thread::_run,(void*)this);
}

#endif