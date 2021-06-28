#include <iostream>
using namespace std;

#include "./multi_client_tcp_server.cpp"

void onClientMessage(SOCKET client, char* msg);

int main(const int argc, const char* argv[]){
    MultiClientTcpServer tcpserver;
    tcpserver.Bind(25009);

    tcpserver.OnConnectDisconnectMessageFunctions(NULL, NULL, onClientMessage);

    tcpserver.Listen();

    return 0;
};

void onClientMessage(SOCKET client, char* msg){
    cout << msg << endl;
    return;
};