#include <iostream>
#include <list>

#include "../socket/socket.hpp"
#include "../socket/datagram.hpp"
#include "../util/blocking_queue.hpp"
#include "../util/thread_pool.hpp"
#include "../util/event_handler.hpp"
#include "../util/timer_queue.hpp"

#include "../commandline/commandline.hpp"

#define SERV_PORT 9877
#define LISTENQ   1024

enum events : int { message_recv };

using event_handler_type = event_handler<events, void(const datagram&)>;
using work_queue_type    = timer_queue;

struct server_internals
{
   work_queue_type*  m_queue;

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
   :  public event_registerable<event_handler_type>
{
   private:
      int m_connfd;

      datagram_reader m_reader;
      datagram_writer m_writer;
      
      std::atomic_bool m_running = true;
      //std::atomic_bool m_listener_stopped = false;
      //std::atomic_bool m_sender_stopped   = false;

      void close_connection()
      {
         //std::cout << " CLOSING CONNECTION " << std::endl;
         event_registerable<event_handler_type>::deregister_functions(*si.m_evh);
         m_running = false;
         close(m_connfd);
      }

      void send_datagram_impl(const std::shared_ptr<datagram>& pdg)
      {
         si.m_queue->add(100ms, [this, pdg](bool){
            //std::cout << " TRYING TO WRITE\n " << *pdg << std::endl;
            if(!m_writer.try_write_fd(m_connfd, *pdg))
            {
               if(m_running)
               {
                  send_datagram_impl(pdg);
               }
            }
         });
      }

   public:
      connection(int connfd)
         :  m_connfd(connfd)
      {
         event_registerable<event_handler_type>::register_function(*si.m_evh, events::message_recv, [this](const datagram& dg){ 
            //std::cout << " FIRING EVENT !!\n" << dg << std::endl;
            this->send_datagram(dg);
         } );
         si.m_evh->handle_noevent();
         setup_listener();
      }

      void setup_listener()
      {
         //std::cout << " SETTING UP LISTENER " << std::endl;
         si.m_queue->add(100ms, [this](bool){
            datagram dg;
            m_reader.try_read_fd(m_connfd);
            m_reader.pop_try_wait(dg);

            switch(dg.m_data_type) 
            {
               case datagram::message:
                  //std::cout << " RECIEVED DATAGRAM MESSAGE" << std::endl;
                  si.m_evh->handle_event_async(events::message_recv, *si.m_queue, std::move(dg));
                  //si.m_evh->handle_event(events::message_recv, dg);
                  break;
               case datagram::close:
                  //std::cout << " RECIEVED DATAGRAM CLOSE" << std::endl;
                  this->close_connection();
                  break;
               case datagram::nodata:
               default:
                  ;
            }
            if(m_running)
            {
               this->setup_listener();
            }
         });
      }

      void send_datagram(const datagram& dg)
      {
         auto pdg = dg.copy_to_shared();

         send_datagram_impl(pdg);
      }

      bool is_up()
      {
         return m_running;
      }

      ~connection()
      {
         close_connection();
      }
};

class connection_handler
{
   private:
      using connection_list_t = std::list<connection>;

      //thread_pool&      m_thread_pool;
      connection_list_t m_connection_list;

   public:
      ////! Default c-tor.
      //connection_handler(thread_pool& pool)
      //   :  m_thread_pool(pool)
      //{
      //}
      connection_handler() = default;
      
      //! Default d-tor.
      ~connection_handler() = default;

      //!
      void add_connection(int conn)
      {
         m_connection_list.emplace_back(conn);
      }

      void clean()
      {
         for(auto iter = m_connection_list.begin(); iter != m_connection_list.end(); )
         {
            if(!iter->is_up())
            {
               iter = m_connection_list.erase(iter);
            }
            else
            {
               ++iter;
            }
         }
      }
};

int main(int argc, char* argv[])
{
   auto cc = commandline::parser()
      .option<int>("pool", "--pool")
      .parse(argc, argv);

   auto npool = cc.has("pool") ? cc.get<int>("pool") : 4;

   work_queue_type pool(npool);
   event_handler_type evh;
   si.m_queue = &pool;
   si.m_evh   = &evh;

   si.m_evh->register_function(events::message_recv, [](const datagram& dg){ 
      std::cout << dg << std::endl;
   } );
   si.m_evh->handle_noevent();
   
   connection_handler connhand;

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
      //connhand.clean();

      //pool.submit([connfd](){
      //   std::cout << " CONNECTION ACCEPTED " << std::endl;
      //   close(connfd);
      //});
   }

   return 0;
}
