#include <memory>
#include <tuple>
#include <iostream>
#include <string.h> /* for memcpy */
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "socket.hpp"
#include "../util/blocking_queue.hpp"

struct datagram
{
   public:
      //! Data buffer type
      using data_buffer_type = std::unique_ptr<char[]>;
      
      //! Special message types
      enum datagram_type : int { nodata, message, close };
      
      //! Special message constants
      static const char soh = 0b00000001;
      static const char eot = 0b00000100;

   public:
      datagram_type    m_data_type{datagram_type::nodata};
      data_buffer_type m_data     {nullptr};
      int              m_data_size{0};
   
   public:
      datagram() = default;

      datagram(datagram_type data_type)
         :  m_data_type(data_type)
      {
      }

      datagram(datagram_type data_type, data_buffer_type&& data, int data_size)
         :  m_data_type(data_type)
         ,  m_data(std::move(data))
         ,  m_data_size(data_size)
      {
      }

      datagram(datagram&&) = default;

      datagram& operator=(datagram&&) = default;

      std::tuple<data_buffer_type, int> convert_to_message() const
      {
         int message_size = m_data_size + 4 + 2;
         data_buffer_type message{new char[message_size]};

         message[0]                = soh;
         message[message_size - 1] = eot;

         memcpy(message.get() + 1, &m_data_type, sizeof(m_data_type));
         memcpy(message.get() + 5, m_data.get(), m_data_size);
         
         return std::tuple<data_buffer_type, int>{std::move(message), message_size};
      }

      std::shared_ptr<datagram> copy_to_shared() const
      {
         data_buffer_type data{new char[m_data_size]};
         memcpy(data.get(), m_data.get(), m_data_size);

         std::shared_ptr<datagram> pdg{new datagram(m_data_type, std::move(data), m_data_size)};
         
         return pdg;
      }
      
      friend std::ostream& operator<<(std::ostream& os, const datagram& dg);

      static std::string type(const datagram& dg)
      {
         switch(dg.m_data_type)
         {
            case datagram_type::nodata:
               return {"nodata"};
            case datagram_type::message:
               return {"message"};
            default:
               return {"UNKNOWN"};
         }
      }

      //static datagram try_to_read(SOCKFD_TYPE sockfd);
};

std::ostream& operator<<(std::ostream& os, const datagram& dg)
{
   os << " datagram: type = "  << datagram::type(dg) << std::endl;
   os << " datagram: size = "  << dg.m_data_size     << std::endl;
   os << " datagram: msg  = '";
   for(int i = 0; i < dg.m_data_size; ++i)
   {
      os << dg.m_data[i];
   }
   os << "'";
   return os;
}

