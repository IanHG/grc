#include <iostream>
#include <list>

#include "../socket/socket.hpp"
#include "../util/blocking_queue.hpp"
#include "../util/thread_pool.hpp"
#include "../util/event_handler.hpp"

#include "../commandline/commandline.hpp"

#define SERV_PORT 9877
#define LISTENQ   1024

enum events : int { message_recv };

using event_handler_type = event_handler<events, void(const std::string&)>;

struct server_internals
{
   thread_pool*  m_pool;

   event_handler_type* m_evh;
} si;

class message
{
   private:
      using user_t    = std::string;
      using message_t = std::string;
      
      //! User
      user_t    m_user    = "";
      //! Message
      message_t m_message = "";

   public:
      //!
      message() = default;

      //! Constructor
      message(const user_t& user, const message_t& msg)
         :  m_user(user)
         ,  m_message(msg)
      {
      }

      const user_t& get_user() const
      {
         return m_user;
      }

      const message_t& get_message() const
      {
         return m_message;
      }
};

class connection
{
   private:
      using mutex_t = std::mutex;

      int     m_connfd;
      
      mutex_t m_write_mutex;

      mutex_t m_read_mutex;
   public:
      connection(int connfd)
         :  m_connfd(connfd)
      {
         setup_listener();
      }

      void setup_listener()
      {
         si.m_pool->submit([this](){
            //std::cout << " LOL HERE " << std::endl;
            char buff[1024];
            if(read(m_connfd, buff, 1024) != EWOULDBLOCK)
            {
               std::string str(buff);
               std::cout << " HERE LOL " << std::endl;
               std::cout << buff << std::endl;
               si.m_evh->handle_event(events::message_recv, *si.m_pool, str);
            }
            this->setup_listener();
         });
      }

      void send_message(const message& msg)
      {
         std::lock_guard<mutex_t> lock(m_write_mutex);

         auto& user = msg.get_user();
         if(write(m_connfd, user.c_str(), user.size()) != user.size())
         {
            std::cout << " PROBLEM WRITING USER " << std::endl;
         }

         auto& str  = msg.get_message();
         if(write(m_connfd, str.c_str(), str.size()) != str.size())
         {
            std::cout << " PROBLEM WRITING MESSAGE " << std::endl;
         }
      }

      ~connection()
      {
         close(m_connfd);
      }
};

class connection_handler
{
   private:
      using connection_list_t = std::list<connection>;

      thread_pool&      m_thread_pool;
      connection_list_t m_connection_list;

   public:
      //! Default c-tor.
      connection_handler(thread_pool& pool)
         :  m_thread_pool(pool)
      {
      }
      
      //! Default d-tor.
      ~connection_handler() = default;

      //!
      void add_connection(int conn)
      {
         m_connection_list.emplace_back(conn);
      }

      ////!
      //void handle_messages(message_queue& q)
      //{
      //   while(!q.empty())
      //   {
      //      while(!q.empty())
      //      {
      //         // Get messages
      //         message_queue::data_type data;
      //         q.pop_try_wait(data); 

      //         // Handle messages
      //         for(auto& conn : m_connection_list)
      //         {
      //            m_thread_pool.submit([&conn, data](){
      //               conn.send_message(data);
      //            });
      //         }
      //      }
      //   }
      //}
};

int main(int argc, char* argv[])
{
   auto cc = commandline::parser()
      .option<int>("pool", "--pool")
      .parse(argc, argv);

   auto npool = cc.has("pool") ? cc.get<int>("pool") : 2;

   thread_pool pool(npool);
   event_handler_type evh;
   si.m_pool = &pool;
   si.m_evh  = &evh;

   si.m_evh->register_function(events::message_recv, [](const std::string& str){ std::cout << str << std::endl;} );

   connection_handler connhand(pool);

   int listenfd, connfd;

   socklen_t clilen;
   sockaddr_in cliaddr, servaddr;

   listenfd = socket(AF_INET, SOCK_STREAM, 0);

   memset(&servaddr, 0, sizeof(servaddr));
   servaddr.sin_family      = AF_INET;
   servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
   servaddr.sin_port        = htons(SERV_PORT);

   bind(listenfd, (sockaddr*) &servaddr, sizeof(servaddr));

   listen(listenfd, LISTENQ); 

   while(true)
   {
      clilen = sizeof(cliaddr);
      connfd = accept(listenfd, (sockaddr*) &cliaddr, &clilen);
      std::cout << " CONNECTION ACCEPTED " << std::endl;
      int status = fcntl(connfd, F_SETFL, fcntl(connfd, F_GETFL, 0) | O_NONBLOCK);

      std::cout << " ADDING CONNECTION " << std::endl;

      connhand.add_connection(connfd);

      //pool.submit([connfd](){
      //   std::cout << " CONNECTION ACCEPTED " << std::endl;
      //   close(connfd);
      //});
   }

   return 0;
}
