#include "gui/interface.hpp"

int main(int argc, char* argv[])
{
   gui::initialize();
   gui::refresh();
      
   {
      int height = 80;
      int width  = 100;
	   int starty = (LINES - height) / 2;	/* Calculating for a center placement */
	   int startx = (COLS - width) / 2;	/* of the window		*/
      auto win = gui::window(height, width, starty, startx);
      //scrollok(win.get(), TRUE);
      idlok(win.get(), TRUE);
      win.draw();
      
      char buff[1024];
      mvwgetstr(win.get(), 79, 60, buff);
      
      gui::refresh();
   
      getch();
   }

   gui::finalize();

   return 0;
}
