#ifndef CLIENTSOCKET_H
#define CLIENTSOCKET_H

#ifdef WIN32
   #include <winsock2.h>
#else WIN32
   #include <sys/types.h>
   #include <netinet/in.h>
   #include <sys/socket.h>
   #include <netdb.h>
   #include <unistd.h>
   #include <arpa/inet.h>
   #include <sys/time.h>
#endif // WIN32

#include "FL/fl_ask.H"
#include "FL/Fl.H"
#include <stdio.h>
#include <stdlib.h>


class ClientSocket
{
private:
   unsigned short       port;
   struct hostent       *hostnm;
   struct sockaddr_in   server;
   unsigned int         s;
   unsigned long        ip;
   fd_set               readfds, writefds;
   struct timeval       TimeOut;
   char					stopped;
#ifdef WIN32
   long                 rc;
   WSADATA              wsa;
#endif // WIN32


public:
	unsigned char  connected;

    ClientSocket();
	char InitSocket(const char *rechner, const char* portnr);
	char ConnectServer(char message = 1);
	void CloseSocket();
	char SendMsg(char *msg, int numBytes);
	char ReceiveMsg(char *buffer, int numBytes);
};




#endif // CLIENTSOCKET_H

