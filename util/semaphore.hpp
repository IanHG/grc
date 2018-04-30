#ifndef SEMAPHORE_H_INCLUDED
#define SEMAPHORE_H_INCLUDED

/**
 * Disclaimer: Based on Rui Figueira's code posted on
 * http://www.crazygaze.com/blog/2016/03/24/portable-c-timer-queue/
 **/

#include <mutex>
#include <condition_variable>

class semaphore {
public:
    semaphore(unsigned int count = 0) : m_count(count) {}
 
    void notify() {
        std::unique_lock<std::mutex> lock(m_mtx);
        m_count++;
        m_cv.notify_one();
    }
 
    void wait() {
        std::unique_lock<std::mutex> lock(m_mtx);
        m_cv.wait(lock, [this]() { return m_count > 0; });
        m_count--;
    }
 
    template <class Clock, class Duration>
    bool waitUntil(const std::chrono::time_point<Clock, Duration>& point) {
        std::unique_lock<std::mutex> lock(m_mtx);
        if (!m_cv.wait_until(lock, point, [this]() { return m_count > 0; }))
            return false;
        m_count--;
        return true;
    }


    //bool waitFor(duration) : To wait for specified duration before timeout
    //bool tryWait() : To check and decrement the semaphore without waiting.
 
private:
    std::mutex m_mtx;
    std::condition_variable m_cv;
    unsigned int m_count;
};

#endif /* SEMAPHORE_H_INCLUDED */
