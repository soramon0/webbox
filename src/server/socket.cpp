#include "socket.hpp"
#include "logger/log.hpp"
#include <cstring>

SOCKET create_socket(std::string host, std::string port) {
  Logger::debug("Configuring address...");

  struct addrinfo hints;
  std::memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;

  struct addrinfo *bind_address;
  if (getaddrinfo(host.c_str(), port.c_str(), &hints, &bind_address)) {
    Logger::fatal("getaddrinfo() failed. (%d)", GETSOCKETERRNO());
  }

  Logger::debug("Creating socket...");
  SOCKET socket_listen =
      socket(bind_address->ai_family, bind_address->ai_socktype,
             bind_address->ai_protocol);

  if (!ISVALIDSOCKET(socket_listen)) {
    Logger::fatal("socket() failed. (%d)", GETSOCKETERRNO());
  }

  if (bind(socket_listen, bind_address->ai_addr, bind_address->ai_addrlen)) {
    Logger::fatal("bind() failed. (%d)", GETSOCKETERRNO());
  }
  freeaddrinfo(bind_address);

  Logger::info("Listening on http://%s:%s", host.c_str(), port.c_str());
  if (listen(socket_listen, 10) < 0) {
    Logger::fatal("listen() failed. (%d)", GETSOCKETERRNO());
  }
  return socket_listen;
}
