
/*
used by MQTT Publisher, Broker and Subscriber
*/

#define DEFAULT_BROKER_ADDRESS				"::1"			// ... will use the loopback interface
#define DEFAULT_FAMILY						AF_INET6		// IPv6 | PF_UNSPEC ... accept either IPv4 or IPv6
#define DEFAULT_PUBLISH_BROKER_SOCKTYPE		SOCK_STREAM		// SOCK_DGRAM....UDP  | SOCK_STREAM ... TCP
#define DEFAULT_PUBLISH_BROKER_PROTO		IPPROTO_TCP		// IPPROTO_UDP....UDP  | IPPROTO_TCP ... TCP
#define DEFAULT_SUBSCRIBER_BROKER_SOCKTYPE	SOCK_DGRAM	// SOCK_DGRAM....UDP  | SOCK_STREAM ... TCP
#define DEFAULT_SUBSCRIBER_BROKER_PROTO		IPPROTO_UDP		// IPPROTO_UDP....UDP  | IPPROTO_TCP ... TCP

#define BUF_SIZ 4096							// welche größe ist relevant ?!
#define BUFFER_SIZE						65500	// 65536

#define DEFAULT_ANNOUNCEMENT_MC			"FF01::10"	// ... ff01::	Interface local; ff02::	Link Local: 

#define TOPIC_MC_LOWER_BOUNDARY			"FF01:0:0:0:0:0:0:100"   //FF02:0:0:0:0:0:0:100  
#define TOPIC_MC_UPPER_BOUNDARY			"FF01:0:0:0:0:0:0:15C"   //FF02:0:0:0:0:0:0:15C

#define DEFAULT_BROKER_PUBLISH_PORT		"50000"      // 50000 ...  TCP test port must be used for publication
#define DEFAULT_BROKER_SUBSCRIBE_PORT	"50001"      // 50001 ...  UDP port must be used for subscription
#define DEFAULT_SUBSCRIBER_PORT			"50002"      // 50002 ...  UDP MC port must be  on Subscriber side



