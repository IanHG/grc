#include "gui/interface.hpp"
#include <signal.h>
#include <typeinfo>
#include <chrono>
#include <thread>
#include <string>
#include <list>
#include <tuple>

#include "keyboard.hpp"
#include "editing_string.hpp"

#include "gui/debug.hpp"

WINDOW* chat_win_ptr = nullptr;

void resizeHandler(int sig)
{
   int nh, nw;
   
   getmaxyx(stdscr, nh, nw);  /* get the new screen size */
   
   wresize(chat_win_ptr, nh, nw);

   wrefresh(chat_win_ptr);
   refresh();
}



int main(int argc, char* argv[])
{
   using namespace std::chrono_literals;
   
   auto& gui = gui::gui::instance();
   debug::initialize();
   debug::message("FIRST MESSAGE");
   
   auto&& writing_window = gui.create_window(10, 10, 1, 1);

   keyboard kb;
   editing_string str;
   str.register_events(kb);
   
   try
   {
      debug::message("STARTING LOOP");
      while(true)
      {
         // Handle keyboard events
         kb.handle_events();
            
         // 
         wclear(writing_window.get());
         auto [s, a] = str.get();
         mvwprintw(writing_window.get(), 1, 1, s.c_str());
         
         // Draw gui
         gui.draw();

         // Then sleep a little
         std::this_thread::sleep_for(10ms);
      }
   }
   catch(std::exception& e)
   {
      printw("CAUGHT EXCEPTION : ");
      printw(e.what());
   }
   catch(...)
   {
      printw("CAUGHT SOMETHING");
   }
  
   gui.draw();			/* Print it on to the real screen */

   return 0;
}
