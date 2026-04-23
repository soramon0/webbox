#include "common.h"
#include "logger/log.hpp"
#include "server/client.hpp"
#include "server/send.hpp"
#include "server/socket.hpp"
#include <cstring>
#include <map>
#include <sys/select.h>

int main() {
  if (DEBUG) {
    Logger::setLevel(Logger::LOG_DEBUG);
  }

  char host[] = "0.0.0.0";
  char port[] = "9000";
  SOCKET server_socket = create_socket(host, port);
  if (server_socket == -1) {
    Logger::fatal("could not listen on %s:%s", host, port);
  }
  while (true) {
    fd_set reads = ClientManager::wait_on_clients(server_socket);
    if (FD_ISSET(server_socket, &reads)) {
      Client *client = ClientManager::get_client(-1);
      client->socket =
          accept(server_socket, (struct sockaddr *)&client->address,
                 &client->address_len);
      if (!ISVALIDSOCKET(client->socket)) {
        ClientManager::drop_client(-1);
        Logger::error("could not accept connection. (%d)", GETSOCKETERRNO());
      } else {
        ClientManager::clients.erase(-1);
        ClientManager::clients[client->socket] = client;
        Logger::debug("new connection: %s", client->get_host().c_str());
      }
    }

    std::map<SOCKET, Client *>::iterator it;

    for (it = ClientManager::clients.begin();
         it != ClientManager::clients.end();) {
      std::map<SOCKET, Client *>::iterator curr = it++;
      Client *c = curr->second;

      // Only handle clients that are ready for reading
      if (!FD_ISSET(c->socket, &reads))
        continue;

      if (c->received >= MAX_REQUEST_SIZE) {
        send_400(c->socket);
        continue;
      }
      ssize_t r = recv(c->socket, c->request + c->received,
                       MAX_REQUEST_SIZE - c->received, 0);
      if (r < 1) {
        Logger::debug("%s disconnected.", c->get_host().c_str());
        ClientManager::drop_client(c->socket);
        continue;
      }
      c->received += r;
      c->request[c->received] = '\0';

      char *q = std::strstr(c->request, "\r\n\r\n");
      if (!q) {
        continue;
      }

      if (std::strncmp("GET /", c->request, 5)) {
        send_400(c->socket);
        continue;
      }

      char *path = c->request + 4;
      char *path_end = std::strstr(path, " ");
      if (!path_end) {
        send_400(c->socket);
      } else {
        *path_end = '\0';
        ClientManager::serve_resource(c, path);
      }
    }
  }

  Logger::debug("Shuttdding down server...");
  CLOSESOCKET(server_socket);

  return 0;
}
