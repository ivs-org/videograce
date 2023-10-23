/**
 * SockCommon.h - Contains common socket's types
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2023
 */

#pragma once

#ifndef _WIN32
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <unistd.h>

#define SOCKET int
#define INVALID_SOCKET  (SOCKET)(~0)
#define SOCKET_ERROR            (-1)
#define GET_LAST_ERROR errno

#else
#define socklen_t int
#define GET_LAST_ERROR WSAGetLastError()
#endif
