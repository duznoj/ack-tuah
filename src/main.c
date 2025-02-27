//======SYSTEM HEADERS======
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <linux/if_tun.h>
#include <linux/if.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <sys/socket.h>
//===========================

// PROJECT HEADERS these will be found in "./include"
#include "../include/ipv4.h"
#include "../include/tcp.h"
#include "../include/uthash.h"

int setup_tun_interface() {
	// Open the tunnel device file
	int tun_fd = open("/dev/net/tun", O_RDWR);
	if(tun_fd < 0) {
		perror("/dev/tun Open-ERR: ");
		return -1;
	}

	// Create a new Interface Request???
	struct ifreq request; // ifreq = interface request
	memset(&request, 0, sizeof(request));
	request.ifr_flags = IFF_TUN | IFF_NO_PI;
	strncpy(request.ifr_name, "tcpTestTun", IFNAMSIZ - 1);

		// Send out the Request to create the Interface with the name
	if(ioctl(tun_fd, TUNSETIFF, (void *)&request) < 0) {
		perror("IOCTL-ERR: ");
		close(tun_fd);
		return -1;
	}


	// A request to setup the network IP of the Interface/virtual network
	struct sockaddr_in sin;
	memset(&sin, 0, sizeof(struct sockaddr_in));
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = inet_addr("10.2.0.0");
	memcpy(&request.ifr_addr, &sin, sizeof(struct sockaddr_in));

	int sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (ioctl(sock, SIOCSIFADDR, &request) < 0) {
		perror("IOCTL SIOCSIFADDR failed");
		close(sock);
		close(tun_fd);
		return -1;
	}


	// A request to setup the subnet mask of the virtual network
	sin.sin_addr.s_addr = inet_addr("255.255.255.0");  // Set Subnet Mask
	memcpy(&request.ifr_netmask, &sin, sizeof(struct sockaddr_in));
	if (ioctl(sock, SIOCSIFNETMASK, &request) < 0) {
		perror("IOCTL SIOCSIFNETMASK failed");
		close(sock);
		close(tun_fd);
		return -1;
	}

	request.ifr_flags |= IFF_UP | IFF_RUNNING;
	if (ioctl(sock, SIOCSIFFLAGS, &request) < 0) {
		perror("ioctl(SIOCSIFFLAGS)");
		close(tun_fd);
		close(sock);
		return -1;
	}

	close(sock);
	return tun_fd;	
}


int main() {
	int tun_fd = setup_tun_interface();

	TCP_Connection *table = NULL;

	if(tun_fd == -1) {
		perror("Error in creating tunnel: ");
		exit(1);
	}
	printf("The virtual network's interface is setup\n\n\n\n");

	uint8_t packet[1500] = {0};
	while(1) {
		int bytesRead = read(tun_fd, packet, sizeof(packet));

		if(bytesRead < 0) {
			perror("Error reading packet: ");
			continue;
		}

		struct ipv4_header ip_header = *(struct ipv4_header *)&packet[0]; // Lil bit of C trickery to extract the packet header
		// Note that this header does not include any "options" in the header. You would have to manually calculate the header length
		// to figure out if any options were there then extract it out accordingly

		// SKIP packets which aren't IPv4
		if((ip_header.ver_and_header_len >> 4) != 4) {
			continue;
		}

		// SKIP packets which aren't TCP
		if(ip_header.protocol != 0x06) {
			continue;
		}

		int ip_header_length = (ip_header.ver_and_header_len & 0x0F) * 4;
		// SKIP packets with options in the header
		if(ip_header_length > 20) {
			continue;
		}

		char source_ip[32] = {0};
		char dest_ip[32] = {0};
		uint32_to_ip(ip_header.source_ip, source_ip);
		uint32_to_ip(ip_header.dest_ip, dest_ip);

		int tcp_segment_len = ntohs(ip_header.total_length) - ip_header_length;

		uint8_t *tcp_segment = packet + ip_header_length;

		struct tcp_header header = *(struct tcp_header *)tcp_segment;

		TCP_Quad quad = {
			.local_ip = ip_header.dest_ip,
			.remote_ip = ip_header.source_ip,
			.local_port = header.dest_port,
			.remote_port = header.source_port
		};

		int tcp_payload_len = tcp_segment_len - 4 * (ntohs(header.dataOffset_reserved_flags) >> 12);


		if(!find_connection(table, quad)) {
			// Should be closed by default
			//add_connection(&table, quad, CLOSED);
	
			// For now let's just 
			add_connection(&table, quad, LISTEN);
		}



		TCP_Connection *connection = find_connection(table, quad);


		printf("%s:%d -> %s:%d, %dB of TCP\n", source_ip, ntohs(header.source_port), dest_ip, ntohs(header.dest_port), tcp_payload_len);

		match_state(connection, packet);
	}
}
