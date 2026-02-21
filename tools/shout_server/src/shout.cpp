#include "shout.hpp"
#include <cctype>
#include <cstdio>
#include <cstring>
#include <sys/select.h>
#include <sys/socket.h>

#define READ_SIZE 4096

int main() {
  struct addrinfo hints;
  std::memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;

  struct addrinfo *bind_addr;
  if (getaddrinfo(0, "8080", &hints, &bind_addr)) {
    std::cerr << "getaddrinfo() failed. " << strerror(GETSOCKETERRNO())
              << std::endl;
    return (EXIT_FAILURE);
  }
  SOCKET socket_listen = socket(bind_addr->ai_family, bind_addr->ai_socktype,
                                bind_addr->ai_protocol);
  if (!ISVALIDSOCKET(socket_listen)) {
    std::cerr << "socket() failed: " << strerror(GETSOCKETERRNO()) << std::endl;
    return (EXIT_FAILURE);
  }
  if (bind(socket_listen, bind_addr->ai_addr, bind_addr->ai_addrlen)) {
    std::cerr << "bind() failed: " << strerror(GETSOCKETERRNO()) << std::endl;
    return (EXIT_FAILURE);
  }
  freeaddrinfo(bind_addr);

  std::cout << "Listening..." << std::endl;
  if (listen(socket_listen, 10) < 0) {
    std::cerr << "listen() failed: " << strerror(GETSOCKETERRNO()) << std::endl;
    return (EXIT_FAILURE);
  }
  fd_set master;
  FD_ZERO(&master);
  FD_SET(socket_listen, &master);
  SOCKET max_socket = socket_listen;
  std::cout << "Waiting for connections..." << std::endl;
  while (1) {
    fd_set reads_set;
    reads_set = master;

    if (select(max_socket + 1, &reads_set, 0, 0, 0) < 0) {
      std::cerr << "select() failed: " << strerror(GETSOCKETERRNO())
                << std::endl;
    }

    for (SOCKET i = 1; i <= max_socket; ++i) {
      if (FD_ISSET(i, &reads_set)) {
        if (i == socket_listen) {
          struct sockaddr_storage client_addr;
          socklen_t client_len = sizeof(client_addr);
          SOCKET socket_client = accept(
              socket_listen, (struct sockaddr *)&client_addr, &client_len);
          if (!ISVALIDSOCKET(socket_client)) {
            std::cerr << "accept() failed: " << strerror(GETSOCKETERRNO())
                      << std::endl;
            return (EXIT_FAILURE);
          }

          FD_SET(socket_client, &master);
          if (socket_client > max_socket)
            max_socket = socket_client;

          char addr_buff[100];
          getnameinfo((struct sockaddr *)&client_addr, client_len, addr_buff,
                      sizeof(addr_buff), 0, 0, NI_NUMERICHOST);
          std::cout << "New connection from " << addr_buff << " with ID "
                    << socket_client << "\n";
        } else {
          char read_buff[READ_SIZE];
          int bytes_received = recv(i, read_buff, READ_SIZE, 0);
          if (bytes_received < 1) {
            std::cout << "Dropping connection with ID " << i << "\n";
            FD_CLR(i, &master);
            CLOSESOCKET(i);
            continue;
          }

          for (int j = 0; j < bytes_received; ++j) {
            read_buff[j] = std::toupper(read_buff[j]);
          }
          send(i, read_buff, bytes_received, 0);
        }
      }
    }
  }

  std::cout << "Closing listening socket..." << std::endl;
  CLOSESOCKET(socket_listen);
  return (EXIT_SUCCESS);
}
