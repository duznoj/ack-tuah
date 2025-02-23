<h1 style="text-align: center;">Ack-Tuah</h1>


Disclaimer: I am no expert on any of this stuff, every explanation i give is what logically makes sense to me personally. The reality of anything I try to explain might be different, so take my words with a pinch of salt.

---

#### Motivation to build a TCP implementation

We've all used the Berkley/Linux Sockets API, either professionally or just to pass Computer Networks Lab or Distributed Systems Lab.

What are these sockets? How does any application receive data? How does one begin to implement TCP? Let's try and answer some of these questions from the ground up

The OSI model gives an ideal seaparation of concerns of how a networked Application should be designed:

<pre align="center">
---------------
| Application |
---------------
| Presentation|
---------------
|   Session   |
---------------
|   Transport |
---------------
|   Network   |
---------------
|   Data Link |
---------------
|   Physical  |
---------------
</pre>

But in practice the TCP/IP Network Model is used which looks like:


<pre align="center">
---------------
| Application |
---------------
|   Transport |
---------------
|   Network   |
---------------
|   Data Link |
---------------
|   Physical  |
---------------
</pre>

Where basically the Application Layer is supposed to handle the Session and Presentation layer functionalities
<br><br>

Now the regular sockets we've used are basically a bridge between the Transport Layer and the Application Layer in the TCP/IP model, i.e, our program/executable.

In essence every single networked application/application protocol that we come across. May it be websites using HTTP, accessing remote computers using SSH or Telnet, or data transfer over the network in the most complex of games, all at some point use the Sockets API and the basic syscalls it provides.<br>

Namely: `send()` `recv()` 

<br>
This can be further solidified with an example HTTP server in C using only basic Linux Networking System Calls which will send a simple h1 tag containing a "bonjour" to the client which tries to connect to this server. May it be a browser, or a PostMan request or a CURL request.
<br><br>

<details>
<summary>Click here to expand code example</summary>

```c
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 8080

// HTML response body
const char *response = "HTTP/1.1 200 OK\r\n"
"Content-Type: text/html; charset=UTF-8\r\n"
"Connection: close\r\n"
"\r\n"
"<html><body><h1>Bonjour</h1></body></html>";

int main() {
    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    char buffer[1024];

    // Create socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("Failed to create socket");
        exit(1);
    }

    // Set server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // Bind socket to the address and port
    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Failed to bind");
        close(server_fd);
        exit(1);
    }

    // Listen for incoming connections
    if (listen(server_fd, 5) < 0) {
        perror("Failed to listen");
        close(server_fd);
        exit(1);
    }

    printf("Server listening on port %d...\n", PORT);

    // Accept incoming connections
    client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_len);
    if (client_fd < 0) {
        perror("Failed to accept connection");
        close(server_fd);
        exit(1);
    }

    // Read the HTTP request (we're not processing it here)
    ssize_t bytes_received = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
    if (bytes_received < 0) {
        perror("Failed to receive data");
        close(client_fd);
        close(server_fd);
        exit(1);
    }
    buffer[bytes_received] = '\0';  // Null-terminate the request

    // Log the incoming request to the console
    printf("Received HTTP Request:\n%s\n", buffer);

    // Send HTTP response
    ssize_t bytes_sent = send(client_fd, response, strlen(response), 0);
    if (bytes_sent < 0) {
        perror("Failed to send response");
        close(client_fd);
        close(server_fd);
        exit(1);
    }

    printf("Response sent successfully.\n");

    // Close the client and server sockets
    close(client_fd);
    close(server_fd);

    return 0;
}
```
</details>

<br>
Simply copy paste this code into any C file and then compile and run it using (provided you gcc or any other C compiler installed on a Linux/Linux like machine):

```bash
$ gcc program.c -o main
$ ./main
```
Now hit up `http://localhost:8080` on your browser.

Notice how the raw data you received when the `recv()` syscall was exectued looked something like:

```bash
GET / HTTP/1.1
Host: localhost:8080
User-Agent: Mozilla/5.0 (X11; Ubuntu; Linux x86_64; rv:135.0) Gecko/20100101 Firefox/135.0
Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8
Accept-Language: en-US,en;q=0.5
Accept-Encoding: gzip, deflate, br, zstd
Connection: keep-alive
Upgrade-Insecure-Requests: 1
Sec-Fetch-Dest: document
Sec-Fetch-Mode: navigate
Sec-Fetch-Site: none
Sec-Fetch-User: ?1
Priority: u=0, i
```
Since this is a basic string in C which is nothing more than an array of single-byte characters.

This is quite literally the only thing the browser sent to us, these some 400-ish plain bytes of ASCII characters.

Now this doesn't mean every  protocol will send you bytes which are valid printable ASCII characters. But it will send you SOME plain ahh bytes for sure.

For example the FTP protocol does not send bytes which are valid ASCII in the initial stages of the connection.

The value of 1 single byte ranges from values [0-255] in decimal or [0-FF] in hexadecimal. So any bytes sent or received over a network can be seen as an array of some numbers in the above mentioned ranges

