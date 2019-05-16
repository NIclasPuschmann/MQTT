/* Data Declarations for MQTT protocol      */
/* used by C                      */
/* concerning publisher and broker*/
#include <WinSock2.h>

#define BUFFER_S 256
#define IP6_ADDR_LENGTH 46


enum QOS { QOS0, QOS1, QOS2 };

#pragma pack(1)
struct MessageHeader {
	unsigned char  ReqType;
#define CONNECT 'C'		     // we dont need as we are using tcp one hand (publish) and multicast with joins to subscribe on the other hand
#define CONNECT_ACK  'A'	// not used with QOS0
#define SUBSCRIBE 'S'		// not used with MC
#define UNSUBSCRIBE 'U'		// not used with MC
#define PUBLISH 'P'
#define MC_ANNOUNCE 'M'
	char pad[sizeof(int) - sizeof(unsigned char)]; // 32 bit-alignment
	unsigned int  msgLength;	  // message length (including message header)
}; // 8 Byte

#pragma pack(1)
struct publishMsg {
	struct MessageHeader header;
	unsigned int packetId;  // always 0 for QOS 0
	char topic[BUFFER_S];
	unsigned int qos; // look at enum QOS to fill this field
	unsigned int retainFlag; // should be false using QOS0 - no message saving at the broker	
	
	
	struct {
		int payloadType;
#define INTEGER 0;
#define FLOAT 1;
		int       intValue;	/* integer value */
		float     floatValue;	/* float value */
	}payload;
	unsigned int dupFlag;  // duplicate flag - only used with QOS > 0
};//292 byte

// for later use when we use reliable multicast
#pragma pack(1)
struct subscribeMsg {
	struct MessageHeader header;
	long int			 packetId;  // always 0 for QOS 0
	int					 topicLength;
	char				 topic[BUFFER_S]; // this should be a list of topic/qos values: just 1 topic in our code
};
#pragma pack(1)
struct MCAnnouncement { // each subscriber has to join a specific mc address (DEFAULT) to listen for announcements send by broker
	struct MessageHeader header;
	char topic[BUFFER_S];
	char mcAddress[IP6_ADDR_LENGTH]; // unique mc address to join for each topic
};

#define _MAX_MSG_SIZE 512
#pragma pack(1)
struct genericMsg{
	struct MessageHeader header; // 8 Bytes
	char   pad1[_MAX_MSG_SIZE];
  //placeholder for all messages which can be differentiated by header.ReqType
	//size should be als large as the largest messageType
};




