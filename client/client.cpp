#include <iostream>

#include "../socket/socket.hpp"

#define SERV_PORT 9877
#define LISTENQ   1024

int main(int argc, char* argv[])
{
   int sockfd;
   sockaddr_in servaddr;

   sockfd = socket(AF_INET, SOCK_STREAM, 0);

   memset(&servaddr, 0, sizeof(servaddr));

   servaddr.sin_family = AF_INET;
   servaddr.sin_port   = htons(SERV_PORT);
   
   inet_pton(AF_INET, "localhost", &servaddr.sin_addr);
   
   connect(sockfd, (sockaddr*) &servaddr, sizeof(servaddr));
   
   std::string msg("HELLO FROM CLIENT");
   write(sockfd, msg.c_str(), msg.size());
   //char buf[1024];
   //while(read(sockfd, buf, 1024) != 0)
   //{
   //   std::cout << buf << std::endl;
   //}

   close(sockfd);

   return 0;
}
