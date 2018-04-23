#ifndef JOIN_THREADS_HPP_INCLUDED
#define JOIN_THREADS_HPP_INCLUDED

#include <thread>

/**
 * Class for ensuring threads get joined on destrcution.
 * For Scope-bound-resource-management (SBRM) of threads.  
 **/
class join_threads
{
   private:
      //! Reference to vector of threads to join on destruction.
      std::vector<std::thread>& threads;

   public:
      //! Constructor from reference
      explicit join_threads(std::vector<std::thread>& threads_)
         : threads(threads_)
      {
      }
      
      //! Destructor, will join all joinable threads in reference
      ~join_threads()
      {
         for(auto& t : threads)
         {
            if(t.joinable())
            {
               t.join();
            }
         }
      }
};

#endif /* JOIN_THREADS_HPP_INCLUDED */
