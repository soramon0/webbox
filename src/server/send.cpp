#include "send.hpp"
#include "server/client.hpp"
#include "server/headers_builder.hpp"

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

void send_400(SOCKET s) {
  std::string body = "<html><body><h1>400 Bad Request</h1></body></html>";

  ssize_t sent = HeadersBuilder(s, "400 Bad Request")
                     .with("Content-Type", "text/html")
                     .with("Content-Length", body.length())
                     .with("Connection", "close")
                     .send();

  if (sent > 0) {
    send_all(s, body);
  }

  ClientManager::drop_client(s);
}

void send_404(SOCKET s) {
  std::string body = "<html><body><h1>404 Not Found</h1><p>The requested "
                     "resource was not found on this server.</p></body></html>";

  ssize_t sent = HeadersBuilder(s, "404 Not Found")
                     .with("Content-Type", "text/html")
                     .with("Content-Length", body.length())
                     .with("Connection", "close")
                     .send();

  if (sent > 0) {
    send_all(s, body);
  }

  ClientManager::drop_client(s);
}
