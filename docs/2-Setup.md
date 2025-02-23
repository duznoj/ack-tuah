<h3 align="center">Setup</h3>

Lets start simple with a `main.c` file and a `Makefile` to build the project.

To buld and run, use the commands (the $ is to show where the command prompt begins DO NOT copy it ffs):
```bash
$ make all
$ sudo ./ack-tuah
```



To begin developing and testing our implementation of the network stuff, we can't just bypass the OS to steal all the packets destined for our computer from the outside world and try to process them ourselves. As most of these packets are for the browser and other stuff. So web-surfing becomes impossible untill we build a complete TCP implementation, even then our program won't know how to do many important network processing like DNS queries etc.

This would basically not allow us to communicate with the outside world unless we've implemented an entire network stack. We don't want that. We want to be able to still use our browsers, but also test our program/TCP implementation in a legit real-world-like-scenario.

#### The Linux Tunnel Interface

This is where the "Linux Tunnel Interface" comes into play. The tunnel interface is a Linux specific feature which allows us to build a sort of "virtual network" inside of our computer system. So now we can directly take the packets being sent inside this virtual network and try to process them to build our TCP implementation.

Note that any of these packets will be packaged in the exact format as they would be if a NIC received them from the outside world, stripped off any Layer-2 info and passed the packet to the OS to handle. The only catch is that everything is happening within our virtual network, for now.

##### How to set up this network?
For this we use the `ioctl()` I/O Control System Call.


Linux exposes I/O devices, physical or virtual in the form of files. In our case the `"/dev/tun"` is the file which represents the Tunnel Network Interface device.

To configure said devices via any program the `ioctl()` System Call is used.

We call `ioctl()` four times.

1. Create a tunnel interface.
2. Setup an IP address of this interface.
3. Setup a subnet mask.
4. Start the interface.

What this all means is:

Basically you're ensuring if the OS comes across any packets with the destination IP Address in the range of the provided subnet, i.e `10.2.0.0/24` or `10.2.0.0 to 10.2.0.255` then the OS at the Network Layer hands the packet to our interface which we've created and named `tcpTestTun`, and set the source address of this packet as the IP at which the interface is set.

Why is the source IP address being changed? I don't really understand tbh. But what I THIINNNK is happening is:

The interface can sort of be thought of as a router? i guess that is what makes the most sense to me? So now the interface can also implement Network Layer stuff like routing tables etc. But we do not care about that stuff for now so we can ignore it.

Anyhow, now that the interface is setup for us to receive raw IP packets and handle them, doing a simple `read()` syscall on the Tunnel file descriptor will give you any IP packets that the interface received.

We can test it out by pinging any of the IP addresses on the subnetand printing out the raw bytes of the packet.

Notice that you receive some packets before even sending a ping. We'll be ignoring those. To also view any activity on our interface from another trusted program for testing we can use tshark or wireshark. I'm using tshark. To monitor any activity on an interface using tshark. First install it using your Linux distro's package manager.

To run tshark use the command:
```bash
$ sudo tshark -i <interface-name> # In our case it is tcpTestTun
```

Notice the tshark output:
```
Running as user "root" and group "root". This could be dangerous.
Capturing on 'tcpTestTun'
    1 0.000000000 fe80::10b5:17fa:d2bc:8f9a → ff02::2      ICMPv6 48 Router Solicitation
    2 10.070692098     10.2.0.0 → 10.2.0.69    ICMP 84 Echo (ping) request  id=0x82b6, seq=1/256, ttl=64
    3 11.250901867     10.2.0.0 → 10.2.0.69    ICMP 84 Echo (ping) request  id=0x82b6, seq=2/512, ttl=64
    4 12.280710394     10.2.0.0 → 10.2.0.69    ICMP 84 Echo (ping) request  id=0x82b6, seq=3/768, ttl=64
```

Ignore line 1. Line 2 onwards show that a ICPM(ping) packet is being sent from 10.2.0.0 to 10.2.0.69 and each packet is 84 Bytes. This is consistent with the output of ack-tuah which prints exactly 84 bytes being received.

```bash
$ sudo ./ack-tuah
Virtual Tunnel Created

84 bytes received:
[0x45, 0x00, 0x00, 0x54, 0x04, 0xEE, 0x40, 0x00, 0x40, 0x01, 0x21, 0x73, 0x0A, 0x02, 0x00, 0x00, 0x0A, 0x02, 0x00, 0x45, 0x08, 0x00, 0xB4, 0xDC, 0x8D, 0x85, 0x00, 0x02, 0x46, 0x45, 0xBB, 0x67, 0x00, 0x00, 0x00, 0x00, 0xF2, 0x1B, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37]
```

This is a raw IP packet, next up we'll start parsing this data and make sense of it.
