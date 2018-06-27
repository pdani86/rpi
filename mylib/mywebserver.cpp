#include "mywebserver.h"

#include <limits.h>
#include <stdlib.h>

HttpRequest& HttpRequest::parseRequest(const char* buf,int len)
{
	this->message = std::string(buf);
	if(len<=0) len = strlen(buf);
	int line_end = -1;
	int first_space = -1;
	int second_space = -1;
	int question_mark = -1;
	for(int i=0;i<len-1;i++)
	{
		if(buf[i]=='?') if(question_mark<0) question_mark = i;
		if(buf[i]==32)
		{
			if(first_space<0) first_space = i;
				else if(second_space<0) second_space = i;
		}
		
		if(
			buf[i]=='\r' && buf[i+1]=='\n'
		) {line_end = i; break;}
	}
	
	if(line_end>0 && second_space>0)
	{
		if(buf[0]=='G') this->method = HttpRequest::HTTP_METHOD_GET;
			else if(buf[0]=='P') this->method = HttpRequest::HTTP_METHOD_POST;
		
		int path_start = first_space+1;
		int path_end = (question_mark<0)?(second_space):(question_mark);
		this->path = std::string(buf+path_start,buf+path_end);
		if(question_mark>0)
		{
			this->query_string = std::string(buf+(question_mark+1),buf+second_space);
			this->params = MyUrl::parseQueryString(this->query_string);
		}
	}

	return *this;
}

HttpConnection::HttpConnection(HttpServer* parent, TcpSocket* socket, int id)
{
	this->parent = parent;
	this->attach(*socket);
	this->id = id;
	this->isClosed = false;
	int one = 1;
	setsockopt(this->_fd(),SOL_SOCKET,SO_REUSEADDR,&one,sizeof(one));
	setsockopt(this->_fd(),SOL_SOCKET,SO_REUSEPORT,&one,sizeof(one));
	/*if(st!=0) {
		printf("setsockopt REUSEADDR error!\n");
	}*/
	//setNonBlocking(false);
}


int HttpConnection::run()
{
	//printf("[%d] Http Connection Thread Started\n",id);
	HttpRequest request = receiveRequest();
	if(request.message.empty()) {
		//printf("[%d] COULDN'T RECEIVE REQUEST\n",id);
		this->close();
		if(parent) parent->connectionReady(this);
		return -1;
	}
	if(parent) {
		parent->httpRequest(this,&request);
		//usleep(2000000);
		this->close();
		this->isClosed = true;
		parent->connectionReady(this);
	}
	return 0;
}

HttpRequest HttpConnection::receiveRequest(int timeoutms)
{
	HttpRequest request;
	int pos = 0;
	int nrecv = 0;
	buf[0] = 0;
	
	int to = timeoutms;
	struct timespec start_time;
	struct timespec now;
	clock_gettime(CLOCK_MONOTONIC,&start_time);
	long long start_time_ms = start_time.tv_sec*1000+start_time.tv_nsec/1000000;
	long long now_ms;
	long long diff_ms = 0;
	
	while(nrecv>=0) {
	
		clock_gettime(CLOCK_MONOTONIC,&now);
		now_ms = now.tv_sec*1000+now.tv_nsec/1000000;
		diff_ms = now_ms - start_time_ms;
		if(diff_ms>=timeoutms) {
			//printf("RECEIVE REQUEST TIMEOUT\n");
			break;
		}
		to = timeoutms - diff_ms;
	
		nrecv = this->recv(&buf[pos],sizeof(buf)-pos-1,to);
		//printf("[%d] nrecv: %d\n",id,nrecv);
		//printf("[%d] errno: %d\n",id,errno);
		//if(errno==11) {nrecv=0; usleep(1000); continue;}
		
		if(nrecv>0)
		{
			pos+=nrecv;
			buf[pos] = 0;
			if(0!=strstr(buf,"\r\n\r\n")) {break;}
			//printf("Request: %s\n",buf);
		}
		
		
		
		if(nrecv<0 && errno==11) {
			usleep(1000);
			nrecv=0;
		}
	}
	if(pos>0) request.parseRequest(buf);
	return request;
}


void HttpConnection::sendResponseHeader(int status,int cLen)
{
	if(status==404) {
		const char* str = "HTTP/1.1 404 Not Found\r\nConnection: close\r\n\r\nNOT FOUND\r\n\r\n\0";
		this->send(str,strlen(str));
		return;
	}

	std::string head("HTTP/1.1 200 OK\r\nConnection: close\r\n");
	std::string cLenLine("Content-Length: ");
	char num[14];
	num[0]=num[sizeof(num)-1]=0;
	snprintf(num,13,"%d",cLen);
	cLenLine = cLenLine + std::string(num) + std::string("\r\n");
	
	std::string msg;
	msg = head + cLenLine + std::string("\r\n");
	this->send(msg.c_str(),msg.size());
}


