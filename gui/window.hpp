#ifndef GUI_WINDOW_H_INCLUDED
#define GUI_WINDOW_H_INCLUDED

#include <ncurses.h>

namespace gui
{
namespace detail
{

// Wrap ncurses
WINDOW* newwin(int height, int width, int starty, int startx)
{
   return ::newwin(height, width, starty, startx);
}

void box(WINDOW* win, int x, int y)
{
   ::box(win, x, y);
}

void wrefresh(WINDOW* win)
{
   ::wrefresh(win);
}

void delwin(WINDOW* win)
{
   ::delwin(win);
}

}

class window
{
   private:
      int m_height = 0;
      int m_width = 0;
      int m_pos_x = 0;
      int m_pos_y = 0;
      WINDOW* m_window;

   public:
      window(int h, int w, int x, int y)
         :  m_height(h)
         ,  m_width(w)
         ,  m_pos_x(x)
         ,  m_pos_y(y)
      {
         m_window = detail::newwin(m_height, m_width, m_pos_x, m_pos_y);
         
         detail::box(m_window, 0,0);
         
         //detail::wrefresh(m_window);
      }

      ~window()
      {
         wborder(m_window, ' ', ' ', ' ',' ',' ',' ',' ',' ');
         
         this->draw();
         
         detail::delwin(m_window);
      }

      void draw()
      {
         detail::wrefresh(m_window);
      }
      
      WINDOW* get()
      {
         return m_window;
      }

};

} /* namespace gui */

#endif /* GUI_WINDOW_H_INCLUDED */
