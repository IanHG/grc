#include <iostream>
#include <thread>

#include "../socket/socket.hpp"
#include "../socket/datagram.hpp"

#define SERV_PORT 9877
#define LISTENQ   1024

int main(int argc, char* argv[])
{
   using namespace std::chrono_literals;
   int sockfd;
   sockaddr_in servaddr;

   sockfd = socket(AF_INET, SOCK_STREAM, 0);

   memset(&servaddr, 0, sizeof(servaddr));

   servaddr.sin_family = AF_INET;
   servaddr.sin_port   = htons(SERV_PORT);
   
   inet_pton(AF_INET, "localhost", &servaddr.sin_addr);
   
   connect(sockfd, (sockaddr*) &servaddr, sizeof(servaddr));
   int status = fcntl(sockfd, F_SETFL, fcntl(sockfd, F_GETFL, 0) | O_NONBLOCK);
   
   std::unique_ptr<char[]> ptr{ new char[10] };
   ptr[0] = 'H';
   ptr[1] = 'E';
   ptr[2] = 'L';
   ptr[3] = 'L';
   ptr[4] = 'O';
   ptr[5] = '!';
   datagram dg(datagram::message, std::move(ptr), 6);
   datagram_writer dw;
   datagram_reader dr;
   dw.try_write_fd(sockfd, dg);
   
   
   while(true)
   {
      datagram dg_recv;
      dr.try_read_fd(sockfd);
      dr.pop_try_wait(dg_recv);
      if(dg_recv.m_data_type != datagram::nodata)
      {
         std::cout << dg_recv << std::endl;
         //break;
      }
      std::this_thread::sleep_for(500ms);
   }

   datagram dg_close(datagram::close);
   dw.try_write_fd(sockfd, dg_close);
   
   //std::this_thread::sleep_for(2s);
   
   //write(sockfd, msg.c_str(), msg.size());
   //char buf[1024];
   //while(read(sockfd, buf, 1024) != 0)
   //{
   //   std::cout << buf << std::endl;
   //}

   close(sockfd);

   return 0;
}
