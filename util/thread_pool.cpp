#include "thread_pool.hpp"

thread_local thread_pool::queue_type* thread_pool::local_work_queue;
thread_local int thread_pool::my_index;
