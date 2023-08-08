#ifndef SOCKET_H

#define RFUNC_NONE 0
#define RFUNC_ERR 0xFF

//
//  0x02 0x00 0x05 0x42 0x75 0x6E 0xC5 0x21 
//  ^^^^ ^^^^^^^^^ ^^^^^^^^^^^^^^^^^^^^^^^^
//    |      |                 |
// Function Length           Data
//

struct rpacket {
  uint8_t func;
  uint16_t len;
  char *data;
};

struct rsocket {
  int32_t sock;
  char *path;
};

struct rsocket setup_server(FILE *__restrict log_file, char *__restrict name);
void shutdown_server(struct rsocket s);
struct rsocket accept_connection(FILE *__restrict log_file, struct rsocket s);
void close_connection(struct rsocket c);

struct rsocket setup_server_connection(FILE *__restrict log_file, char *__restrict name, char *__restrict serverName);
void shutdown_server_connection(struct rsocket c);
struct rpacket make_empty_packet();
struct rpacket make_error_packet();
struct rpacket get_packet(FILE *__restrict log_file, struct rsocket c);
void send_packet(struct rsocket c, struct rpacket p);

#define SOCKET_H
#endif
