#ifndef MYTCP_H
#define MYTCP_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <fcntl.h>
#include <time.h>

#include <errno.h>


class TcpSocket
{
private:
	TcpSocket(const TcpSocket&);
	TcpSocket& operator=(const TcpSocket&);
protected:
int sockfd;

public:
	TcpSocket();
	TcpSocket(int _sockfd);
	~TcpSocket();
	
	inline void attach(int new_fd) {if(isValid()) {::close(sockfd);} sockfd=new_fd;}
	inline void attach(TcpSocket& other) {close(); this->sockfd = other.sockfd; other.detach(); }
	inline int detach() {int old_sockfd = sockfd; sockfd=-1; return old_sockfd; }

	inline int _fd() {return sockfd;}
	int setNonBlocking(bool noblock=true);
	int bind();
	int recv(void* buf,int buflen,int timeoutms=0);
	int send(const void* buf,int buflen,int timeoutms=0);
	int close();
	inline int isValid() {return (sockfd>=0);}
};

class TcpSocketSet
{
public:
};

class TcpClient:public TcpSocket
{
public:
    TcpClient();
    inline TcpClient(int _sockfd):TcpSocket(_sockfd) {}
    ~TcpClient();

    int connect(const char* host,unsigned short port);
};




class TcpServer:public TcpSocket
{
public:
TcpServer();
~TcpServer();

int bind(unsigned short port);
TcpClient* accept();
int listen(int maxPending);

};

#endif