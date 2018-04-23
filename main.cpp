#include "gui/interface.hpp"
#include <signal.h>
#include <typeinfo>
#include <chrono>
#include <thread>
#include <string>
#include <list>
#include <tuple>

#include <unistd.h> // for getlogic_r
#include <sys/types.h>
#include <pwd.h>

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
   //char *braille = 
   // " ⠁⠂⠃⠄⠅⠆⠇⠈⠉⠊⠋⠌⠍⠎⠏\n"
   // "⠐⠑⠒⠓⠔⠕⠖⠗⠘⠙⠚⠛⠜⠝⠞⠟\n"
   // "⠠⠡⠢⠣⠤⠥⠦⠧⠨⠩⠪⠫⠬⠭⠮⠯\n"
   // "⠰⠱⠲⠳⠴⠵⠶⠷⠸⠹⠺⠻⠼⠽⠾⠿\n";

   //setlocale(LC_ALL, "");
   
   // GET USER INFO
   passwd* pass = getpwuid(getuid());
   char* username = pass->pw_name;

   auto& gui = gui::gui::instance();
   debug::initialize();
   debug::message("FIRST MESSAGE");
   //debug::message(std::string(braille));
   
   // CREATE GUI
   double height_frac = 0.80;
   double width_frac  = 0.70;

   int x, y;
   getmaxyx(stdscr, y, x);

   int height_display_window = y * height_frac;
   int width_display_window  = x * width_frac;
   
   int height_writing_window = y * (1.0 - height_frac);
   int width_writing_window  = x * width_frac;

   debug::message("x : " + std::to_string(x));
   debug::message("y : " + std::to_string(y));
   debug::message("display height: " + std::to_string(height_display_window));
   debug::message("display width : " + std::to_string(width_display_window));
   debug::message("writing height: " + std::to_string(height_writing_window));
   debug::message("writing width : " + std::to_string(width_writing_window));
   
   auto&& display_window = gui.create_window(height_display_window, width_display_window, 0, 0);
   auto&& writing_window = gui.create_window(height_writing_window, width_writing_window, 0, height_display_window);
   
   // CRETAE KEYBOARD
   keyboard kb;
   editing_string str;
   str.register_events(kb);
   
   kb.register_function(10, [&str, &display_window, &username](){ 
      auto [s, a] = str.get();
      
      int max_x, max_y;
      getmaxyx(display_window.get(), max_y, max_x);
      
      int x, y;
      getyx(display_window.get(), y, x);
      
      if( y == max_y - 2)
      {
         wclear(display_window.get());
         display_window.rebox();
         wmove(display_window.get(), 1, 1);
      }
      else
      {
         wmove(display_window.get(), y + 1, 1);
      }
      
      wprintw(display_window.get(), username);
      wprintw(display_window.get(), " : ");
      wprintw(display_window.get(), s.c_str());
      
      str.clear(); 
   } );
   
   try
   {
      debug::message("STARTING LOOP");
      while(true)
      {
         // Handle keyboard events
         kb.handle_events();
            
         // 
         wclear(writing_window.get());
         writing_window.rebox();
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
