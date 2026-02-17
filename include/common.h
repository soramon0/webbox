#pragma once

#include <iostream>

#define APP_NAME "Webserve"
#define ISVALIDSOCKET(s) (s >= 0)
#define CLOSESOCKET(s) close(s)
#define SOCKET int
#define GETSOCKETERRNO() (errno)

