#include "../include/ipv4.h"
#include <stdio.h>

void uint32_to_ip(uint32_t ip, char *ip_str) {
	// Convert the uint32_t (REQUIRED TO BE IN BIG ENDIAN FROM OUTSIDE) to dotted-decimal notation

	sprintf(ip_str, "%u.%u.%u.%u", 
			(ip) & 0xFF, // Extract the first byte (most significant byte)
			(ip >> 8) & 0xFF, // Extract the second byte
			(ip >> 16) & 0xFF,  // Extract the third byte
			(ip >> 24) & 0xFF);        // Extract the fourth byte (least significant byte)
}