HttpServer::HttpServer(int maxClients)
{
	if(maxClients<1) maxClients = 1;
	if(maxClients>8192) maxClients = 8192;
	this->maxClients = maxClients;
	clients = new HttpConnection*[maxClients];
	memset(clients,0,sizeof(HttpConnection*)*maxClients);
	nClients = 0;
	int one = 1;
	setsockopt(this->_fd(),SOL_SOCKET,SO_REUSEADDR,&one,sizeof(one));
	setsockopt(this->_fd(),SOL_SOCKET,SO_REUSEPORT,&one,sizeof(one));
}

HttpServer::~HttpServer()
{
	
}

void HttpServer::connectionReady(HttpConnection* conn)
{
	//printf("Client Served\n");
	//printf("Client Joining...\n");
	//conn->join();
	//usleep(200000);
	//printf("Joined\n");
	//delete conn;
}

void HttpServer::httpRequest(HttpConnection* client,HttpRequest* request)
{
	/*static int cnt = 0;
	printf("--------------------------\n");
	printf("#%04d --- Request received\n",cnt);
	printf("--------------------------\n");
	printf("Request Message:\n%s\n",request->message.c_str());
	printf("--------------------------\n");
	cnt++;
	printf("Method: ");
	if(request->method==HttpRequest::HTTP_METHOD_GET) printf("GET"); else printf("POST");
	printf("\n");
	printf("Path: %s\n",request->path.c_str());
	printf("Params: %s\n",request->query_string.c_str());
	printf("Params: ...\n");*/
	// ...
	
	// RESPOND
	const char* httpResponseStr = "HTTP/1.1 200 OK\r\n\r\nHello Pi!\r\n\r\n\0";
	client->send(httpResponseStr,strlen(httpResponseStr));
}

void HttpServer::_cleanupClients()
{
	for(int i=0;i<maxClients;i++)
	{
		HttpConnection* conn = clients[i];
		if(conn==0) continue;
		if(conn->isClosed)
		{
			conn->join();
			delete conn;
			clients[i] = 0;
			nClients--;
		}
	}
}

int HttpServer::_findEmptyConnectionSlot()
{
	if(nClients>=maxClients) return -1;
	for(int i=0;i<maxClients;i++)
	{
		if(clients[i]==0) return i;
	}
	return -1;
}

bool HttpServer::_addConnection(HttpConnection* conn)
{
	int ix = _findEmptyConnectionSlot();
	if(ix<0) return false;
	clients[ix] = conn;
	nClients++;
	return true;
}

void HttpServer::loop()
{
	int cnt = 0;
	
	while(1)
	{
		_cleanupClients();
		
		if(nClients>=maxClients) {usleep(1000); continue;}
		TcpClient* client = accept();
		if(client==0) {usleep(5000); continue;}
		HttpConnection* conn = new HttpConnection(this,client,cnt++);
		if(!_addConnection(conn)) {
			delete conn;
		} else {
			conn->start();
		}
	}
}

std::string WebServer::resolveAbsolutePath(std::string& rootDir,std::string& path)
{
	std::string result;
	char str[PATH_MAX];
	str[0] = 0;
	//printf("root: %s; path: %s\n",rootDir.c_str(),path.c_str());
	
	std::string pp = (rootDir+path);
	//realpath(pp.c_str(),str);
	//printf("realpath for \"%s\": %s\n",pp.c_str(),str);
	//result = "./index.html";
	if(str[0]==0) {
		result = "";
	} else {
		result = std::string(str);
	}
	//return result;
	return pp;
}

void WebServer::httpRequest(HttpConnection* client,HttpRequest* request)
{
	//HttpServer::httpRequest(client,request);
	std::string path = resolveAbsolutePath(rootDir,request->path);
	if(path=="") {
		client->sendResponseHeader(404,0);
		return;
	}
	//printf("[%d] Sending file [%s]\n",client->Id(),path.c_str());
	int fd = findOpenFile(path.c_str());
	
	if(fd<0) {
		//printf("[%d] ------------- File not found!\n",client->Id());
		client->sendResponseHeader(404,0);
		return;
	}
	int size = lseek(fd,0,SEEK_END);
	lseek(fd,0,SEEK_SET);
	client->sendResponseHeader(200,size);
	
	//printf("[%d] Sending file [%s]\n",client->Id(),path.c_str());
	sendFile(client,fd,size);
	::close(fd);
}


void WebServer::sendFile(HttpConnection* client,int fd,int size)
{
	sendfile(client->_fd(),fd,0,size);
}

int WebServer::findOpenFile(const char* path)
{
	int fd = open(path,O_RDONLY);
	return fd;
}