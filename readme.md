## Group Members
- Nicholas Donahue
- David Ulloa

## Language Used
- C

## Libraries
- `<sys/socket.h>`: Used to create sockets with `socket()`, connect them with `connect()`, and send/receive data through `send()` and `recv()`.
- `<netinet/in.h>`: Contains constants and structures needed for internet domain addresses. This program uses the `sockaddr_in` structure to specify the server address and port when establishing a socket connection.
- `<netdb.h>`: Defines the structure `hostent` and provides functions for network database operations. It is used to resolve the hostname to an IP address using `gethostbyname()`, which is essential for establishing socket connections to a URL.

## Compiling Instructions
This program utilizes POSIX Libraries and Functions, so it must be compiled on an Operating System that adheres to POSIX standards.(Used AlmaLinux 8.9)

To compile the code, use the following command:
```
make all clean
```
