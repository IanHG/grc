#include "debug.hpp"

#include <mutex>

#include "window.hpp"

#define DEBUG

namespace debug
{

using mutex_t = std::mutex;

gui::window* debug_window = nullptr;
mutex_t debug_mutex;

#define DEBUG_WIDTH 40

/**
 * Initialize debug message module.
 **/
void initialize()
{
#ifdef DEBUG
   auto& gui = gui::gui::instance();
   
   int x, y;
   getmaxyx(stdscr, y, x);

   auto&& dw = gui.create_window(y, DEBUG_WIDTH, x - DEBUG_WIDTH, 0);
   debug_window = &dw;
   
   //wmove(debug_window->get(), 1, 1);

   debug_window->draw();
#endif /* DEBUG */
}

/**
 * Print message in debug window.
 **/
void message(const std::string& msg)
{
#ifdef DEBUG
   std::lock_guard<mutex_t> lock(debug_mutex);
   
   int max_x, max_y;
   getmaxyx(stdscr, max_y, max_x);
   
   int x, y;
   getyx(debug_window->get(), y, x);
   
   if( y == max_y - 2)
   {
      wclear(debug_window->get());
      debug_window->rebox();
      wmove(debug_window->get(), 1, 1);
   }
   else
   {
      wmove(debug_window->get(), y + 1, 1);
   }

   wprintw(debug_window->get(), msg.c_str());

   debug_window->draw();
#endif /* DEBUG */
}

#undef DEBUG_WIDTH

} /* namespace debug */
