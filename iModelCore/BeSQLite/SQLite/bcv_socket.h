

#if defined(_WIN32)
#define __WIN32__
#endif

#ifdef __WIN32__
#ifndef _SSIZE_T_DEFINED
#ifdef  _WIN64
typedef unsigned __int64 ssize_t;
#else
typedef _W64 unsigned int ssize_t;
#endif
#define _SSIZE_T_DEFINED
#endif
#endif

#ifdef __WIN32__
#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#define BCV_SOCKET_TYPE SOCKET
#else
#include <sys/socket.h> 
#include <arpa/inet.h> 
#define BCV_SOCKET_TYPE int
#endif

static int bcv_socket_is_valid(BCV_SOCKET_TYPE s){
#ifdef __WIN32__
  return (s!=INVALID_SOCKET);
#else
  return (s>=0);
#endif
}

static void bcv_close_socket(BCV_SOCKET_TYPE s){
#ifdef __WIN32__
  shutdown(s, SD_BOTH);
  closesocket(s);
#else
  shutdown(s, SHUT_RDWR);
  close(s);
#endif
}

static void bcv_socket_init(){
#ifdef __WIN32__
  WORD v;
  WSADATA wsa;
  v = MAKEWORD(2,2);
  WSAStartup(v, &wsa);
#endif
}

