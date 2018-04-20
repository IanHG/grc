#include "debug.hpp"

#include <mutex>

#include "window.hpp"

#define DEBUG

namespace debug
{

using mutex_t = std::mutex;

gui::window* debug_window = nullptr;
mutex_t debug_mutex;

/**
 * Initialize debug message module.
 **/
void initialize()
{
#ifdef DEBUG
   auto& gui = gui::gui::instance();
   
   int x, y;
   getmaxyx(stdscr, y, x);

   auto&& dw = gui.create_window(0, 0, y, 20);
   debug_window = &dw;
   
   wmove(debug_window->get(), 1, 1);

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

   wprintw(debug_window->get(), msg.c_str());
   wprintw(debug_window->get(), "   ");
   debug_window->draw();
#endif /* DEBUG */
}

} /* namespace debug */
