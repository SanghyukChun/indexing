#ifndef _UTILH_
#define _UTILH_

#define LOG 0
#ifdef LOG
#define LOG_MESSAGE(msg) printf("%s\n", msg)
#else
#define LOG_MESSAGE(msg)
#endif

#include <linux/types.h>
#include <unistd.h>
#include <stdint.h>


typedef struct FlowInfo {
	__be32 saddr; /* source IP address. */
	__be32 daddr; /* destination IP address. */
	__be16 sport; /* source port number. */
	__be16 dport; /* destination port number. */
	u_char protocol; /* transport layer protocol. */
	struct timeval start; /* first packet arriving time. */
	struct timeval end; /* last packet arribing time. */
} FlowInfo;

typedef struct FlowMeta {
    FlowInfo flowinfo;
    uint16_t fileID;
    uint32_t offset;
} FlowMeta;

enum {
	TYPE_SADDR = 1,
	TYPE_DADDR = 2,
	TYPE_SPORT = 4,
	TYPE_DPORT = 8
};

#endif