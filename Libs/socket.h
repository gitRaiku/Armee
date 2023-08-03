#ifndef SOCKET_H

#define RFUNC_NONE 0
#define RFUNC_ERR 0xFF

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
void make_empty_packet(char *buf, uint32_t *bl);
struct rpacket get_packet(FILE *__restrict log_file, struct rsocket c);

#define SOCKET_H
#endif
