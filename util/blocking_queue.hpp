#ifndef BLOCKING_QUEUE_H_INCLUDED
#define BLOCKING_QUEUE_H_INCLUDED

#include <mutex>
#include <condition_variable>
#include <deque>

template<class T>
class blocking_queue
{
   public:
      using data_type  = T;

   private:
      using mutex_type = std::mutex;

      mutable mutex_type m_mutex;
      std::condition_variable m_condition;
      std::deque<data_type> m_queue;
   
   public:
      /*!
       *
       */
      blocking_queue() = default;

      /*!
       *
       */
      blocking_queue(const blocking_queue&) = delete;
      
      /*!
       *
       */
      blocking_queue& operator=(const blocking_queue&) = delete;

      /*! @function void push(const data_type& t) 
       *  @brief Push a value onto the queue.
       *  @param t The value to be pushed onto queue
       */
      void push(const data_type& t)
      {
         {
            std::lock_guard<mutex_type> queue_lock(m_mutex);
            m_queue.emplace_back(t);
         }
         m_condition.notify_one();
      }
      
      /*!
       *
       */
      bool pop_try_wait(data_type& t)
      {
         // get the event
         std::unique_lock<mutex_type> lock(m_mutex);
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
      bool pop_force_wait(data_type& t)
      {
         // get the event
         std::unique_lock<mutex_type> lock(m_mutex);
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
         std::lock_guard<mutex_type> lock(m_mutex);
         return m_queue.empty();
      }
};

#endif /* BLOCKING_QUEUE_H_INCLUDED */
