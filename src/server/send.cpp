#include "send.hpp"

ssize_t send_all(SOCKET s, const char *buf, size_t size) {
  ssize_t total_sent = 0;
  while (static_cast<size_t>(total_sent) < size) {
    ssize_t sent = send(s, buf + total_sent, size - total_sent, 0);
    if (sent <= 0)
      return sent;
    total_sent += sent;
  }
  return total_sent;
}

ssize_t send_all(SOCKET s, const std::string &str) {
  return send_all(s, str.c_str(), str.length());
}
