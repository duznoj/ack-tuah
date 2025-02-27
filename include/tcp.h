#ifndef TCP_H
#define TCP_H

#include "uthash.h"
#include <stdbool.h>

struct tcp_header {
	uint16_t source_port;
	uint16_t dest_port;
	uint32_t seq_number;
	uint32_t ack_number;
	uint16_t dataOffset_reserved_flags;
	uint16_t window_size;
	uint16_t checksum;
	uint16_t urgent_pointer;
	// ignore options
}__attribute__((packed));

typedef struct {
	uint32_t local_ip;
	uint32_t remote_ip;
	uint16_t local_port;
	uint16_t remote_port;
} TCP_Quad;

typedef enum {
	CLOSED,
	LISTEN,
	SYN_RECVD,
	ESTABLISHED
} TCP_State;



typedef struct {
	TCP_Quad connection_quad; // key

	TCP_State connection_state; // values
	UT_hash_handle hh; // uthash stuff
} TCP_Connection;


// FLAGS

#define FIN 0x01
#define SYN 0x02
#define RST 0x04
#define PSH 0x08
#define ACK 0x10
#define URG 0x20

void add_connection(TCP_Connection **table, TCP_Quad, TCP_State);

TCP_Connection*  find_connection(TCP_Connection *table, TCP_Quad key);

void match_state(TCP_Connection *, uint8_t *);


#endif
