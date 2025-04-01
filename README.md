# IRC Server - ft_irc

## Overview
**ft_irc** is a lightweight IRC (Internet Relay Chat) server written in C++98. It handles multiple clients using non-blocking I/O and supports essential IRC commands for communication and channel management.

## Features
- **Handles multiple clients simultaneously** using `poll()` (or equivalent).
- **No forking** and fully non-blocking I/O.
- **TCP/IP-based communication** (IPv4 & IPv6).
- **IRC Commands Supported:**
  - **Basic commands**: Authentication, setting nicknames, usernames, joining channels, sending private messages.
  - **Channel moderation commands**:
    - `KICK` - Remove a user from a channel.
    - `INVITE` - Invite a user to a channel.
    - `TOPIC` - Change or view the channel topic.
    - `MODE` - Modify channel settings (`i`, `t`, `k`, `o`, `l`).
- **Compatible with IRC clients** (tested with a reference client).

## Installation & Compilation

### Build the Server
```sh
 make
```
This will compile the project and create the `ircserv` executable.

### Clean Build Files
```sh
 make clean  # Removes object files
 make fclean # Removes object files and executable
 make re     # Rebuilds everything from scratch
```

## Usage
Run the server with the required arguments:
```sh
 ./ircserv <port> <password>
```
Example:
```sh
 ./ircserv 6667 mysecurepass
```
- `port`: The port number the server listens on (e.g., 6667).
- `password`: Required for clients to connect.

## Testing the Server
You can use `nc` (netcat) to manually test basic connectivity:
```sh
 nc -C 127.0.0.1 6667
```
To simulate fragmented commands:
```sh
 echo -n "NICK user1\n" | nc -C 127.0.0.1 6667
 echo -n "USER user1 0 * :Real Name\n" | nc -C 127.0.0.1 6667
```

For a full IRC experience, use an IRC client like **HexChat** or **irssi** to connect:
```sh
 /server 127.0.0.1 6667 mysecurepass
```

