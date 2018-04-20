#ifndef BLOCKING_QUEUE_H_INCLUDED
#define BLOCKING_QUEUE_H_INCLUDED

#include <mutex>
#include <condition_variable>
#include <deque>

template<class T>
class blocking_queue
{
   private:
      mutable std::mutex m_mutex;
      std::condition_variable m_condition;
      std::deque<T> m_queue;
   
   public:
      /*! @function void push(const T& t) 
       *  @brief Push a value onto the queue.
       *  @param t The value to be pushed onto queue
       */
      void push(const T& t)
      {
         {
            std::lock_guard<std::mutex> queue_lock(m_mutex);
            m_queue.emplace_back(t);
         }
         m_condition.notify_one();
      }
      
      /*!
       *
       */
      bool pop_try_wait(T& t)
      {
         // get the event
         std::unique_lock<std::mutex> lock(m_mutex);
         auto wait_success = m_condition.wait_for
            (  lock
            ,  std::chrono::milliseconds(100)
            ,  [this](){ return !this->m_queue.empty(); }
            );

         if(wait_success)
         { 
            // we successfully popped an event
            t = std::move(m_queue.front());
            m_queue.pop_front();
         }

         return wait_success;
      }
      
      /*!
       *
       */
      bool pop_force_wait(T& t)
      {
         // get the event
         std::unique_lock<std::mutex> lock(m_mutex);
         m_condition.wait
            (  lock
            ,  [this](){ return !this->m_queue.empty(); }
            );

         t = std::move(m_queue.front());
         m_queue.pop_front();
         return true;
      }

      /*!
       *
       */
      bool empty() const
      {
         std::lock_guard<std::mutex> lock(m_mutex);
         return m_queue.empty();
      }
};

#endif /* BLOCKING_QUEUE_H_INCLUDED */
