#ifndef GUI_GUI_H_INCLUDED
#define GUI_GUI_H_INCLUDED

#include <list>
#include <memory>

#include "widget.hpp"
#include "window.hpp"

#include "../util/singleton.hpp"

namespace gui
{

class gui
   :  public singleton<gui>
{
   private:
      std::list<std::unique_ptr<widget> > m_elements;
      
   public:
      gui();

      ~gui();

      void draw();

      window& create_window(int h, int w, int x, int y)
      {
         m_elements.emplace_back(new window(h, w, x, y));
         return static_cast<window&>(*m_elements.back().get());
      }
};

} /* namespace gui */

#endif /* GUI_GUI_H_INCLUDED */
