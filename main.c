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
	if(tun_fd == -1) {
		perror("Error in creating tunnel: ");
		exit(1);
	}
	printf("The virtual network's interface is setup\n\n\n\n");

	uint8_t packet[1500] = {0};
	while(1) {
		int bytesRead = read(tun_fd, packet, sizeof(packet));

		printf("%d bytes received:\n[", bytesRead);
		for(int i=0; i<bytesRead; i++) {
			printf("0x%02X, ", packet[i]);
		}

		puts("\x1B[2D\x1B[0J]\n\n");

	}
}
