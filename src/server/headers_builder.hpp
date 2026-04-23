#pragma once

#include "common.h"
#include "server/send.hpp"
#include <sstream>
#include <string>

#define HTTP_VERSION "HTTP/1.1"

class HeadersBuilder {
private:
  SOCKET socket;
  std::string buff;

public:
  inline HeadersBuilder(SOCKET s, const std::string &version,
                        const std::string &status)
      : socket(s) {
    buff.reserve(512);
    buff += version + " " + status + "\r\n";
  }
  inline HeadersBuilder(SOCKET s, const std::string &status) : socket(s) {
    buff.reserve(512);
    buff += HTTP_VERSION;
    buff += " " + status + "\r\n";
  }

  inline HeadersBuilder &with(const std::string &name,
                              const std::string &value) {
    buff += name + ": " + value + "\r\n";
    return *this;
  }

  inline HeadersBuilder &with(const std::string &name, size_t value) {
    std::stringstream ss;
    ss << value;
    buff += name + ": " + ss.str() + "\r\n";
    return *this;
  }

  inline ssize_t send() {
    buff += "\r\n";
    ssize_t result = send_all(socket, buff);
    buff.clear();
    return result;
  }
};
