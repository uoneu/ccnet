#include "../sockets_ops.h"
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <iostream>
#include "../inet_address.h"
#include "../socket.h"

using namespace std;

using namespace ccnet::net;

int main( int argc, char* argv[] )
{
    if( argc <= 2 )
    {
        printf( "usage: %s ip_address port_number\n", basename( argv[0] ) );
        return 1;
    }
    const char* ip = argv[1];
    int port = atoi( argv[2] );

    InetAddress addr(ip, port);
    Socket listen_sock(::socket(PF_INET, SOCK_STREAM, IPPROTO_TCP));
    assert(listen_sock.fd() > 2);
    listen_sock.setReusePort(true);
    listen_sock.bindAddress(addr);
    listen_sock.listen();
    
    InetAddress client;
    Socket connfd(listen_sock.accept(&client));

    if ( connfd.fd() < 0 )
    {
        printf( "errno is: %d\n", errno );
    }
    else
    {
        cout << client.toIpPort()<< endl;
        
    }

    cout << "----\n";
    cout << sockets::isSelfConnect(listen_sock.fd()) << endl;

    return 0;
}




/*
int main( int argc, char* argv[] )
{
    if( argc <= 2 )
    {
        printf( "usage: %s ip_address port_number\n", basename( argv[0] ) );
        return 1;
    }
    const char* ip = argv[1];
    int port = atoi( argv[2] );

    InetAddress addr(ip, port);

    //struct sockaddr_in ;
   //bzero( &address, sizeof( address ));
    //sockets::fromIpPort(ip, port, &address);

    //int sock = sockets::createNonblockingOrDie(PF_INET);
    int sock = ::socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
  
    assert( sock >= 0 );

    sockets::bindOrDie(sock, addr.getSockAddr());
    sockets::listenOrDie(sock);

    struct sockaddr_in client;
    int connfd = sockets::accept(sock, &client);
    InetAddress tmp(client);
    cout << tmp.toIpPort();
   
   
    if ( connfd < 0 )
    {
        printf( "errno is: %d\n", errno );
    }
    else
    {
        
        char buffer[1024];
        while (1) {
            sleep(2);
            memset( buffer, '\0', 1024);
            int ret = read(connfd, buffer, 1023);
            cout << buffer << endl;
            if (ret == 0) break;

        }


        char remote[INET_ADDRSTRLEN];
        sockets::toIpPort(remote, sizeof(remote), sockets::sockaddr_cast(&client));
        printf( "clinet-> %s\n", remote);

        struct sockaddr_in tmp = sockets::getLocalAddr(connfd);
        sockets::toIpPort(remote, sizeof(remote), sockets::sockaddr_cast(&tmp));
        printf( "clinet-> %s\n", remote);

        tmp = sockets::getPeerAddr(connfd);
        sockets::toIpPort(remote, sizeof(remote), sockets::sockaddr_cast(&tmp));
        printf( "clinet-> %s\n", remote);
        
        sockets::close( connfd );
    

    }

    cout << "----\n";
    cout << sockets::isSelfConnect(sock) << endl;

    sockets::close(sock);

    return 0;
}
*/


