#include "client.hpp"
#include "lib/mime_type.h"
#include "logger/log.hpp"
#include <cstdio>
#include <fstream>
#include <new>
#include <sstream>
#include <sys/select.h>

Client::Client(SOCKET s)
    : address_len(sizeof(address)), socket(s), received(0) {
  request[received] = '\0';
}

std::string Client::get_host() const {
  char host[NI_MAXHOST];

  int ret = getnameinfo((struct sockaddr *)&address, address_len, host,
                        sizeof(host), 0, 0, NI_NUMERICHOST);
  if (ret != 0) {
    Logger::error("getnameinfo: %s", gai_strerror(ret));
    return "unknown";
  }

  return host;
}

std::map<SOCKET, Client *> ClientManager::clients;

Client *ClientManager::get_client(SOCKET s) {
  std::map<SOCKET, Client *>::iterator it = clients.find(s);

  if (it != clients.end())
    return it->second;

  Client *n = new (std::nothrow) Client(s);
  if (!n) {
    Logger::fatal("get_client: Out of memeory.");
  }
  clients[s] = n;
  return n;
}

void ClientManager::drop_client(SOCKET s) {
  std::map<SOCKET, Client *>::iterator it = clients.find(s);

  if (it != clients.end()) {
    delete it->second;
    clients.erase(it);
    CLOSESOCKET(s);
  } else {
    Logger::warn("drop_client: socket(%d) not found in clients", s);
  }
}

fd_set ClientManager::wait_on_clients(SOCKET server) {
  fd_set reads;
  FD_ZERO(&reads);
  FD_SET(server, &reads);
  SOCKET max_socket = server;

  std::map<SOCKET, Client *>::iterator it;

  for (it = clients.begin(); it != clients.end(); it++) {
    FD_SET(it->first, &reads);
    if (it->first > max_socket) {
      max_socket = it->first;
    }
  }

  if (select(max_socket + 1, &reads, 0, 0, 0) < 0) {
    Logger::fatal("select: failed. (%d)", GETSOCKETERRNO());
  }

  return reads;
}

void send_400(Client *client) {
  std::string c400 = "HTTP/1.1 400 Bad Request\r\n"
                     "Connection: close\r\n"
                     "Content-Length: 11\r\n\r\nBad Request";
  send(client->socket, c400.c_str(), c400.length(), 0);
  ClientManager::drop_client(client->socket);
}

void send_404(Client *client) {
  std::string c404 = "HTTP/1.1 404 Not Found\r\n"
                     "Connection: close\r\n"
                     "Content-Length: 9\r\n\r\nNot Found";
  send(client->socket, c404.c_str(), c404.length(), 0);
  ClientManager::drop_client(client->socket);
}

void ClientManager::serve_resource(Client *client, std::string path) {
  Logger::debug("serve_resource: %s %s", client->get_host().c_str(),
                path.c_str());

  if (path == "/")
    path = "/index.html";
  if (path.length() > 100)
    return send_400(client);
  if (path.find("..") != std::string::npos)
    return send_404(client);

  std::string full_path = "public" + path;

  // Open file using ifstream in binary mode
  std::ifstream file(full_path.c_str(), std::ios::binary | std::ios::ate);

  if (!file.is_open()) {
    return send_404(client);
  }

  // std::ios::ate (at the end) lets us get the size immediately
  std::streamsize size = file.tellg();
  file.seekg(0, std::ios::beg); // Rewind to start

  std::string ct = get_content_type(full_path);
  std::stringstream ss;

  ss << "HTTP/1.1 200 OK\r\n"
     << "Connection: close\r\n"
     << "Content-Type: " << ct << "\r\n"
     << "Content-Length: " << size << "\r\n"
     << "\r\n";
  send_all(client->socket, ss.str());

  char buffer[1024];
  while (file.read(buffer, sizeof(buffer)) || file.gcount() > 0) {
    std::streamsize bytes_read = file.gcount();

    std::streamsize total_sent = 0;
    while (total_sent < bytes_read) {
      int sent = send(client->socket, buffer + total_sent,
                      static_cast<int>(bytes_read - total_sent), 0);
      if (sent <= 0)
        goto cleanup;
      total_sent += sent;
    }
  }

cleanup:
  file.close();
  drop_client(client->socket);
}


ssize_t send_string(SOCKET s, const std::string &str) {
  return send(s, str.c_str(), str.length(), 0);
}

ssize_t send_all(SOCKET s, const std::string &str) {
  ssize_t total_sent = 0;
  const char *raw_ptr = str.c_str();

  while ((size_t)total_sent < str.length()) {
    ssize_t sent = send(s, raw_ptr + total_sent, str.length() - total_sent, 0);
    if (sent <= 0)
      return sent;
    total_sent += sent;
  }
  return total_sent;
}
