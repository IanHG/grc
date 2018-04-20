#include "gui.hpp"

#include <ncurses.h>

namespace gui
{

/**
 * Initialize the gui.
 **/
gui::gui()
{
   ::initscr();
   ::keypad(stdscr, TRUE);
   ::cbreak();
   ::noecho();
   ::curs_set(0);
   ::refresh();
}

/**
 * Destroy the gui.
 **/
gui::~gui()
{
   ::endwin();
}

/**
 * Draw the gui.
 **/
void gui::draw()
{
   for(const auto& elem : m_elements)
   {
      elem->draw();
   }

   ::refresh();  
}

} /* namespace gui */
