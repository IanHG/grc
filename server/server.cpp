#include <iostream>
#include <list>

#include "../socket/socket.hpp"
#include "../util/blocking_queue.hpp"
#include "../util/thread_pool.hpp"

#include "../commandline/commandline.hpp"

#define SERV_PORT 9877
#define LISTENQ   1024

class server_thread_pool
   :  public thread_pool
   //,  public singleton<server_thread_pool>
{
};

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

class message_queue
   : public blocking_queue<message>
{
   private:
      using queue_t = blocking_queue<message>;

   public:
      //!
      message_queue() = default;
      
      //! Add a message to the queue.
      auto add_message(const message& msg)
      {
         return queue_t::push(msg);
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

      //!
      void handle_messages(message_queue& q)
      {
         while(!q.empty())
         {
            while(!q.empty())
            {
               // Get messages
               message_queue::data_type data;
               q.pop_try_wait(data); 

               // Handle messages
               for(auto& conn : m_connection_list)
               {
                  m_thread_pool.submit([&conn, data](){
                     conn.send_message(data);
                  });
               }
            }
         }
      }
};

int main(int argc, char* argv[])
{
   auto cc = commandline::parser()
      .option<int>("pool", "--pool")
      .parse(argc, argv);

   auto npool = cc.has("pool") ? cc.get<int>("pool") : 2;

   thread_pool pool(npool);

   connection_handler connhand(pool);

   message_queue msg_queue;
   msg_queue.add_message({"lol","lol"});

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

      connhand.add_connection(connfd);

      connhand.handle_messages(msg_queue);

      //pool.submit([connfd](){
      //   std::cout << " CONNECTION ACCEPTED " << std::endl;
      //   close(connfd);
      //});
   }

   return 0;
}
