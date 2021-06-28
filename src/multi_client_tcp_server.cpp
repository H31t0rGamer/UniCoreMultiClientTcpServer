#ifndef _IOSTREAM_
#include <iostream>
#endif

#ifndef _WS2TCPIP_H_
#include <ws2tcpip.h>
#endif

#ifndef _WINSOCK2API_
#include <winsock2.h>
#endif

#define MAX_RECV 4096

#define perror(x, y) \
    std::cout << "[MultiClientTcpServer - ERROR] " x << y << std::endl; \
    ExitProcess(EXIT_FAILURE);

#define pwarn(x) \
    std::cout << "[MultiClientTcpServer - WARN] " x << std::endl;



// Default functions
void DEFAULT_CONNECTION_FUNCTION__(){
    return;
};
#define DEFAULT_CONNECTION_FUNCTION DEFAULT_CONNECTION_FUNCTION__

void DEFAULT_DISCONNECTION_FUNCTION__(){
    return;
};
#define DEFAULT_DISCONNECTION_FUNCTION DEFAULT_DISCONNECTION_FUNCTION__

void DEFAULT_MESSAGE_FUNCTION__(SOCKET sock, char* msg){
    return;
};
#define DEFAULT_MESSAGE_FUNCTION DEFAULT_MESSAGE_FUNCTION__

// end

class MultiClientTcpServer{
    public:
        MultiClientTcpServer();
        ~MultiClientTcpServer();
        int Listen();
        int Bind(int Port);
        int OnConnectDisconnectMessageFunctions(void (*connect)(), void (*disconnect)(), void (*message)(SOCKET, char*));
    private:
        WSADATA WSAData;
        WORD dllVersion;
        SOCKET sock;
        u_short sockPort;
        fd_set masterfd;
        void (*onClientConnected)();
        void (*onClientDisconnected)();
        void (*onClientMessage)(SOCKET, char*);
};


MultiClientTcpServer::MultiClientTcpServer(){
    this->dllVersion = MAKEWORD(2, 2);
    if(WSAStartup(dllVersion, &this->WSAData) < 0){
        perror("WSAStartup function error: ", WSAGetLastError());
    };

    this->sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(sock < 0){
        perror("socket function error: ", WSAGetLastError());
    };
};

MultiClientTcpServer::~MultiClientTcpServer(){
    WSACleanup();
};

int MultiClientTcpServer::Bind(int Port){

    this->sockPort = htons(Port);
    sockaddr_in sockAddr;

    sockAddr.sin_addr.S_un.S_addr = INADDR_ANY;
    sockAddr.sin_family = AF_INET;
    sockAddr.sin_port = this->sockPort;

    int yes = 1;

    if(setsockopt(this->sock, SOL_SOCKET, SO_REUSEADDR, (const char*)&yes, sizeof(yes)) < 0){
        perror("setsockopt function error: ", WSAGetLastError());
    };

    if(bind(this->sock, (sockaddr*)&sockAddr, sizeof(sockAddr)) < 0){
        perror("bind function error: ", WSAGetLastError());
    };
};

int MultiClientTcpServer::Listen(){
    if(listen(this->sock, SOMAXCONN) < 0){
        perror("listen function error: ", WSAGetLastError());
    };

    FD_ZERO(&this->masterfd);
    FD_SET(this->sock, &this->masterfd);

    while(true){
        fd_set copyfd = this->masterfd;
        int socketCount = select(NULL, &copyfd, NULL, NULL, NULL);

        for(int ms=0; ms<socketCount; ms++){
            SOCKET sock = copyfd.fd_array[ms];

            if(sock == this->sock){
                sockaddr_in clientAddr;
                int clientAddrSize = sizeof(clientAddr);
                SOCKET acceptedClient = accept(sock, (sockaddr*)&clientAddr, &clientAddrSize);
                FD_SET(acceptedClient, &this->masterfd);
            } else {
                char buf[MAX_RECV];
                int bytesIn = 0;
                onClientConnected();
                bytesIn = recv(sock, buf, MAX_RECV, 0);

                switch(bytesIn){
                    case 0:{
                        onClientDisconnected();
                        closesocket(sock);
                        FD_CLR(sock, &this->masterfd);
                        break;
                    };
                    case -1:{
                        pwarn("recv function error or client disconnected.");
                        closesocket(sock);
                        FD_CLR(sock, &this->masterfd);
                        break;
                    };
                    default:{
                        onClientMessage(sock, buf);
                    };
                };
            };
        };
    };
    return 0;
};

int MultiClientTcpServer::OnConnectDisconnectMessageFunctions(void (*connect)(), void (*disconnect)(), void (*message)(SOCKET, char*)){
    if(!connect) connect = DEFAULT_CONNECTION_FUNCTION;
    if(!disconnect) disconnect = DEFAULT_DISCONNECTION_FUNCTION;
    if(!message) message = DEFAULT_MESSAGE_FUNCTION;
    this->onClientConnected = connect;
    this->onClientDisconnected = disconnect;
    this->onClientMessage = message;
    return 0;
};