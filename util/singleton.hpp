#ifndef SINGLETON_H_INCLUDED
#define SINGLETON_H_INCLUDED

/**
 *
 **/
template<class A>
class singleton
{
   private:
   
   protected:
      //! Protected default constructor.
      singleton() = default;

   public:
      //!@{
      //! Deleted copy/move ctor and assignment.
      singleton(const singleton&) = delete;
      singleton(singleton&&) = delete;
      singleton& operator=(const singleton&) = delete;
      singleton& operator=(singleton&&) = delete;
      //!@}
      
      //! Create instance
      static A& instance()
      {
         static A a; 
         return a;
      }
};

#endif /* SINGLETON_H_INCLUDED */
