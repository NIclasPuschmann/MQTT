//  a restricted amount of clients (FD_SETSIZE--> up to 64 publishers) might want to connect to broker;
//  publisher are held in list



extern int initServer(char *Address, char *PublisherPort, char *SubscriberPort);

extern struct request *getRequest();

extern void processMsg(struct genericMsg *pubPtr);

extern int sendAnswer(struct answer  *answPtr);

extern int exitServer();
