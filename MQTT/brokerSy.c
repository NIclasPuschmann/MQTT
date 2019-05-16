
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <winsock2.h>
#include <ws2ipdef.h>
#include <windows.h>
#include <ws2tcpip.h>
#include <mstcpip.h>
#include <ctype.h>
#include <tchar.h>
#include <signal.h>
#include "data.h"
#include "config.h"

#define WIN32_LEAN_AND_MEAN

// Link with ws2_32.lib
#pragma comment(lib, "Ws2_32.lib")

#define TCP 0
#define UDP 1

static SOCKET ServSock[2][FD_SETSIZE]; //	ServSock[0][] TCP Broker listening socket [0][0] towards Publisher and Data Sockets [0][1...NumSock];
									   //   ServSock[1][] UDP Broker socket towards Subscriber; 
static unsigned int numSocks[2];	   // number of open sockets; index 0--> TCP, index 1--> UDP



int add_client(SOCKET sock, int sockType)
{
	int i;
	switch (sockType)
	{
	case SOCK_STREAM:
		i = numSocks[TCP];
		if (i < FD_SETSIZE)
		{
			ServSock[TCP][i] = sock;
			numSocks[TCP]++;
		}
		else
			fprintf(stderr, "in add_client: skipped client, number of sockets too large!\n");
		break;
	case SOCK_DGRAM:  //this case should not happen as we use just one UDP socket to send packets to different subscriber mc addresses
		fprintf(stderr, "in add_client: new UDP socket should not occur!\n");
		if (i = numSocks[UDP] < FD_SETSIZE)
		{
			ServSock[UDP][i] = sock;
			numSocks[UDP]++;
		}
		else
			fprintf(stderr, "in add_client: skipped client, number of sockets too large!\n");
		break;
	default: fprintf(stderr, "in add_client: not supported socket type!\n"); return (-1);
	}
	return 0;
}

int remove_client(SOCKET sock)
{
	// just handling TCP sockets
	int i;
	for (i = 0; i < (int)numSocks[TCP]; i++) {
		
		if (ServSock[TCP][i] == sock) {
			ServSock[TCP][i] = ServSock[TCP][numSocks[TCP]-1];
			ServSock[TCP][numSocks[TCP]-1] = 0;
			break;
		}
	}
	
	if (i == numSocks[TCP]) {
		printf("Socket not included\n");
	}
	
	else 
	numSocks[TCP] = numSocks[TCP]-1; 
	printf("Socket closed... \n");
	closesocket(sock);
	return 0;
}

unsigned int fill_set(fd_set *fds)
{
	unsigned int max = 0;   // SOCKET is unsigned int (basetsd.h)
	unsigned int i, j;

	for (j = 0; j < 2; j++)
	{
		for (i = 0; i < numSocks[j]; i++)
		{
			FD_SET(ServSock[j][i], fds);
			max++;
		}
	}
	return max;
}

SOCKET get_sender(fd_set *fds)
{
	SOCKET i = 0;

	while (!FD_ISSET(i, fds))
		i++;

	return i;
}

// must be in the range of TOPIC_MC_LOWER_BOUNDARY and TOPIC_MC_UPPER_BOUNDARY
// we just increment the last 2 Bytes within the range for testing
void newMCAddr(char *lastMCAddr) //lastMCAddr is char
{
	    static short int i = 0;
		struct in6_addr addrBuff;
	
		strcpy_s(lastMCAddr, sizeof(TOPIC_MC_LOWER_BOUNDARY), TOPIC_MC_LOWER_BOUNDARY);  //puts the Lower MC Boundery into lastMCAddr

		inet_pton(DEFAULT_FAMILY, lastMCAddr, &addrBuff);  //from char to in6_addr

		addrBuff.u.Word[7] = addrBuff.u.Word[7] + htons(i);//erhöhe um i=mcAddr_Shift, Achtung network byte order!


		inet_ntop(DEFAULT_FAMILY, &addrBuff, lastMCAddr, IP6_ADDR_LENGTH);  //from in6_addr to char

		printf("New MC Address %s \n", lastMCAddr);
		i++;
	//}
};



