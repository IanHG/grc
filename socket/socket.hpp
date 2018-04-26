#ifndef SOCKET_H_INCLUDED
#define SOCKET_H_INCLUDED

#include <unistd.h>  

#if defined (__unix__)

#include <sys/types.h>  /* basic system datatypes */
#include <sys/socket.h> /* basic socket stuff */
#include <netinet/in.h> /* sockaddr_in and other internet definitions */
#include <arpa/inet.h>  /* inet(3) functions */

#define SOCKFD_TYPE int

#elif defined (__WIN65) || defined (__WIN32)
#define SOCKFD_TYPE SOCKET
#endif

#include <string.h> /* for memset */
#include <fcntl.h>

#endif /* SOCKET_H_INCLUDED */
