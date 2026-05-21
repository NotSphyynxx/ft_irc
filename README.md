# ft_irc - Internet Relay Chat Server

> A custom, lightweight, and strictly RFC 1459/2812 compliant Internet Relay Chat (IRC) server written entirely in C++98.

## 📝 Overview

`ft_irc` is a backend systems project designed to handle real-time, multiplexed network communication. It implements a fully functional IRC server capable of handling multiple concurrent clients in a single-threaded environment using synchronous I/O multiplexing. 

This server acts as the central hub for clients (like LimeChat, irssi, or netcat) to connect, authenticate, join channels, send private messages, manage channel operators, and even facilitate direct peer-to-peer file transfers (DCC).

## 🚀 Key Features

*   **Single-Threaded Multiplexing:** Built entirely around the `poll()` system call, handling hundreds of concurrent connections without thread overhead or race conditions.
*   **100% Non-Blocking I/O:** Every socket (listener and clients) is forced into `O_NONBLOCK` mode via `fcntl()`, ensuring the server never freezes while reading or writing to slow clients.
*   **Strict RFC Compliance:** Implements rigorous parsing for commands and colon-delimited trailing parameters according to RFC 1459 and RFC 2812.
*   **Robust Channel Management:** Full support for channel creation, operator privileges, and dynamic channel modes (`+i`, `+t`, `+k`, `+o`, `+l`).
*   **DCC File Transfer Support:** Perfectly routes CTCP handshakes (`\x01DCC SEND...\x01`) via `PRIVMSG`, allowing clients to establish direct peer-to-peer file transfers.

## ⚙️ Architecture & Tech Stack

*   **Language:** C++98
*   **Network API:** POSIX Sockets (`socket`, `bind`, `listen`, `accept`, `send`, `recv`)
*   **Multiplexing:** `poll()`
*   **Memory Management:** Strictly leak-free, managing buffers dynamically without crashing under heavy loads (e.g., partial sends/receives).

## 🛠️ Installation & Compilation

Ensure you have `make` and a C++ compiler (`c++` or `clang++`) installed.

```bash
# Clone the repository
git clone <git@github.com:Delyassss/ft_irc.git>
cd ft_irc

# Compile the server
make

# Run the server
./ircserv <port> <password>
```

**Example:**
```bash
./ircserv 6667 1337pass
```

## 💻 How to Connect

You can connect to the server using any standard IRC client. **LimeChat** is highly recommended for macOS users testing this project.

### Using an IRC Client (LimeChat / irssi):
1.  Add a new server in your client.
2.  Set the Host to `127.0.0.1` (or your machine's IP).
3.  Set the Port to the port you provided (e.g., `6667`).
4.  Set the Server Password to the password you provided (e.g., `1337pass`).
5.  Connect!

### Using Netcat (Raw Terminal):
```bash
nc 127.0.0.1 6667
# Once connected, manually type the handshake:
PASS 1337pass
NICK myNickname
USER myUser 0 * :My Real Name
```

## 📜 Supported Commands

### Connection & Authentication
*   `PASS <password>` - Authenticate with the server.
*   `NICK <nickname>` - Set or change your nickname.
*   `USER <username> <hostname> <servername> :<realname>` - Register your user details.
*   `QUIT [:<message>]` - Disconnect from the server safely.

### Chat & Interaction
*   `PRIVMSG <target> :<message>` - Send a private message to a user or a channel.
*   `NOTICE <target> :<message>` - Send a notice (similar to PRIVMSG, but never triggers automated replies).

### Channel Operations
*   `JOIN <channel>{,<channel>} [<key>{,<key>}]` - Join a channel (creates it if it doesn't exist).
*   `KICK <channel> <user> [:<reason>]` - Forcibly remove a user from a channel (Operator only).
*   `INVITE <nickname> <channel>` - Invite a user to an invite-only channel (Operator only).
*   `TOPIC <channel> [:<topic>]` - View or change the channel's topic.

### Channel Modes (`MODE <channel> {[+|-]|o|i|t|k|l} [<limit>|<user>|<password>]`)
*   `i` : Set/remove Invite-only channel.
*   `t` : Set/remove the restrictions of the TOPIC command to channel operators.
*   `k` : Set/remove the channel key (password).
*   `o` : Give/take channel operator privilege.
*   `l` : Set/remove the user limit to channel.

## 🤖 Bonus Features (IRC Bot)

To satisfy the bonus requirements of the project, a custom IRC Bot is integrated into the server environment. It listens for specific triggers in channels and responds automatically.

*   `!joke` - The bot replies to the channel with a randomly selected programming or dad joke.
*   `!roll` - The bot simulates rolling a dice and outputs a random number (e.g., 1-100) to the channel, perfect for settling disputes or playing games.
## 🧠 Handling Partial Data & Buffers

Network data doesn't always arrive in perfect, full strings. `ft_irc` employs a robust internal buffering system inside the `Client` class. 
*   If `recv()` grabs a partial command (missing the `\r\n`), it is appended to an internal read buffer until the full string arrives.
*   If the OS Kernel's `SO_SNDBUF` is full during a `send()`, the remaining data is safely pushed to a write buffer and sent only when `poll()` triggers a `POLLOUT` event, guaranteeing the server never blocks or crashes under heavy load.