SOOOOOOOO...the message sent by the browser can also be seen as:
[47 45 54 20 2F 20 48 54 54 50 2F 31 2E 31 0A 48 6F 73 74 3A 20 6C 6F 63 61 6C 68 6F 73 74 3A 38 30 38 30 0A 55 73 65 72 2D 41 67 65 6E 74 3A 20 4D 6F 7A 69 6C 6C 61 2F 35 2E 30 20 28 58 31 31 3B 20 55 62 75 6E 74 75 3B 20 4C 69 6E 75 78 20 78 38 36 5F 36 34 3B 20 72 76 3A 31 33 35 2E 30 29 20 47 65 63 6B 6F 2F 32 30 31 30 30 31 30 31 20 46 69 72 65 66 6F 78 2F 31 33 35 2E 30 0A 41 63 63 65 70 74 3A 20 74 65 78 74 2F 68 74 6D 6C 2C 61 70 70 6C 69 63 61 74 69 6F 6E 2F 78 68 74 6D 6C 2B 78 6D 6C 2C 61 70 70 6C 69 63 61 74 69 6F 6E 2F 78 6D 6C 3B 71 3D 30 2E 39 2C 2A 2F 2A 3B 71 3D 30 2E 38 0A 41 63 63 65 70 74 2D 4C 61 6E 67 75 61 67 65 3A 20 65 6E 2D 55 53 2C 65 6E 3B 71 3D 30 2E 35 0A 41 63 63 65 70 74 2D 45 6E 63 6F 64 69 6E 67 3A 20 67 7A 69 70 2C 20 64 65 66 6C 61 74 65 2C 20 62 72 2C 20 7A 73 74 64 0A 43 6F 6E 6E 65 63 74 69 6F 6E 3A 20 6B 65 65 70 2D 61 6C 69 76 65 0A 55 70 67 72 61 64 65 2D 49 6E 73 65 63 75 72 65 2D 52 65 71 75 65 73 74 73 3A 20 31 0A 53 65 63 2D 46 65 74 63 68 2D 44 65 73 74 3A 20 64 6F 63 75 6D 65 6E 74 0A 53 65 63 2D 46 65 74 63 68 2D 4D 6F 64 65 3A 20 6E 61 76 69 67 61 74 65 0A 53 65 63 2D 46 65 74 63 68 2D 53 69 74 65 3A 20 6E 6F 6E 65 0A 53 65 63 2D 46 65 74 63 68 2D 55 73 65 72 3A 20 3F 31 0A 50 72 69 6F 72 69 74 79 3A 20 75 3D 30 2C 20 69]

There are no fancy JavaScript Objects called (req, res) being sent over the network which can be accessed right after receiving something from the internet. There are no app.get("/endpoint") methods like the ones you use in expressJS, these fancy libraries literally just parse out this array of bytes. Execute a whole load of CPU instructions to create these (req, res) objects in the JavaScript Runtime Environment, all for you to easily be able to communicate with the outside world (only in HTTP might I add) in an easy manner without having a care in the world about how things are actually done under the hood.

For example: if we ran the above program and hit `http:localhost:8080/xyz` the first line of the message that the server receives from the browser will look like: `GET /xyz HTTP/1.1`, i.e, the route/endpoint is nothing but the string starting from the 5th character of the message sent by the browser. But notice how the server still repsonds with the bonjour cuz we set it up in such a way that the response is always sent irrespective of what that endpoint requested is.

Now of course all of this is an extreme oversimplification of how complicated the HTTP protocol is. But it does show that the foundation of the Web or any other technology over the network/internet is simply sending of some bytes from one end, and then parsing them out on the other end by means of some set rules called a protocol.

How to know how the bytes of a protocol are supposed to be parsed? You can find documentation for the protocols in the form of RFC documents on the internet and build your own implementation for the protocol. Or maybe even build your own Application Layer Protocol.

## Build a TCP Implementation? How?
If anyone paid attention in Computer Networks class. A LOOTTTT of time was spent on discussing how important the Transport Layer Protocol named TCP is, how complicated it is, the different kinds of algorithms it uses.

Well, can we make a TCP implementation ourselves?
The short answer is yes. It's probably not a good idea tho. But imma do it anyways.

So how do we start implementing a Transport Layer Protocol, namely TCP? For that we have to get the "data" directly from the Network Layer below us. 

#### The Network Layer
Handles the routing of IP packets across the world and ensure it reaches the correct system.

And once it reaches the correct system/device. It's the Transport Layer's job to handle/maintain any sort of connections etc, arrange the packets in=order in case of TCP, and then hand over the data to the correct Application based on the Port Number of the IP packet/TCP segment.


What happens the moment the Network Interface Card (NIC, a hardware device which does the actual communication of our computer system) receives some data from the outside world/internet at the Data-Link/Physical level?

Well, the OS takes the data from the NIC, has the time of the life with that data. parsing the IPv4 headers and the TCP headers, and doing all of the Transport Layer Magic, and finally then after processing tf out of that packet does it give the data inside the packet to the required Application which is waiting on a `recv()`

Who says we can't bypass the OS and handle the TCP stuff ourselves as well?

In this project we'll see how we can get network access at lower-level than the Application Layer, receive raw IPv4 packets and handle everything right from the moment the "data" enters our computer from the outside Internet

The end goal of this project is to be able to communicate with a 3rd party server/computer using our TCP implementation. It will be a success if we're able to communicate at the application layer with any 3rd party server over a TCP connection.
