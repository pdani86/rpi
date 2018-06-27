#include "mytcp.h"

int TcpSocket::setNonBlocking(bool noblock)
{
    int flags = fcntl(sockfd,F_GETFL,0);
    if(flags<0) return -1;
    flags = (noblock)?(flags|O_NONBLOCK):(flags&~O_NONBLOCK);
    int ret = fcntl(sockfd,F_SETFL,flags);
    return 0;
}

TcpSocket::TcpSocket()
{
    sockfd = socket(AF_INET,SOCK_STREAM,0);
    int flag = 1;
    int result = setsockopt(sockfd,
	IPPROTO_TCP,
	TCP_NODELAY,
	(char *) &flag,
	sizeof(int));
}

TcpSocket::TcpSocket(int _sockfd)
{
    sockfd = _sockfd;
    int flag = 1;
    int result = setsockopt(sockfd,
	IPPROTO_TCP,
	TCP_NODELAY,
	(char *) &flag,
	sizeof(int));
}

TcpSocket::~TcpSocket()
{
    ::close(sockfd);
}

int TcpSocket::close()
{
    int ret = ::close(sockfd);
    sockfd = -1;
    return ret;
}

int TcpSocket::bind()
{
    return 0;
}

int TcpSocket::recv(void* buf,int buflen,int timeoutms)
{
    // TODO: NON-BLOCKING with -1 timeoutms?
    //int flags = MSG_DONTWAIT;
    int flags = 0;
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = timeoutms*1000;
    setsockopt(sockfd,SOL_SOCKET,SO_RCVTIMEO,(const char*)&tv,sizeof(struct timeval));
    int ret = ::recv(sockfd,buf,buflen,flags);
    return ret;
}

int TcpSocket::send(const void* buf,int buflen,int timeoutms)
{
    int ret = ::send(sockfd,buf,buflen,0);
    return ret;
}

// ----------------------------------------------

TcpClient::TcpClient()
{
}

TcpClient::~TcpClient()
{
}

int TcpClient::connect(const char* host,unsigned short port)
{
    struct sockaddr_in serv_addr;
    struct hostent* server;

    server = gethostbyname(host);
    if(server==0) return -1;

    bzero(&serv_addr,sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char*)server->h_addr,
	(char*)&serv_addr.sin_addr.s_addr,
	server->h_length
    );
    serv_addr.sin_port = htons(port);
    int ret = ::connect(sockfd,(struct sockaddr*)&serv_addr,sizeof(serv_addr));
    return ret;
}



// ----------------------------------------------



TcpServer::TcpServer()
{
    setNonBlocking(true);
}

TcpServer::~TcpServer()
{

}

int TcpServer::bind(unsigned short port)
{
    struct sockaddr_in serv_addr;

    bzero(&serv_addr,sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(port);

    int ret = ::bind(sockfd,(struct sockaddr*)&serv_addr,sizeof(serv_addr));
    return ret;
}

TcpClient* TcpServer::accept()
{
    struct sockaddr cli_addr;
    socklen_t addrlen = sizeof(cli_addr);
    int newsockfd = ::accept(sockfd,&cli_addr,&addrlen);
    
    TcpClient* newClient = 0;
    if(newsockfd>=0)
    {
		newClient = new TcpClient(newsockfd);
    }
    return newClient;
}

int TcpServer::listen(int maxPending)
{
    return ::listen(sockfd,maxPending);
}
