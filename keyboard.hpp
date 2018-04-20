#ifndef KEYBOARD_H_INCLUDED
#define KEYBOARD_H_INCLUDED

#include <ncurses.h>
#include <thread>
#include <future>
#include <mutex>
#include <iostream>
#include <deque>

#include "util/singleton.hpp"
#include "util/event_handler.hpp"
#include "util/blocking_queue.hpp"

using ev_type = int;

/*
 * struct keyboard_queue - represents keyboard event queue
 */
class keyboard_queue
   : public event_handler<ev_type, void()>
   , private blocking_queue<ev_type>
{
   using event_handler_t = event_handler<ev_type, void()>;
   using queue_t = blocking_queue<ev_type>;

   protected:
      void update(const event_t& keyboard_event)
      {
         queue_t::push(keyboard_event);
      }

   public:

      void handle_events()
      {
         if(queue_t::empty())
         {
            event_handler_t::handle_noevent();
         }
         else
         {
            while(!queue_t::empty())
            {
               // get an event
               event_t event = 0;
               queue_t::pop_try_wait(event); 

               // then handle event
               event_handler_t::handle_event(event);
            }
         }
      }
};

/*
 *
 */
class keyboard
   : public keyboard_queue
{
   private:
      bool m_active = false;
      keyboard_queue keyboard_queue;
      std::future<void> m_return_from_thread;
      
      bool is_active() const
      {
         return m_active;
      }
      
      void read_event()
      {
         int ch = getch();
         if(ch != ERR)
         {
            keyboard_queue::update(ch);
         }
      }
      
      void loop()
      {
         while(is_active())
         {
            read_event();
         }
      }

   public:
      keyboard()
      {
         m_active = true;
         m_return_from_thread = std::async(std::launch::async, std::bind(&keyboard::loop,this));
      }

      keyboard(const keyboard&) = delete;
      keyboard& operator=(const keyboard&) = delete;
      
      ~keyboard()
      {
         m_active = false;
         m_return_from_thread.wait(); // wait for thread to finish
      }
};

#endif /* KEYBOARD_H_INCLUDED */