void sendUDPMsg(struct genericMsg* annMsg, char *mcGroupAddr, char *port) {

	struct addrinfo *result = NULL, hints;
	char msgPlaceholder[_MAX_MSG_SIZE+8]; // add 8 bytes because of the Message Header Size

	memcpy((void *)msgPlaceholder, (const void *)annMsg, sizeof(*annMsg));

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = DEFAULT_FAMILY;
	hints.ai_socktype = DEFAULT_SUBSCRIBER_BROKER_SOCKTYPE;
	hints.ai_flags = AI_NUMERICHOST;
	hints.ai_protocol = DEFAULT_SUBSCRIBER_BROKER_PROTO;

	getaddrinfo(mcGroupAddr, DEFAULT_SUBSCRIBER_PORT, &hints, &result);


	unsigned int n = 0;
	setsockopt(ServSock[1][0], IPPROTO_IPV6, IPV6_MULTICAST_IF, (char*)&n, sizeof(n));
	sendto(ServSock[1][0], msgPlaceholder, sizeof(msgPlaceholder), 0, result->ai_addr, result->ai_addrlen);
	


	char multiaddr[100];
	inet_ntop(DEFAULT_FAMILY, &((struct sockaddr_in6*)(result->ai_addr))->sin6_addr, multiaddr, sizeof(multiaddr));
	printf("Multicastaddress:  %s\n", multiaddr);
	struct sockaddr_in6 addr;
	ZeroMemory(&addr, sizeof(addr));
	addr.sin6_port = htons(port);
	addr.sin6_family = DEFAULT_FAMILY;
	inet_pton(DEFAULT_FAMILY, mcGroupAddr, &addr.sin6_addr);

	printf("Message sent! \n");

};

//Shows information about the Publish Message
void PublishInfo(struct publishMsg *pubinfo) {
	printf("-----------------------------------------\n");
	printf("PUBLISH MESSAGE INFORMATION: \n\n");
	printf("Topic \t\t %s\n", pubinfo->topic);
	printf("PayloadType \t %d \n", pubinfo->payload.payloadType);
	printf("IntValue \t %d \n", pubinfo->payload.intValue);
	printf("FloatValue \t %f \n", pubinfo->payload.floatValue);
	printf("-----------------------------------------\n");
}

