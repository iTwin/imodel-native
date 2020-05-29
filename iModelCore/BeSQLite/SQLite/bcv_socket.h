

#if !defined(__WIN32__) && (defined(WIN32) || defined(_WIN32))
# define __WIN32__
#endif

#ifdef __WIN32__
# include <winsock2.h>
# include <windows.h>
# include <ws2tcpip.h>
# define BCV_SOCKET_TYPE SOCKET
#else
# include <unistd.h>
# include <sys/socket.h> 
# include <arpa/inet.h> 
# define BCV_SOCKET_TYPE int
#endif

int bcv_socket_is_valid(BCV_SOCKET_TYPE s);
void bcv_close_socket(BCV_SOCKET_TYPE s);
void bcv_socket_init();

