#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include "data.h"

#pragma warning(disable : 4996)


struct list_entry {
	char topicRoute[BUFFER_S];
	char addr[IP6_ADDR_LENGTH]; // MC address
	struct list_entry *next;
};

struct list_type {
	struct list_entry *data;
	unsigned count;
};


struct list_type *init_list()
{ 
	struct list_type *list;
	list = (struct list_type *)malloc(sizeof(struct list_type));
	if (list==NULL)
	{
		perror("malloc() failed");
		return NULL;
	}
	list->data = NULL;
	list->count = 0;
	return list;
}

// new topic: store topic and  mc address
int add_topic(struct list_type *list, char *topic, char* mcAddr)
{
	struct list_entry *n;

	n = (struct list_entry*)malloc(sizeof(*n));
	if (n==NULL)
	{
		perror("malloc() failed");
		return 1;
	}

	strcpy(n->topicRoute, topic);
	strcpy(n->addr, mcAddr);

	n->next = list->data;

	list->data = n;
	list->count++;

	return 0;
}

struct list_entry *find_topic(struct list_type *list, char *topic)
{

	struct list_entry *n;
	n = list->data;
	while (n!=NULL)
	{
		if (!strcmp(n->topicRoute, topic))
			return n;
		n = n->next;
	}
	
	return NULL;
}



// subscriber can leave mc address

int remove_topic(struct list_type *list, char *topic)
{
	struct list_entry *lz, *lst = NULL;

	if (!list->count)
		return 1;

	for (lz = list->data; lz!=NULL; lz = lz->next)
	{
		if (!strcmp(lz->topicRoute, topic))
			break;
		lst = lz;
	}

	if (lz==NULL)
		return 1;

	if (lst)
		lst->next = lz->next;
	else
		list->data = lz->next;

	free(lz);
	list->count--;

	return 0;
}

void getAddr(struct list_entry *le, char *mcAddr)  // mcAddr must be at least 46 characters
{
	//
	if (le != NULL)
		strcpy_s(mcAddr, IP6_ADDR_LENGTH, le->addr);

}