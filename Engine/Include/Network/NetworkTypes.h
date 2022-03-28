#pragma once

#include <stdint.h>

#if PLATFORM_WINDOWS
#include <winsock.h>
#elif PLATFORM_LINUX
#include <sys/socket.h>
#include <netdb.h>
#include <sys/ioctl.h>
#elif PLATFORM_3DS
#include <sys/socket.h>
#include <netdb.h>
#include <sys/ioctl.h>
#elif PLATFORM_DOLPHIN
#include <network.h>
#elif PLATFORM_ANDROID
#include <sys/socket.h>
#include <netdb.h>
#include <sys/ioctl.h>
#endif

#if PLATFORM_WINDOWS
    typedef SOCKET SocketHandle;
#elif PLATFORM_LINUX
    typedef int32_t SocketHandle;
#elif PLATFORM_3DS
    typedef int32_t SocketHandle;
#elif PLATFORM_DOLPHIN
    typedef int32_t SocketHandle;
#endif

