/* Aufgabenstellung serverSys.c - Win32 Konsole Version - connectionless */
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "data.h"
#include "config.h"
#include <winsock2.h>
#include <ws2tcpip.h>
#include <ctype.h>


#define WIN32_LEAN_AND_MEAN

// Link with ws2_32.lib
#pragma comment(lib, "Ws2_32.lib")

/************************************************/
/*** Declaration socket descriptor            ***/
/************************************************/
SOCKET ConnSocket;

/*****************************************************/
/*** Deklaration of socket address "local"         ***/
/*****************************************************/

struct sockaddr *sockaddr_ip6_local = NULL;

/******************************************************/
/*** Deklaration of socket address "remote" static  ***/
/******************************************************/
static struct sockaddr_in6 remoteAddr;   //Broker Address


int joinMCAddress(char *mcGroupAddr)
{
	struct ipv6_mreq mreq;  //multicast address
	struct addrinfo *resultMulticastAddress = NULL,
		*ptr = NULL,
		hints;
	char dest1[120];
	char dest2[120];
	/* Resolve multicast group address to join mc group*/
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET6;
	hints.ai_flags = AI_NUMERICHOST;
	getaddrinfo(mcGroupAddr, NULL, &hints, &resultMulticastAddress);
	
	if (getaddrinfo(mcGroupAddr, NULL, &hints, &resultMulticastAddress) != 0) {
		fprintf(stderr, "getaddrinfo MCAddress failed with error: %d\n", WSAGetLastError());
		WSACleanup();
		exit(-1);
	}
	
	inet_ntop(AF_INET6, &((struct sockaddr_in6*)(resultMulticastAddress->ai_addr))->sin6_addr, dest1, 100);

	/****************************************************************************/
	/* Join the multicast topic group to get publish message from broker		*/
	/****************************************************************************/
	memcpy(&mreq.ipv6mr_multiaddr, &((struct sockaddr_in6*)(resultMulticastAddress->ai_addr))->sin6_addr, sizeof(mreq.ipv6mr_multiaddr));
	
	inet_ntop(AF_INET6, &mreq.ipv6mr_multiaddr, dest2, 120);
	inet_ntop(AF_INET6, &resultMulticastAddress->ai_addr, dest2, 120);

	/* Accept multicast from any interface */
	// scope ID 0 : from any int, x your Int. scope (netsh int ipv6 sh addr)

	mreq.ipv6mr_interface = 0;

	/* Join the multicast address (check with: netsh interface ipv6 show joins x)*/
	if (setsockopt(ConnSocket, IPPROTO_IPV6, IPV6_JOIN_GROUP, (char*)&mreq, sizeof(mreq)) != 0) {
		fprintf(stderr, "setsockopt function failed, see Error:  %d\n", WSAGetLastError());
		WSACleanup();
		exit(-1);
	}

	freeaddrinfo(resultMulticastAddress);

	return(0);

}

int initClient() {
	//default MC Adress and port DEFAULT_SUBSCRIBER_PORT to get the broker announcements 

	int trueValue = 1, loopback = 0;  //setsockopt
	int val, i = 0;
	int addr_len;

	char *MCAddress = DEFAULT_ANNOUNCEMENT_MC; // announcement mc socket udp
	char *Port = DEFAULT_SUBSCRIBER_PORT;
	struct addrinfo *resultLocalAddress = NULL,
		*localAddr = NULL,
		*ptr = NULL,
		hints;
	WSADATA wsaData;
	WORD wVersionRequested;

	wVersionRequested = MAKEWORD(2, 2);
	if (WSAStartup(wVersionRequested, &wsaData) == SOCKET_ERROR) {
		printf("SERVER:   WSAStartup() failed!\n");
		printf("          error code:   %d\n", WSAGetLastError());
		exit(-1);
	}

	/**********************************************************/
	/*** Create a UDP Socket,								***/
	/*** connectionless service, address family INET6       ***/
	/**********************************************************/
	//...

	ConnSocket = socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP);

	if (ConnSocket == INVALID_SOCKET) {
		fprintf(stderr, "Error creating the socket");

	}


	/* Initialize socket */
	/* Reusing port for several subscriber listening to same multicast addr and port (testing on local machine only) */

	int s = setsockopt(ConnSocket, SOL_SOCKET, SO_REUSEADDR, (char *)&trueValue, sizeof(trueValue));

	if (s == SOCKET_ERROR) {
		printf("Error at function setsockopt");
	}

	/* Resolve local address (anyaddr) to bind*/
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET6;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_protocol = IPPROTO_UDP;

	hints.ai_flags = AI_PASSIVE; //localhost

	val = getaddrinfo(NULL, Port, &hints, &resultLocalAddress);

	if (val != 0) {
		printf("getaddrinfo localAddress failed with error: %d\n", val);
		WSACleanup();
		exit(-1);
	}

	// Retrieve the address 
	for (ptr = resultLocalAddress; ptr != NULL; ptr = ptr->ai_next) {

		printf("getaddrinfo response %d\n", i++);
		printf("\tFlags: 0x%x\n", ptr->ai_flags);
		printf("\tFamily: ");
		switch (ptr->ai_family) {
		case AF_UNSPEC:
			printf("Unspecified\n");
			break;
		case AF_INET:
			printf("AF_INET (IPv4)\n");
			break;
		case AF_INET6:
			printf("AF_INET6 (IPv6)\n");

			sockaddr_ip6_local = (struct sockaddr *) ptr->ai_addr;
			addr_len = ptr->ai_addrlen;
			break;
		default:
			printf("Other %ld\n", ptr->ai_family);
			break;
		}
	}

	/*********************************************/
	/*** Bind Socket ***/
	/*********************************************/

	if (bind(ConnSocket, sockaddr_ip6_local, addr_len) == SOCKET_ERROR) {
		printf("ERROR at binding Socket \n");
		closesocket(ConnSocket);

	}
	else   printf("\tPort : %s sucessfully bound\n", Port);


	freeaddrinfo(resultLocalAddress);

	/****************************************************************************/
	/* Specify the multicast group to get announcements from broker				*/
	/****************************************************************************/

	if (joinMCAddress(DEFAULT_ANNOUNCEMENT_MC) != 0)
		printf("Joining MC Group Address for getting Announcements failed!\n");

	return(0);
}

void getRequest(struct genericMsg *req) {

	int recvcc;	/*  received message length */
	int remoteAddrSize = sizeof(struct sockaddr_in6);
	char msgPlaceholder[_MAX_MSG_SIZE];


	struct sockaddr_in6 remoteAddr;
	
	printf("in getRequest\n\n");

	recvcc = recvfrom(ConnSocket, (char *)msgPlaceholder, sizeof(msgPlaceholder), 0, &remoteAddr, &remoteAddrSize);


	memcpy((void *)req, (const void *)msgPlaceholder, sizeof(msgPlaceholder));


}


int closeClient() {

	/************************/
	/*** Close Socket     ***/
	/************************/
	closesocket(ConnSocket);
	printf("in exit srver\n");
	return(0);
}