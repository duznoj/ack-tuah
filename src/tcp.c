#include "../include/tcp.h"
#include "../include/ipv4.h"
#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <arpa/inet.h>

void add_connection(TCP_Connection **table, TCP_Quad quad, TCP_State state) {
	TCP_Connection *connection = malloc(sizeof(TCP_Connection));

	if(!connection) {
		assert(0 && "Malloc Failed");
		return;
	}

	connection->connection_quad = quad;
	connection->connection_state = state;

	HASH_ADD(hh, *table, connection_quad, sizeof(TCP_Quad), connection);
}

TCP_Connection*  find_connection(TCP_Connection *table, TCP_Quad key) {
	TCP_Connection *result = NULL;
	HASH_FIND(hh, table, &key, sizeof(TCP_Quad), result);
	return result;
}

uint8_t get_tcp_flags(uint16_t dataOffset_reserved_flags) {
	return (uint8_t)(ntohs(dataOffset_reserved_flags) & 0x3F);
}
  

void match_state(TCP_Connection *connection, uint8_t *packet) {
	if(connection->connection_state == CLOSED) {
		return;
	}

	struct ipv4_header in_ip_header = *(struct ipv4_header *)packet;
	int ip_header_length = (in_ip_header.ver_and_header_len & 0x0F) * 4;

	uint8_t *tcp_segment = packet + ip_header_length; // or can also be named ip_payload
	struct tcp_header in_tcp_header = *(struct tcp_header *)tcp_segment;


	uint8_t flags = get_tcp_flags(in_tcp_header.dataOffset_reserved_flags);

	if(connection->connection_state == LISTEN) {
		// Should only receive a SYN packet
		if(flags != SYN){
			printf("SYN only Expected\n");
			return;
		}

		printf("Above packet is a SYN packet!!!!\n");
		connection->connection_state = SYN_RECVD;
	}

}
