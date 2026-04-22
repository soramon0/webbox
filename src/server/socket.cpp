#include "socket.hpp"
#include "logger/log.hpp"
#include <cstring>

SOCKET create_socket(const char *host, const char *port) {
  Logger::debug("Configuring address...");
  struct addrinfo hints;
  std::memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;

  struct addrinfo *bind_address;
  if (getaddrinfo(host, port, &hints, &bind_address)) {
    Logger::error("getaddrinfo: failed. (%d)", GETSOCKETERRNO());
    return (-1);
  }

  Logger::debug("Creating socket...");
  SOCKET socket_listen =
      socket(bind_address->ai_family, bind_address->ai_socktype,
             bind_address->ai_protocol);

  if (!ISVALIDSOCKET(socket_listen)) {
    Logger::error("socket: failed. (%d)", GETSOCKETERRNO());
    return (-1);
  }

  if (bind(socket_listen, bind_address->ai_addr, bind_address->ai_addrlen)) {
    Logger::error("bind: failed. (%d)", GETSOCKETERRNO());
    return (-1);
  }
  freeaddrinfo(bind_address);

  Logger::info("Listening on http://%s:%s", host, port);
  if (listen(socket_listen, 10) < 0) {
    Logger::error("listen: failed. (%d)", GETSOCKETERRNO());
    return (-1);
  }
  return socket_listen;
}
