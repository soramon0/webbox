#include "logger/log.hpp"
#include "server/socket.hpp"

int main() {
  if (DEBUG) {
    Logger::setLevel(Logger::LOG_DEBUG);
  }

  char host[] = "0.0.0.0";
  char port[] = "9000";
  SOCKET listen_socket = create_socket(host, port);
  if (listen_socket == -1) {
    Logger::fatal("could not listen on %s:%s", host, port);
  }
  close(listen_socket);

  return 0;
}
