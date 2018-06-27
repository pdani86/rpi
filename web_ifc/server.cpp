#include "mytcp.h"
#include "mytcp.cpp"

#include "mywebserver.h"
#include "mywebserver.cpp"

#include "thread.h"

#include "rpi_api.h"

class MyWebServer:public WebServer
{
	//char* buffer;
	rpiApi api;
public:
	inline MyWebServer() {
		//buffer = (char*)malloc(512*1024);
		//this->setRootDir("/home/dani/programming/mywebserver/www");
		this->setRootDir("/home/pi/Programming/web/www");
	}
	inline ~MyWebServer() {/*free(buffer);*/}
	void httpRequest(HttpConnection* client,HttpRequest* request);
};

void MyWebServer::httpRequest(HttpConnection* client,HttpRequest* request)
{
	//printf("[%d] REQUEST PATH: %s\n",client->Id(),request->path.c_str());
	if(request->path == "/") {request->path = "/index.html";}
	if(request->path == "/api") {
		api.handleRequest(client,request);
	} else {
		WebServer::httpRequest(client,request);
	}
}

int main(int argc, char *argv[])
{
	MyWebServer server;
	int port = 80;
	do
	{
		printf("Binding to port %d ...\n",port);
		if(server.bind(port)>=0) break;
		printf("Port is unavailable\n");
		port++;
	} while(port<90);
	if(port<90) {printf("Server bound to port %d\n",port);} else {printf("Couldn't find available port\n"); return 0;}
	if(server.listen(60)!=0) printf("listen error\n");
	server.loop();
    return 0;
}