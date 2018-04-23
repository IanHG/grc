#ifndef WORK_STEALING_QUEUE_HPP_INCLUDED
#define WORK_STEALING_QUEUE_HPP_INCLUDED

#include <deque>
#include <mutex>

/**
 * Thread specific task queue that enables other threads to steal tasks
 * if they have nothing else to do.
 **/
template<class T>
class work_stealing_queue
{
   public:
      using data_type = T;
   
   private:
      //! The underlying double-ended queue
      std::deque<data_type> the_queue;
      //! Mutex for access to queue
      mutable std::mutex the_mutex;

   public:
      //! default constructor
      work_stealing_queue() = default;
      
      //! Deleted copy constructor
      work_stealing_queue(const work_stealing_queue&) = delete;
      
      //! Deleted copy assignement
      work_stealing_queue& operator=(const work_stealing_queue&) = delete;
      
      //! Push task to the queue
      void push(data_type data)
      {
         std::lock_guard<std::mutex> lock(the_mutex);
         the_queue.push_front(std::move(data));
      }
      
      //! Is the queue empty
      bool empty()
      {
         std::lock_guard<std::mutex> lock(the_mutex);
         return the_queue.empty();
      }
      
      //! Try to pop a task from the queue
      bool try_pop(data_type& res)
      {
         std::lock_guard<std::mutex> lock(the_mutex);
         if(the_queue.empty())
         {
            return false;
         }

         res = std::move(the_queue.front());
         the_queue.pop_front();
         return true;
      }

      //! Try to steal a task from the queue
      bool try_steal(data_type& res)
      {
         std::lock_guard<std::mutex> lock(the_mutex);
         if(the_queue.empty())
         {
            return false;
         }

         res = std::move(the_queue.back());
         the_queue.pop_back();
         return true;
      }
};

#endif /* WORK_STEALING_QUEUE_H_INCLUDED */
