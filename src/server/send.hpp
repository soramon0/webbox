#pragma once

#include "common.h"
#include <string>

ssize_t send_all(SOCKET s, const char *buf, size_t size);
ssize_t send_all(SOCKET s, const std::string &str);
