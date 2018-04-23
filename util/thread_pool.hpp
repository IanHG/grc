#ifndef THREAD_POOL_H_INCLUDED
#define THREAD_POOL_H_INCLUDED

#include <thread>
#include <atomic>
#include <vector>
#include <future>

#include "function_wrapper.hpp"
#include "work_stealing_queue.hpp"
#include "join_threads.hpp"

/**
 * Thread pool with thread specific queues, 
 * but enables work stealing between threads.
 **/
class thread_pool
{
   private:
      using task_type  = function_wrapper;
      using queue_type = work_stealing_queue<task_type>;
      
      //! Should threads terminate
      std::atomic_bool done;
      //! Index taht shuffles through threads for assignment of work
      std::atomic_uint queue_index;
      //! Vector of all work queues
      std::vector<std::unique_ptr<queue_type> > queues;
      //! Vector of threads
      std::vector<std::thread> threads;
      //! Ensures that all threads are joined (and thus correctly terminated) on pool destruction
      join_threads joiner;
      //! Pointer to thread local queue
      static thread_local queue_type* local_work_queue;
      //! Thread local thread-index
      static thread_local int my_index;
      
      //! Worker thread function
      void worker_thread(unsigned my_index_)
      {
         my_index = my_index_;
         local_work_queue = queues[my_index].get();
      
         while(!done)
         {
            run_pending_task();
         }
      }
      
      //! Get task from local queue, if available
      bool pop_task_from_local_queue(task_type& task)
      {
         return local_work_queue && local_work_queue->try_pop(task);
      }

      //! Steal task from other queue, if available
      bool pop_task_from_other_thread_queue(task_type& task)
      {
         for(unsigned i = 0; i < queues.size(); ++i)
         {
            unsigned const index = (my_index + i + 1) % queues.size();
            if(queues[index]->try_steal(task))
            {
               return true;
            }
         }
         return false;
      }

   public:
      //! Constructor. Will start all threads in pool or fail gracefully.
      thread_pool(unsigned num_threads = 0)
         :  done(false)
         ,  queue_index(0)
         ,  joiner(threads)
      {
         const unsigned thread_count = num_threads ? num_threads : std::thread::hardware_concurrency();
         queues.reserve(thread_count);

         my_index = 0;  // master thread also gets index 0 (may also run jobs, but not the standard way of doing things).

         try
         {
            for(unsigned i = 0; i < thread_count; ++i)
            {
               queues.emplace_back(new queue_type);
            }
            for(unsigned i = 0; i < thread_count; ++i)
            {
               threads.emplace_back(&thread_pool::worker_thread, this, i);
            }
         }
         catch(...)
         {
            done = true;
            throw;
         }
      }
      
      //! Destructor. Will terminate all threads when currently running tasks finish.
      ~thread_pool()
      {
         done = true;
      }
      
      //! Submit task to queue
      template<class F>
      std::future<typename std::result_of<F()>::type> submit(F&& f)
      {
         using result_type = typename std::result_of<F()>::type;

         std::packaged_task<result_type()> task(std::move(f));
         std::future<result_type> res(task.get_future());
         
         // move task to a queue
         unsigned index = (queue_index++)%queues.size();
         queues[index]->push(std::move(task));
         
         return res;
      }
      
      //! Run a pending task
      void run_pending_task()
      {
         task_type task;
         if (  pop_task_from_local_queue(task)
            || pop_task_from_other_thread_queue(task)
            )
         {
            task();
         }
         else
         {
            std::this_thread::yield();
         }
      }
};


#endif /* THREAD_POOL_H_INCLUDED */
