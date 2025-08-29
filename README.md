# Group-Chat-Server📲

This project involves building a simple group chat application composed of a server and multiple clients. The server is responsible for accepting client connections, receiving messages, and broadcasting them to all connected clients while maintaining global message ordering. Each client acts as a fuzzer that generates random messages and sends them to the server while also listening for incoming messages. Communication between server and clients follows a strict message protocol with type identifiers, sender metadata (IP and port), and newline-terminated content. To ensure proper termination, a two-phase commit protocol is implemented: clients notify the server when they are done sending messages, and the server responds when all clients have finished, prompting all processes to exit gracefully. The system is designed to handle concurrent messaging and scaling with up to 100 clients

## ⚙️ How to Build

### Prerequisites

- CMake installed
- A C compiler (e.g., clang or gcc)

### Build Instructions

```bash
# From the root of your project directory
cmake -S . -B build
cmake --build build
```
This will produce two executables in the build/ directory:
-server
-client


###How to run
Start the server by:
-<port>: Port number to listen on (e.g., 8000)<br>
-<# of clients>: Total number of expected clients (e.g., 5)<br>

```./build/server <port> <# of clients>```

ex.
```./build/server 8000 3```

Run the client by:
```./build/client <IP> <port> <# of messages> <log file path>```

-<IP>: IP address of the server (e.g., 127.0.0.1)<br>
-<port>: Port number the server is listening on (e.g., 8000)<br>
-<# of messages>: Number of random messages the client should send<br>
-<log file path>: File to save received messages<br>

ex.
```./build/client 127.0.0.1 8000 10 client1.log```



