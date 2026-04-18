#include "lib/mime_type.h"
#include "server/socket.hpp"

int main() {
  std::cout << "Source code for " << get_content_type("main.html") << std::endl;

  SOCKET listen_socket = create_socket("0.0.0.0", "9000");
  close(listen_socket);

  return 0;
}
