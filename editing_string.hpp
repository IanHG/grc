#ifndef EDITTING_STRING_H_INCLUDED
#define EDITTING_STRING_H_INCLUDED

#include <mutex>
#include <tuple>

#include "util/event_handler.hpp"
#include "keyboard.hpp"

#include "gui/debug.hpp"

/**
 * String that can be edited on the fly
 **/
class editing_string
   :  public event_registerable<keyboard>
{
   using character_t = char;
   using list_t      = std::list<character_t>;
   using iterator_t  = list_t::iterator;
   using mutex_t     = std::mutex;

   private:
      mutex_t m_mutex;

      list_t m_characters = {};

      iterator_t m_active = m_characters.end();

   public:
      editing_string()
      {
      }

      void insert(char c)
      {
         std::lock_guard<mutex_t> lock(m_mutex);

         if(m_active != m_characters.end())
         {
            m_active++;
         }
         m_active = m_characters.insert(m_active, c);
      }

      void del()
      {
         std::lock_guard<mutex_t> lock(m_mutex);

         if(m_active != m_characters.end())
         {
            m_active = m_characters.erase(m_active);
            m_active--;
         }
      }

      void incr_active()
      {
         std::lock_guard<mutex_t> lock(m_mutex);
         
         if(m_active != m_characters.end())
         {
            ++m_active;
         }
      }

      void decr_active()
      {
         std::lock_guard<mutex_t> lock(m_mutex);

         if(m_active != m_characters.begin())
         {
            --m_active;
         }
      }

      std::tuple<std::string, int> get()
      {
         std::lock_guard<mutex_t> lock(m_mutex);

         std::string str;
         int active = -1;
         int iactive = 0;
         for(auto iter = m_characters.begin(); iter != m_characters.end(); ++iter)
         {
            str += *iter;

            if (iter == m_active)
            {
               active = iactive;
            }
            
            ++iactive;
         }

         return std::tuple<std::string, int>{str, active};
      }

      void clear()
      {
         std::lock_guard<mutex_t> lock(m_mutex);

         m_characters.clear();
         m_active = m_characters.end();
      }
      
      //! Register events in keyboard event handler
      void register_events(keyboard& kb)
      {
         this->deregister_events(kb);
         
         // Alphanumeric keys
         #define generate_alpha_event(C) \
            event_registerable<keyboard>::register_function(kb, C     , [this](){ this->insert(C); } ); \
            event_registerable<keyboard>::register_function(kb, C + 32, [this](){ this->insert(C + 32); } );

         generate_alpha_event('A');
         generate_alpha_event('B');
         generate_alpha_event('C');
         generate_alpha_event('D');
         generate_alpha_event('E');
         generate_alpha_event('F');
         generate_alpha_event('G');
         generate_alpha_event('H');
         generate_alpha_event('I');
         generate_alpha_event('J');
         generate_alpha_event('K');
         generate_alpha_event('L');
         generate_alpha_event('M');
         generate_alpha_event('N');
         generate_alpha_event('O');
         generate_alpha_event('P');
         generate_alpha_event('Q');
         generate_alpha_event('R');
         generate_alpha_event('S');
         generate_alpha_event('T');
         generate_alpha_event('U');
         generate_alpha_event('V');
         generate_alpha_event('W');
         generate_alpha_event('Y');
         generate_alpha_event('Z');
         
         generate_alpha_event('.');
         generate_alpha_event(':');
         generate_alpha_event(',');
         generate_alpha_event(';');
         generate_alpha_event('?');
         generate_alpha_event('!');

         #undef generate_alpha_event
         
         #define generate_numeric_event(N) \
            event_registerable<keyboard>::register_function(kb, N, [this](){ this->insert(N); } );

         generate_numeric_event('1');
         generate_numeric_event('2');
         generate_numeric_event('3');
         generate_numeric_event('4');
         generate_numeric_event('5');
         generate_numeric_event('6');
         generate_numeric_event('7');
         generate_numeric_event('8');
         generate_numeric_event('9');
         generate_numeric_event('0');

         #undef generate_numeric_event
         
         event_registerable<keyboard>::register_function(kb, ' ', [this](){ this->insert(' '); } );

         // Arrow keys
         event_registerable<keyboard>::register_function(kb, KEY_LEFT  , [this](){ this->decr_active(); } );
         event_registerable<keyboard>::register_function(kb, KEY_RIGHT , [this](){ this->incr_active(); } );


         // Special keys
         //event_registerable<keyboard>::register_function(kb, KEY_ENTER    , [this](){ this->clear(); } );
         //event_registerable<keyboard>::register_function(kb, 10           , [this](){ this->clear(); } );
         event_registerable<keyboard>::register_function(kb, KEY_DL       , [this](){ this->del(); } );
         event_registerable<keyboard>::register_function(kb, KEY_BACKSPACE, [this](){ this->del(); } );

         #define KEY_ESC 27

         event_registerable<keyboard>::register_function(kb, KEY_ESC , [this, &kb](){ 
            this->deregister_events(kb);
            event_registerable<keyboard>::register_function(kb, KEY_ESC, [this, &kb](){ this->register_events(kb); } );
         } );

         #undef KEY_ESC
      }
      
      //! 
      void deregister_events(keyboard& kb)
      {
         event_registerable<keyboard>::deregister_functions(kb);
      }
};

#endif /* EDITTING_STRING_H_INCLUDED */
