#include "gui.hpp"

#include <ncurses.h>

namespace gui
{

void initialize()
{
   // Initialize ncurses
   ::initscr();
}

void finalize()
{
   ::endwin();
}

void refresh()
{
   ::refresh();  
}

} /* namespace gui */
