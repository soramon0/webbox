#pragma once

#include "common.h"
#include <map>
#include <string>

#define MAX_REQUEST_SIZE 2047

class Client {
public:
  socklen_t address_len;
  struct sockaddr_storage address;
  SOCKET socket;
  char request[MAX_REQUEST_SIZE + 1];
  int received;

  Client(SOCKET s);

  std::string get_host() const;

private:
  Client();
};

class ClientManager {
private:
  static std::map<SOCKET, Client *> clients;

public:
  static Client *get_client(SOCKET s);
  static void drop_client(SOCKET s);
  static fd_set wait_on_clients(SOCKET server);
  static void serve_resource(Client *client, std::string path);
  static ssize_t send_all(SOCKET s, const char *buf, size_t len);
  static ssize_t send_all(SOCKET s, const std::string &str);
};
