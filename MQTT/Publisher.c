#define WIN32_LEAN_AND_MEAN

#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include "data.h"
#include "config.h"


#pragma comment(lib, "ws2_32.lib")
#pragma warning(disable : 4996)

int startWinsock(void);




int main(int argc, char *argv[])
{
	long rc;
	SOCKET s;
	SOCKADDR_IN6 addr;
	char buf[292];
	struct publishMsg p;

	p.payload.intValue = INTEGER;
	p.payload.floatValue = FLOAT;
	p.header.ReqType = PUBLISH;
	p.qos = 0;
	p.packetId = 0;
	p.retainFlag = 0;
	p.dupFlag = 0;
	char *addrBroker = argv[2];



	// Winsock starten
	rc = startWinsock();
	if (rc != 0)
	{
		printf("Error: startWinsock, Error code: %d\n", rc);
		return 1;
	}
	else
	{
		printf("Winsock startet!\n");
	}

	// Socket erstellen
	s = socket(DEFAULT_FAMILY, DEFAULT_PUBLISH_BROKER_SOCKTYPE, 0);
	if (s == INVALID_SOCKET)
	{
		printf("Error: The socket could not be created, Error code: %d\n", WSAGetLastError());
		return 1;
	}
	else
	{
		printf("TCP Socket created!\n");
	}

	// Verbinden
	memset(&addr, 0, sizeof(addr)); // zuerst alles auf 0 setzten
	addr.sin6_family = DEFAULT_FAMILY;
	addr.sin6_port = htons(atoi(argv[1])); // wir verwenden port 50000
	inet_pton(DEFAULT_FAMILY, addrBroker, &addr.sin6_addr);
	

	rc = connect(s, (struct sockaddr*)&addr, sizeof(addr));
	if (rc == SOCKET_ERROR)
	{
		printf("Error: connect failed, Error code: %d\n", WSAGetLastError());
		return 1;
	}
	else
	{
		printf("Connected !");
	}


	// Daten austauschen
	while (rc != SOCKET_ERROR)
	{
		printf("\nInsert topic name: ");
		scanf(" %s", &p.topic);
		printf("\nEntry PayloadType (INT = 0 | FLOAT = 1): ");
		scanf(" %d", &p.payload.payloadType);
		printf("Enter Value: ");
		if (p.payload.payloadType == 0)
		{
			scanf(" %d", &p.payload.intValue);
			p.payload.floatValue = 0.0;
		}
		else
			if (p.payload.payloadType == 1)
			{
				scanf(" %f", &p.payload.floatValue);
				p.payload.intValue = 0;
			}
			else
			{
				printf("Wrong Payload Type Usage above\n");
				continue;
			}


		send(s, (char*)&p, sizeof(p), 0);


	}
	
	closesocket(s);
	WSACleanup();
	return 0;
}

int startWinsock(void)
{
	WSADATA wsa;
	return WSAStartup(MAKEWORD(2, 0), &wsa);
}