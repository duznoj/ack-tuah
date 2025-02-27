<h1 align="center">Headers Parsing</h1>

Now that we're receiving packets. These are of the form:

<pre align ="center">
 -----------
| IP Header |
|-----------|
|           |
|IP Payload |
|           |
 -----------
</pre>

We can parse the IP Header using <a href="https://datatracker.ietf.org/doc/html/rfc791#section-3.1" target="_blank">RFC 791: Section 3.1</a>

We define a "packed" IPv4 Header struct, a packed struct means that the compiler doesn't add any padding between the fields of the struct.


Now create an instance of this struct and fill it from the packet. To get the values of some of the fields you might need to do some bitwise operations.

Note that all these fields are stored in Network Byte Order (Big Endian), so we have to make sure we're interacting with this data in a proper manner. I am building this on a Little-Endian machine, and while I am trying to write the code in an endianness-agnostic way. I still might miss something so beware.

From the IP header, we'll skip packets which aren't IPv4, i.e, `ipv4_header.ver != 0x04` and also any packets which are not TCP, i.e, `ipv4_header.protocol != 0x06`.


Now we can calculate the the offset to the beginning of the IP Payload, i.e, the start of the TCP Segment using the `total_length` and `header_length` fields of the IPv4 header.

Now in a similar fashion we create a TCP header struct. Parse out the TCP header by referring to Section [RFC 793: Section 3.1](https://www.ietf.org/rfc/rfc793.txt)

And then get the offset to the beginning of the TCP Payload, i.e, the start of the Application Layer Data.

Since TCP is a "connection oriented" protocol, we have to maintain a record of all the connections that our device has. Every connection can be uniquely identified by a quad-tuple of the (local_ip, local_port, remote_ip, remote_port)


So to keep a record of the conenctions, we'll create a HashMap where the above mentioned quad-tuple is the key of every entry/connection which maps to some sort of a state of the connection which is ever-changing.


To maintain a record of the state in which a connection between the local device and the remote device. We'll maintain a HashMap of a quad-tuple struct of (local_ip, local_port, remote_ip, remote_port) as keys to some sort of a connection state struct.


Figure 6 on Page 23 of [RFC 793: Section 3.1](https://www.ietf.org/rfc/rfc793.txt) gives a good depiction of the states a TCP host can be in, and when does it transition to other states in the form of a state machine.

<pre align="center">

                              +---------+ ---------\      active OPEN
                              |  CLOSED |            \    -----------
                              +---------+<---------\   \   create TCB
                                |     ^              \   \  snd SYN
                   passive OPEN |     |   CLOSE        \   \
                   ------------ |     | ----------       \   \
                    create TCB  |     | delete TCB         \   \
                                V     |                      \   \
                              +---------+            CLOSE    |    \
                              |  LISTEN |          ---------- |     |
                              +---------+          delete TCB |     |
                   rcv SYN      |     |     SEND              |     |
                  -----------   |     |    -------            |     V
 +---------+      snd SYN,ACK  /       \   snd SYN          +---------+
 |         |<-----------------           ------------------>|         |
 |   SYN   |                    rcv SYN                     |   SYN   |
 |   RCVD  |<-----------------------------------------------|   SENT  |
 |         |                    snd ACK                     |         |
 |         |------------------           -------------------|         |
 +---------+   rcv ACK of SYN  \       /  rcv SYN,ACK       +---------+
   |           --------------   |     |   -----------
   |                  x         |     |     snd ACK
   |                            V     V
   |  CLOSE                   +---------+
   | -------                  |  ESTAB  |
   | snd FIN                  +---------+
   |                   CLOSE    |     |    rcv FIN
   V                  -------   |     |    -------
 +---------+          snd FIN  /       \   snd ACK          +---------+
 |  FIN    |<-----------------           ------------------>|  CLOSE  |
 | WAIT-1  |------------------                              |   WAIT  |
 +---------+          rcv FIN  \                            +---------+
   | rcv ACK of FIN   -------   |                            CLOSE  |
   | --------------   snd ACK   |                           ------- |
   V        x                   V                           snd FIN V
 +---------+                  +---------+                   +---------+
 |FINWAIT-2|                  | CLOSING |                   | LAST-ACK|
 +---------+                  +---------+                   +---------+
   |                rcv ACK of FIN |                 rcv ACK of FIN |
   |  rcv FIN       -------------- |    Timeout=2MSL -------------- |
   |  -------              x       V    ------------        x       V
    \ snd ACK                 +---------+delete TCB         +---------+
     ------------------------>|TIME WAIT|------------------>| CLOSED  |
                              +---------+                   +---------+

</pre>

 
<hr>

#### HashMaps in C:
HashMaps are not a part of the C Standard Library, so there are no in-built implementations of a HashMap. Now we could build our own HashMap implementation, but building a good general-purpose HashMap implementation would become an entire project itself.

For now, we'll use troydhanson's [uthash](https://troydhanson.github.io/uthash/) library and create some functions around the macros that it provides.

<hr>

Now that we are only processing TCP packets in our program, that means the ICMP ping packets will be dropped by our program.

To check whether a TCP connection is being established or if it's working or not we'll use `netcat`, `nc` for short. netcat is a program which allows us to send raw strings/data over a TCP connection to any remote host.

To begin connecting to any of the IPs in our virtual network's subnet, the command is:

```bash
$ nc <ip> <port> # Command Syntax
$ nc 10.2.0.69 443 # Exmaple command
```

Now we can parse and print out some of the fields in the packet received to check if we're parsing it correctly.

The first packet that netcat will send is the `SYN` packet. This is a part of the 3-way-TCP-handshake.
Note that since we aren't responding to the SYN packet sent by netcat, it retransmits a SYN packet to us to keep trying and establish a connection. We can ignore the re-transmitted packets for now.

Next we'll try to complete the 3-way-handshake to establish a connection to be able to start sending data.
