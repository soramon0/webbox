#include "client.hpp"
#include <cstdio>
#include <cstring>
#include <sys/select.h>
#include <unistd.h>

#define READ_SIZE 4096

int main(int argc, char *argv[]) {
  if (argc < 3) {
    std::cerr << "usage: " << APP_NAME " hostname port" << std::endl;
    return (EXIT_FAILURE);
  }
  struct addrinfo hints;
  std::memset(&hints, 0, sizeof(hints));
  hints.ai_socktype = SOCK_STREAM;
  struct addrinfo *address;
  if (getaddrinfo(argv[1], argv[2], &hints, &address)) {
    std::cerr << "getaddrinfo() failed. " << strerror(GETSOCKETERRNO())
              << std::endl;
    return (EXIT_FAILURE);
  }
  char address_buff[100];
  char service_buff[100];
  std::cout << "Connecting to ";
  getnameinfo(address->ai_addr, address->ai_addrlen, address_buff,
              sizeof(address_buff), service_buff, sizeof(service_buff),
              NI_NUMERICHOST);
  std::cout << address_buff << " " << service_buff << std::endl;
  SOCKET socket_peer =
      socket(address->ai_family, address->ai_socktype, address->ai_protocol);
  if (!ISVALIDSOCKET(socket_peer)) {
    std::cerr << "socket() failed: " << strerror(GETSOCKETERRNO()) << std::endl;
    return (EXIT_FAILURE);
  }
  if (connect(socket_peer, address->ai_addr, address->ai_addrlen)) {
    std::cerr << "connect() failed: " << strerror(GETSOCKETERRNO())
              << std::endl;
    return (EXIT_FAILURE);
  }
  freeaddrinfo(address);

  std::cout << "Connected." << std::endl;
  std::cout << "To send data, enter text followed by enter." << std::endl;
  while (1) {
    fd_set reads_set;
    FD_ZERO(&reads_set);
    FD_SET(socket_peer, &reads_set);
    FD_SET(STDOUT_FILENO, &reads_set);

    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 100000;

    if (select(socket_peer + 1, &reads_set, 0, 0, &timeout) < 0) {
      std::cerr << "select() failed: " << strerror(GETSOCKETERRNO())
                << std::endl;
      return (EXIT_FAILURE);
    }

    if (FD_ISSET(socket_peer, &reads_set)) {
      char read_buff[READ_SIZE];
      int bytes_received = recv(socket_peer, read_buff, READ_SIZE, 0);
      if (bytes_received < 1) {
        std::cout << "Connection closed by peer." << std::endl;
        break;
      }
      std::cout << "Received (" << bytes_received << " bytes): ";
      std::cout.write(read_buff, bytes_received);
    }

    if (FD_ISSET(STDIN_FILENO, &reads_set)) {
      char read_buff[READ_SIZE];
      if (!fgets(read_buff, READ_SIZE, stdin))
        break;
      std::cout << "Sending: " << read_buff;
      int bytes_sent = send(socket_peer, read_buff, strlen(read_buff), 0);
      std::cout << "Sent " << bytes_sent << " bytes.\n";
    }
  }
  return (EXIT_SUCCESS);
}
