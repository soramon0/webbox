#include "client.hpp"
#include <cstring>
#include <netdb.h>

int main(int argc, char *argv[]) {
  if (argc < 3) {
    std::cerr << "usage: " << APP_NAME " hostname port" << std::endl;
    return (EXIT_FAILURE);
  }
  struct addrinfo hints;
  std::memset(&hints, 0, sizeof(hints));
  hints.ai_socktype = SOCK_STREAM;
  struct addrinfo *peer_address;
  if (getaddrinfo(argv[1], argv[2], &hints, &peer_address)) {
    std::cerr << "getaddrinfo() failed. " << GETSOCKETERRNO() << std::endl;
    return (EXIT_FAILURE);
  }
  char address_buff[100];
  char service_buff[100];
  std::cout << "Connecting to ";
  getnameinfo(peer_address->ai_addr, peer_address->ai_addrlen, address_buff,
              sizeof(address_buff), service_buff, sizeof(service_buff),
              NI_NUMERICHOST);
  std::cout << address_buff << " " << service_buff << std::endl;
  return (EXIT_SUCCESS);
}
