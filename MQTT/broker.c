#include <malloc.h>    /* broker.c */ 
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "data.h"
#include "config.h"
#include <winsock2.h>
#include "brokerSy.h"
#include "topicMgmt.h"

#include <direct.h>
#include <stdlib.h>

#define DEBUG

#pragma warning(disable : 4996)



void fillannnouncement(struct MCAnnouncement *req, char *topic, char *mcaddr)
{
	req->header.ReqType = 'M';
	req->header.msgLength = sizeof(struct MCAnnouncement);

	strcpy(req->topic, topic);
	strcpy(req->mcAddress, mcaddr);
}


void processMsg(struct genericMsg *pubPtr)
{
	struct MCAnnouncement req;
	struct publishMsg *ptr;
	struct list_entry *listentry=NULL;

	static struct list_type *listheadptr=NULL;
	char mcaddr[IP6_ADDR_LENGTH];

	if (pubPtr == NULL)
			return;

	
	switch ((pubPtr->header).ReqType) {
	case CONNECT:
		printf("got a connect message\n");
		break;	// we don't support
	case PUBLISH:
		printf("got a publish message\n");
		ptr = (struct publishMsg *)pubPtr;

		//printf("in process msg:\n packetid %d \t qos %d\n", ptr->packetid, ptr->qos);
		if ((ptr->payload).payloadType == 0)  //integer 
			printf("topic %s \t value %d\n\n", ptr->topic, (ptr->payload).intValue);
		else 
			printf("topic %s \t value %f\n\n", ptr->topic, (ptr->payload).floatValue);
			// topic und value ausgeben
		

		//initialize topic-mcaddress-lists
		if (!listheadptr)
			listheadptr = init_list();

		listentry = find_topic(listheadptr, ptr->topic);
		if (listentry == NULL){
			newMCAddr(mcaddr); //fuer neues topic neue mac adress aufmachen
			if ((add_topic(listheadptr, ptr->topic, mcaddr)) == 0)
			{
				fillannnouncement(&req, ptr->topic, mcaddr);
				sendUDPMsg((struct genericMsg*)&req, DEFAULT_ANNOUNCEMENT_MC, DEFAULT_SUBSCRIBER_PORT);
				Sleep(5000);

			}
			else
				printf("add new topic failed\n");
		}
		else
			getAddr(listentry, mcaddr);

		//now forward publish message towards subscriber mc address
		sendUDPMsg(pubPtr, mcaddr, DEFAULT_SUBSCRIBER_PORT);
		break;
	case SUBSCRIBE:
		// we don't support yet               
		break;

	default:
		fprintf(stderr, "processmsg: not supported message type\n");

	}
}




int main(int argc, char *argv[]) {
	int i;
	char *Server = NULL;
	char *PortPubl = DEFAULT_BROKER_PUBLISH_PORT; 
	char *PortSubs = DEFAULT_BROKER_SUBSCRIBE_PORT; // port# 50001

	//Programmparameter ueberpruefen
	printf("Broker active and waiting... (^C exits server) \n");

	initServer(Server, PortPubl, PortSubs);  /* Establish Server TCP-Socket (Publish) and UDP Port for subscriber*/
	//initialisierung server

	exitServer();
}