class datagram_reader
   : public blocking_queue<datagram>
{
   private:
      using bound_type       = std::tuple<bool, int, int>;
      using data_buffer_type = typename datagram::data_buffer_type;
      

      //int m_readin_buffer_size = 2048;
      int m_readin_buffer_size = 10;
      int m_buffer_size        = 4 * m_readin_buffer_size;
      int m_buffer_head        = 0;
      
      data_buffer_type m_buffer {new char[m_buffer_size]};

      void increase_buffer_size()
      {
         int new_buffer_size = 2 * m_buffer_size;
         data_buffer_type new_buffer{new char[new_buffer_size]};

         memcpy(new_buffer.get(), m_buffer.get(), m_buffer_head);

         m_buffer = std::move(new_buffer);
         m_buffer_size = new_buffer_size;
      }
      
      /**
       *
       **/
      bound_type search_for_message()
      {
         int start = -1, end = -1;
         int i;
         for(i = 0; i < m_buffer_head; ++i)
         {
            if(m_buffer[i] == datagram::soh)
            {
               start = i;
               break;
            }
         }
         if(start != -1)
         {
            for(; i < m_buffer_head; ++i)
            {
               if(m_buffer[i] == datagram::eot)
               {
                  end = i;
                  break;
               }
            }
         }
         return {(start != -1) && (end != -1), start, end};
      }

      /**
       *
       **/
      datagram extract_datagram(const bound_type& t)
      {
         auto& [b, start, end] = t;
         int size_data_type = sizeof(datagram::datagram_type);
         
         // calculate size of data (+1 is for soh character)
         int header_start = start + 1;
         int header_size  = size_data_type;
         int data_start   = start + size_data_type + 1;
         int data_size    = end - data_start;
         
         // create datagram
         datagram::datagram_type data_type;
         data_buffer_type data{new char[data_size]};

         memcpy(&data_type, m_buffer.get() + header_start, header_size);
         memcpy(data.get(), m_buffer.get() + data_start  , data_size);
         

         //int one = 1;
         //char* data_type_lol = (char*)&data;

         //   
         //std::cout << " DATA TYPE LOL " << std::endl;
         //for(int i = 0; i < 4; ++i)
         //{
         //   std::cout << data_type_lol[i] << std::endl;
         //}
         
         std::cout << " DATA RECIEVED" << std::endl;
         std::cout << header_start << std::endl;
         std::cout << header_size << std::endl;
         std::cout << data_start << std::endl;
         std::cout << data_size << std::endl;
         std::cout << data_type << std::endl;
         for(int i = 0; i < data_size; ++i)
         {
            std::cout << data[i] << std::endl;
         }
         std::cout << " DATA RECIEVED END " << std::endl;
         // clean buffer
         //std::cout << " START: " << start << std::endl;
         int head = end + 1;
         for(int i = start; i < m_buffer_head; ++i)
         {
            if(head < m_buffer_head)
            {
               m_buffer[i] = m_buffer[head];
            }
            else
            {
               m_buffer[i] = '\0';
            }
         
            ++head;
         }
         m_buffer_head = start + (m_buffer_head - end) - 1;

         return datagram{data_type, std::move(data), data_size};
      }

   public:
      datagram_reader()
      {
      }

      void try_read_fd(SOCKFD_TYPE sockfd)
      {
         while(true)
         {
            // try to read
            int buffer_capacity = std::min(m_readin_buffer_size, m_buffer_size - m_buffer_head);
            std::cout << " BUFFER HEAD " << m_buffer_head << std::endl;
            int read_bytes = 0;
            if( (read_bytes = read(sockfd, m_buffer.get() + m_buffer_head, buffer_capacity)) == -1)
            {
               perror("PERROR after read");
               std::cout << "READ BYTES " << read_bytes << std::endl;
               if (errno == EWOULDBLOCK || errno == EAGAIN) {
                  return;
               }
            }
            else if(read_bytes == 0)
            {
               blocking_queue<datagram>::push(datagram(datagram::close));
               return;
            }
            std::cout << "READ BYTES " << read_bytes << std::endl;
            
            // update buffer
            m_buffer_head += read_bytes;
            if(read_bytes == buffer_capacity)
            {
               if(m_buffer_head == m_buffer_size)
               {
                  increase_buffer_size();
               }
            }
            else
            {
               break;
            }
         }
         
         //int lol = 0;
         while(true)
         {
            for(int i = 0; i < m_buffer_head; ++i)
            {
               std::cout << m_buffer[i];
            }
            std::cout << std::endl;
            auto t = search_for_message();
            std::cout << std::get<0>(t) << "   " << std::get<1>(t) << "    " << std::get<2>(t) << std::endl;
            if(!std::get<0>(t))
            {
               break;
            }
            blocking_queue<datagram>::push(extract_datagram(t));
            //if(lol)
            //{
            //   std::exit(0);
            //}
            //else
            //{
            //   std::cout << " LOL " << lol << std::endl;
            //}
            //++lol;
         }
      }
};

class datagram_writer
{
   public:
      bool try_write_fd(SOCKFD_TYPE sockfd, const datagram& dg)
      {
         auto [message, size] = dg.convert_to_message();
         
         int write_size = 0;
         if(write_size = write(sockfd, message.get(), size) != size)
         {
            std::cout << " HERE ?? " << write_size << std::endl;
            if (errno == EWOULDBLOCK || errno == EAGAIN) {
               return false;
            }
         }
         std::cout << write_size << std::endl;
         return true;
      }
};