int initServer(char *Server, char *PublisherPort, char *SubscriberPort)
{
	WSADATA wsaData;
	WORD wVersionRequested;

	struct addrinfo *result = NULL, *ptr = NULL, hints;

	fd_set SockSet;
	SOCKET DataSock;

	int i, j, RetVal, AmountRead;
	int sockType = SOCK_STREAM;
	unsigned int max;

	struct genericMsg msg;
	/* Ask for Winsock version 2.2.*/
	wVersionRequested = MAKEWORD(2, 2);

	if (WSAStartup(wVersionRequested, &wsaData) == SOCKET_ERROR) {
		printf("Publisher:   WSAStartup() failed!\n");
		printf("             error code:   %d\n", WSAGetLastError());
		WSACleanup();
		exit(-1);
	}
	for (j = 0; j <= 1; j++) {
		/* j == 0 :Create an TCP endpoint for communication with publishers: "50000"*/
		/* j == 1 :Create an UDP endpoint for communication with publishers*: "50001"*/

		ZeroMemory(&hints, sizeof(hints));
		hints.ai_family = DEFAULT_FAMILY;  
		if (j == 0) {
			hints.ai_socktype = DEFAULT_PUBLISH_BROKER_SOCKTYPE;
			hints.ai_protocol = DEFAULT_PUBLISH_BROKER_PROTO;
		}
		else {
			hints.ai_socktype = DEFAULT_SUBSCRIBER_BROKER_SOCKTYPE;
			hints.ai_protocol = DEFAULT_SUBSCRIBER_BROKER_PROTO;
		}

		hints.ai_flags = AI_NUMERICHOST | AI_PASSIVE;
		RetVal = getaddrinfo(Server, j ? SubscriberPort : PublisherPort, &hints, &result);  // DEFAULT_BROKER_PORT	"50000"    -->TCP test port / "50001" DEFAULT_SUBSCRIBER_PORT	--> UDP test  
		if (RetVal != 0) {
			fprintf(stderr, "getaddrinfo failed with error %d: %s\n",
				RetVal, gai_strerror(RetVal));
			WSACleanup();
			return -1;
		}
		//
		// For each address getaddrinfo returned, we create a new socket,
		// bind that address to it, and create a queue to listen on.
		//
		for (i = 0, ptr = result; ptr != NULL; ptr = ptr->ai_next) {

			// Highly unlikely, but check anyway.
			if (i == FD_SETSIZE) {
				printf("getaddrinfo returned more addresses than we could use.\n");
				break;
			}
			// This example only supports PF_INET and PF_INET6.
			if ((ptr->ai_family != AF_INET) && (ptr->ai_family != AF_INET6))
				continue;

			// Open a socket with the correct address family for this address.
			ServSock[j][i] = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
			if (ServSock[j][i] == INVALID_SOCKET) {
				fprintf(stderr, "socket() failed with error %d: %s\n",
					WSAGetLastError(), WSAGetLastError());
				continue;
			}

			if ((ptr->ai_family == AF_INET6) &&
				IN6_IS_ADDR_LINKLOCAL((IN6_ADDR *)INETADDR_ADDRESS(ptr->ai_addr)) &&
				(((SOCKADDR_IN6 *)(ptr->ai_addr))->sin6_scope_id == 0)
				) {
				fprintf(stderr,
					"IPv6 link local addresses should specify a scope ID!\n");
			}


			else   printf("Bind to port %s \n  ", j ? SubscriberPort : PublisherPort);

			if (j == 0) { // SOCK_STREAM

				if (bind(ServSock[j][i], ptr->ai_addr, (int)ptr->ai_addrlen) == SOCKET_ERROR) {
					fprintf(stderr, "bind() failed with error %d: %s\n",
						WSAGetLastError(), WSAGetLastError());
					closesocket(ServSock[j][i]);
					continue;
				}
				if (listen(ServSock[j][i], 5) == SOCKET_ERROR) {
					fprintf(stderr, "listen() failed with error %d: %s\n",
						WSAGetLastError(), WSAGetLastError());
					closesocket(ServSock[j][i]);
					continue;
				}
				printf("'Listening' on port %s, protocol %s, protocol family %s\n",
					PublisherPort, "TCP", (ptr->ai_family == AF_INET) ? "AF_INET" : "AF_INET6");
			}
			i++;
		}

		freeaddrinfo(ptr);

		if (i == 0) {
			if (j == 0) fprintf(stderr, "Fatal error: unable to serve TCP socket on any address.\n");
			else fprintf(stderr, "Fatal error: unable to serve UDP socket on any address.\n");
			WSACleanup();
			return -1;
		}
		numSocks[j] = i;
	}// for-iteration over the two ports


	// We now put the server into an eternal loop,
	// serving requests as they arrive.



	while (1) //Waiting on Publishers
	{
		FD_ZERO(&SockSet);
		max = fill_set(&SockSet);
		printf("number of sockets %d\n", max);

		select(max, &SockSet, NULL, NULL, NULL);

		if (FD_ISSET(ServSock[0][0], &SockSet)) //hack: we just support one TCP listening socket 
		{
			printf("after select --> NEW SOCKET\n");
			DataSock = accept(ServSock[0][0], NULL, 0);
			add_client(DataSock, SOCK_STREAM);
		}
		else
		{

			printf("after select --> DATA\n");
			DataSock = get_sender(&SockSet);

			ZeroMemory(&msg, sizeof(struct genericMsg));
			AmountRead = recv(DataSock, (char*)&msg, sizeof(msg), 0);// as we send max. 292 Byte 
			// we do not exceed TCP segment size; hopefully we get all Bytes with one recv
			if (AmountRead <= 0) {
				remove_client(DataSock);
			}
			else {
				processMsg((struct genericMsg*)&msg);
				struct publishMsg *pubinfo = (struct publishMsg *)(&msg);
				PublishInfo(pubinfo);

			}
		}
	}
}

int exitServer() {

	return(0);
}