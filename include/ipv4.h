#ifndef IPv4_H
#define IPv4_H

#include <stdio.h>
#include <stdint.h>
struct ipv4_header {
	uint8_t ver_and_header_len; // 1st 4 bits = ver, then header length
	uint8_t tos; // tos = type of sercive?? Service urmom
	uint16_t total_length; // Total length of what? entire packet probably
	uint16_t identification;
	uint16_t fragment_offset;
	uint8_t ttl; // TTL = time to live
	uint8_t protocol; // Which protocol the packet payload is of
	uint16_t header_check_sum;
	uint32_t source_ip;
	uint32_t dest_ip;
	// options ranging from 0-40 Bytes are supposed to start here idk how to handle that shit?????	
}__attribute__((packed));



void uint32_to_ip(uint32_t ip, char *ip_str);

#endif // IPv4_H
