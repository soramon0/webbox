#include "client.hpp"
#include "lib/mime_type.h"
#include "logger/log.hpp"
#include "server/headers_builder.hpp"
#include "server/send.hpp"
#include <cstdio>
#include <fstream>
#include <new>
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
  std::string body = "<html><body><h1>400 Bad Request</h1></body></html>";

  ssize_t sent = HeadersBuilder(client->socket, "400 Bad Request")
                     .with("Content-Type", "text/html")
                     .with("Content-Length", body.length())
                     .with("Connection", "close")
                     .send();

  if (sent > 0) {
    send_all(client->socket, body);
  }

  ClientManager::drop_client(client->socket);
}

void send_404(Client *client) {
  std::string body = "<html><body><h1>404 Not Found</h1><p>The requested "
                     "resource was not found on this server.</p></body></html>";

  ssize_t sent = HeadersBuilder(client->socket, "404 Not Found")
                     .with("Content-Type", "text/html")
                     .with("Content-Length", body.length())
                     .with("Connection", "close")
                     .send();

  if (sent > 0) {
    send_all(client->socket, body);
  }

  ClientManager::drop_client(client->socket);
}

void ClientManager::serve_resource(Client *client, std::string path) {
  std::string host = client->get_host();
  Logger::debug("serve_resource: %s %s", host.c_str(), path.c_str());

  if (path == "/")
    path = "/index.html";
  if (path.length() > 100)
    return send_400(client);
  if (path.find("..") != std::string::npos)
    return send_404(client);

  std::string full_path = "public" + path;
  std::ifstream file(full_path.c_str(), std::ios::binary | std::ios::ate);
  if (!file.is_open()) {
    return send_404(client);
  }

  // std::ios::ate (at the end) lets us get the size immediately
  std::streamsize size = file.tellg();
  file.seekg(0, std::ios::beg); // Rewind to start

  ssize_t headers_sent = HeadersBuilder(client->socket, "200 OK")
                             .with("Connection", "close")
                             .with("Content-Type", get_content_type(full_path))
                             .with("Content-Length", static_cast<size_t>(size))
                             .send();
  if (headers_sent <= 0) {
    if (headers_sent < 0) {
      Logger::error("serve_resource(): could not send headers %s %s",
                    host.c_str(), path.c_str());
    }
    return;
  }

  char buffer[1024];
  while (file.read(buffer, sizeof(buffer)) || file.gcount() > 0) {
    size_t bytes_read = static_cast<size_t>(file.gcount());
    ssize_t bytes_sent = send_all(client->socket, buffer, bytes_read);
    if (bytes_sent <= 0) {
      if (bytes_sent < 0) {
        Logger::error("serve_resource(): could not send body %s %s",
                      host.c_str(), path.c_str());
      }
      return drop_client(client->socket);
    }
  }
  drop_client(client->socket);
}
