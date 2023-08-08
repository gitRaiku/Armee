#include <stdio.h>
#include <stdint.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "socket.h"
#include "log.h"
/// Server code

struct rsocket setup_server(FILE *__restrict log_file, char *__restrict name) {
  struct sockaddr_un ssockaddr = {0};

  struct rsocket s = {0};

  s.sock = socket(AF_UNIX, SOCK_STREAM, 0);
  if (s.sock == -1) {
    logg(10, log_file, "Couldn't open the socket! [%m]!");
    exit(1);
  }
  socklen_t len = sizeof(ssockaddr);

  s.path = malloc(strlen(name) + strlen("/tmp/.sock") + 1);
  sprintf(s.path, "/tmp/%s.sock", name);
  ssockaddr.sun_family = AF_UNIX;
  strcpy(ssockaddr.sun_path, s.path);
  unlink(s.path);

  int32_t rc = bind(s.sock, (struct sockaddr *) &ssockaddr, len);
  if (rc == -1) {
    logg(10, log_file, "Couldn't bind the socket! [%m]!");
    close(s.sock);
    exit(1);
  }

  rc = listen(s.sock, 10);
  if (rc == -1) {
    logg(10, log_file, "Could not start listening!");
    close(s.sock);
    exit(1);
  }
  logg(0, log_file, "Sucessfully setup connection at %s!\n", s.path);

  return s;
}

void shutdown_server(struct rsocket s) {
  shutdown(s.sock, SHUT_RDWR);
  unlink(s.path);
  free(s.path);
}

struct rsocket accept_connection(FILE *__restrict log_file, struct rsocket s) {
  struct sockaddr_un csockaddr = {0};
  socklen_t len = sizeof(csockaddr);
  struct rsocket c = {0};
  c.sock = accept(s.sock, (struct sockaddr *) &csockaddr, &len);
  if (c.sock == -1) {
    logg(5, log_file, "Couldn't accept a connection! [%m]!");
    return c;
  }

  len = sizeof(csockaddr);
  if ((getpeername(c.sock, (struct sockaddr *) &csockaddr, &len)) == -1) {
    logg(5, log_file, "Couldn't get the peer name of the connection! [%m]!");
    close(c.sock);
    c.sock = -1;
    return c;
  }
  s.path = strdup(csockaddr.sun_path);
  logg(0, log_file, "Got a connection: %s!", csockaddr.sun_path);

  return c;
}

void close_connection(struct rsocket c) {
  shutdown(c.sock, SHUT_RDWR);
  free(c.path);
}

/// Client code:

struct rsocket setup_server_connection(FILE *__restrict log_file, char *__restrict name, char *__restrict serverName) {
  int32_t rc;
  struct sockaddr_un ssockaddr = {0}; 
  struct sockaddr_un csockaddr = {0};

  struct rsocket c = {0};
  c.sock = socket(AF_UNIX, SOCK_STREAM, 0);

  /// TODO: Make sure name is unique
  c.path = malloc(strlen(name) + strlen("/tmp/.client") + 1);
  sprintf(c.path, "/tmp/%s.client", name);
  csockaddr.sun_family = AF_UNIX;
  strcpy(csockaddr.sun_path, c.path);
  unlink(c.path);

  rc = bind(c.sock, (struct sockaddr *) &csockaddr, sizeof(csockaddr));
  if (rc == -1) {
    logg(10, log_file, "Could not bind the socket! [%m]!");
    close(c.sock);
    exit(1);
  }

  ssockaddr.sun_family = AF_UNIX;
  {
    char sname[512] = "";
    snprintf(sname, sizeof(sname), "/tmp/%s.sock", serverName);
    strcpy(ssockaddr.sun_path, sname);
  }
  rc = connect(c.sock, (struct sockaddr *) &ssockaddr, sizeof(ssockaddr));
  if (rc == -1) {
    logg(10, log_file, "Could not connect to the server! [%m]!");
    close(c.sock);
    exit(1);
  }
  logg(0, log_file, "Connected to the server!");

  return c;
}

void shutdown_server_connection(struct rsocket c) {
  shutdown(c.sock, SHUT_RDWR);
  unlink(c.path);
  free(c.path);
}

struct rpacket make_empty_packet() {
  struct rpacket p = {0};
  return p;
}

struct rpacket make_error_packet() {
  struct rpacket p = {0};
  p.func = RFUNC_ERR;
  return p;
}

struct rpacket get_packet(FILE *__restrict log_file, struct rsocket c) {
  struct rpacket p = {0};

  if ((recv(c.sock, &p.func, 1, 0)) == -1) { logg(5, log_file, "Error recieving function! %m"); close(c.sock); return make_error_packet(); }
  fprintf(stdout, "Got function: %x\n", p.func);
  if ((recv(c.sock, &p.len, 2, 0)) == -1) { logg(5, log_file, "Error recieving length! %m"); close(c.sock); return make_error_packet(); }
  fprintf(stdout, "Got len: %x\n", p.len);
  p.data = malloc(p.len + 1);
  if ((recv(c.sock, p.data, p.len, 0)) == -1) { logg(5, log_file, "Error recieving data! %m"); close(c.sock); return make_error_packet(); }
  p.data[p.len] = '\0';
  fprintf(stdout, "Got data: %s\n", p.data);

  return p;
}

void send_packet(struct rsocket c, struct rpacket p) {
  send(c.sock, &p.func, 1, 0);
  send(c.sock, &p.len, 2, 0);
  send(c.sock, p.data, p.len, 0);
}
