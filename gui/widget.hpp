#ifndef GUI_WIDGET_H_INCLUDED
#define GUI_WIDGET_H_INCLUDED

class widget
{
   private:

   public:
      widget() = default;

      virtual ~widget() = 0;

      virtual void draw() const = 0;
};

#endif /* GUI_WIDGET_H_INCLUDED */
