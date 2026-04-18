#include "server/socket.hpp"

int main() {
  SOCKET listen_socket = create_socket("0.0.0.0", "9000");
  close(listen_socket);

  return 0;
}
