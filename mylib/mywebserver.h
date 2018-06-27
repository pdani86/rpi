
#ifndef MYWEBSERVER_H
#define MYWEBSERVER_H

#include <stdlib.h>
#include <iostream>
#include <string>
#include <vector>
#include <map>

#include <sys/sendfile.h>
#include <time.h>

#include "thread.h"

class MyUrl
{
public:
	static inline std::vector<std::string> stringSplit(std::string& str,char ch)
	{
		std::vector<std::string> result;
		int pos = 0;
		int last_delim = -1;
		while(true)
		{
			int ix = str.find(ch,last_delim+1);
			int start = last_delim + 1;
			int end = ix - 1;
			last_delim = ix;

			if(ix<0)
			{
				int len = str.size() - start;
				if(len>0)
				{
					result.push_back(str.substr(start,len));
				}
				break;
			} else {
				int len = end - start + 1;
				result.push_back(str.substr(start,len));
			}
		}
		return result;
	}

	static inline std::map<std::string,std::string> parseQueryString(std::string query_str)
	{
		std::map<std::string,std::string> result;

		std::vector<std::string> pairs = stringSplit(query_str,'&');
		for(int i=0;i<pairs.size();i++)
		{
			std::vector<std::string> param_val = stringSplit(pairs[i],'=');
			if(param_val.size()==1) {
				result[param_val[0]] = std::string();
			} else if(param_val.size()==2) {
				result[param_val[0]] = param_val[1];
			}
		}

		return result;
	}

	static inline std::string getPath(std::string& in)
	{
		std::string result;
		int ix = in.find('?');
		if(ix<0) return std::string();
		return in.substr(0,ix);
	}

	static inline std::string getQueryString(std::string& in)
	{
		std::string result;
		int ix = in.find('?');
		if(ix<0) return std::string();
		return in.substr(ix+1,in.length()-(ix+1));
	}
};

class HttpRequest
{
public:
	
	enum E_HTTP_METHOD {HTTP_METHOD_GET, HTTP_METHOD_POST};
	
	std::string message;
	
	E_HTTP_METHOD method;
	std::string path;
	std::string query_string;
	std::map<std::string,std::string> params;
	
	inline HttpRequest() {method = HTTP_METHOD_GET;}
	
	HttpRequest& parseRequest(const char* buf,int len=0);
};

class HttpServer;

class HttpConnection:public TcpSocket,public Thread
{
friend class HttpServer;

	HttpServer* parent;
	int id;
	
	char buf[16384];
	volatile bool isClosed; // signal to HttpServer that this connection can be deleted (join thread)
public:
	int run();
	
	HttpConnection(HttpServer* parent, TcpSocket* socket, int id=0);
	
	HttpRequest receiveRequest(int timeoutms=10000);
	
	void sendResponseHeader(int status,int cLen);
	inline int Id() {return id;}
};


class HttpServer:public TcpServer
{
friend class HttpConnection;
	int id;

	HttpConnection** clients;
	int maxClients;
	int nClients;
	
protected:
	virtual void connectionReady(HttpConnection* conn);
public:
	HttpServer(int maxClients = 128);
	~HttpServer();
	
	int _findEmptyConnectionSlot();
	bool _addConnection(HttpConnection* conn);
	void _cleanupClients();

	void loop();

	virtual void httpRequest(HttpConnection* client,HttpRequest* request);
};

class WebServer:public HttpServer
{
	std::string rootDir;
public:
    //void start();
    //void stop();

	void setRootDir(std::string rootDir) {this->rootDir = rootDir;}
	std::string resolveAbsolutePath(std::string& rootDir, std::string& path);
	
	void httpRequest(HttpConnection* client,HttpRequest* request);
	
	void sendFile(HttpConnection* client,int fd,int size);
	int findOpenFile(const char* path);
};



#endif
