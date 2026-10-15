/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for license terms and full copyright notice.
*--------------------------------------------------------------------------------------------*/

#include <errno.h>
#include <fcntl.h>
#include <netinet/tcp.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>

static int failSetOption;
static int optionCalls;
static int optionSocket = -1;

static int testSetSocketOption(
  int fd, int level, int option, const void *value, socklen_t size
){
  optionCalls++;
  optionSocket = fd;
  if( failSetOption ){
    errno = ENOPROTOOPT;
    return -1;
  }
  return setsockopt(fd, level, option, value, size);
}

#define setsockopt testSetSocketOption
#include "../../SQLite/blockcachevfs.c"
#undef setsockopt

#define CHECK(condition) do { \
  if( !(condition) ){ \
    fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #condition); \
    exit(EXIT_FAILURE); \
  } \
} while(0)

static int makeServer(char *address, int listening){
  struct sockaddr_in addr;
  socklen_t size = sizeof(addr);
  int fd = socket(AF_INET, SOCK_STREAM, 0);
  CHECK(fd>=0);
  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
  CHECK(bind(fd, (struct sockaddr*)&addr, sizeof(addr))==0);
  CHECK(getsockname(fd, (struct sockaddr*)&addr, &size)==0);
  if( listening ){
    CHECK(listen(fd, 1)==0);
  }
  snprintf(address, 64, "127.0.0.1:%u", ntohs(addr.sin_port));
  return fd;
}

static void testClientNoDelay(void){
  char address[64];
  char *error = 0;
  int server = makeServer(address, 1);
  int rc = SQLITE_OK;
  int value = 0;
  socklen_t size = sizeof(value);
  int client = bcvConnectSocket(&rc, address, &error);
  CHECK(rc==SQLITE_OK);
  CHECK(error==0);
  CHECK(client>=0);
  CHECK(getsockopt(client, IPPROTO_TCP, TCP_NODELAY, &value, &size)==0);
  CHECK(value==1);
  CHECK(optionCalls==1);
  {
    int accepted = accept(server, 0, 0);
    CHECK(accepted>=0);
    value = 1;
    CHECK(getsockopt(accepted, IPPROTO_TCP, TCP_NODELAY, &value, &size)==0);
    CHECK(value==0);
    bcv_close_socket(accepted);
  }
  bcv_close_socket(client);
  bcv_close_socket(server);
}

static void testOptionFailure(void){
  char address[64];
  char *error = 0;
  int server = makeServer(address, 1);
  int rc = SQLITE_OK;
  int before = optionCalls;
  failSetOption = 1;
  CHECK(bcvConnectSocket(&rc, address, &error)==INVALID_SOCKET);
  CHECK(rc==BCVFS_CANTOPEN);
  CHECK(error!=0);
  CHECK(strstr(error, address)!=0);
  CHECK(optionCalls==before+1);
  CHECK(fcntl(optionSocket, F_GETFD)==-1);
  CHECK(errno==EBADF);
  sqlite3_free(error);
  failSetOption = 0;
  bcv_close_socket(server);
}

static void testConnectionFailure(void){
  char address[64];
  char *error = 0;
  int server = makeServer(address, 0);
  int rc = SQLITE_OK;
  CHECK(bcvConnectSocket(&rc, address, &error)==INVALID_SOCKET);
  CHECK(rc==BCVFS_CANTOPEN);
  CHECK(error!=0);
  CHECK(fcntl(optionSocket, F_GETFD)==-1);
  CHECK(errno==EBADF);
  sqlite3_free(error);
  bcv_close_socket(server);
}

static void testExistingError(void){
  char *error = 0;
  int rc = SQLITE_NOMEM;
  int before = optionCalls;
  CHECK(bcvConnectSocket(&rc, "127.0.0.1:1", &error)==INVALID_SOCKET);
  CHECK(rc==SQLITE_NOMEM);
  CHECK(error==0);
  CHECK(optionCalls==before);
}

int main(void){
  CHECK(sqlite3_initialize()==SQLITE_OK);
  testClientNoDelay();
  testOptionFailure();
  testConnectionFailure();
  testExistingError();
  CHECK(sqlite3_shutdown()==SQLITE_OK);
  puts("CloudSQLite client socket tests passed");
  return EXIT_SUCCESS;
}
