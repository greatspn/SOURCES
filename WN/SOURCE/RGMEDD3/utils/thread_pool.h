#ifndef CONCURRENT_THREADPOOL_H
#define CONCURRENT_THREADPOOL_H

#include <atomic>
#include <thread>
#include <mutex>
#include <array>
#include <list>
#include <functional>
#include <condition_variable>

namespace nbsdx {
namespace concurrent {

/**
 *  Simple thread_pool that creates `thread_count` threads upon its creation,
 *  and pulls from a queue to get new jobs.
 *
 *  This class requires a number of c++11 features be present in your compiler.
 */
class thread_pool {
    
    const size_t thread_count;
    const size_t max_queue;
    std::vector<std::thread> threads;
    std::list<std::function<void(void)>> queue;

    std::atomic_int         jobs_left;
    std::atomic_bool        bailout;
    std::atomic_bool        finished;
    std::condition_variable job_available_var;
    std::condition_variable wait_var;
    std::condition_variable queue_full_var;
    std::mutex              wait_mutex;
    std::mutex              queue_mutex;

    /**
     *  Take the next job in the queue and run it.
     *  Notify the main thread that a job has completed.
     */
    void thread_loop() {
        while( !bailout ) {
            next_job()();
            --jobs_left;
            wait_var.notify_one();
        }
    }

    /**
     *  Get the next job; pop the first item in the queue, 
     *  otherwise wait for a signal from the main thread.
     */
    std::function<void(void)> next_job() {
        std::function<void(void)> res;
        std::unique_lock<std::mutex> job_lock( queue_mutex );

        // Wait for a job if we don't have any.
        job_available_var.wait( job_lock, [this]() ->bool { return queue.size() || bailout; } );
        
        // Get job from the queue
        bool qfull = false;
        if( !bailout ) {
            // std::cout << "  q="<< queue.size() << std::endl;
            qfull = (queue.size() == max_queue);
            res = queue.front();
            queue.pop_front();
        }
        else { // If we're bailing out, 'inject' a job into the queue to keep jobs_left accurate.
            res = []{};
            ++jobs_left;
        }

        if (qfull) // Wake up one producer
            queue_full_var.notify_one();
        return res;
    }

public:
    thread_pool(size_t _count, size_t _max_queue)
        : thread_count(_count), max_queue(_max_queue), jobs_left( 0 )
        , bailout( false )
        , finished( false ) 
    {
        threads.resize(thread_count);
        for( unsigned i = 0; i < thread_count; ++i )
            threads[ i ] = std::thread( [this]{ this->thread_loop(); } );
    }

    /**
     *  join_all on deconstruction
     */
    ~thread_pool() {
        join_all();
    }

    /**
     *  Get the number of threads in this pool
     */
    inline unsigned size() const {
        return thread_count;
    }

    /**
     *  Get the number of jobs left in the queue.
     */
    inline unsigned remaining_jobs() {
        std::lock_guard<std::mutex> guard( queue_mutex );
        return queue.size();
    }

    /**
     *  Add a new job to the pool. If there are no jobs in the queue,
     *  a thread is woken up to take the job. If all threads are busy,
     *  the job is added to the end of the queue.
     */
    void add_job( std::function<void(void)> job ) {
        std::unique_lock<std::mutex> guard( queue_mutex );

        // Wait if the queue is full
        if (queue.size() >= max_queue)
            queue_full_var.wait(guard);

        queue.emplace_back( job );
        ++jobs_left;
        job_available_var.notify_one();
    }

    /**
     *  Join with all threads. Block until all threads have completed.
     *  Params: WaitForAll: If true, will wait for the queue to empty 
     *          before joining with threads. If false, will complete
     *          current jobs, then inform the threads to exit.
     *  The queue will be empty after this call, and the threads will
     *  be done. After invoking `thread_pool::join_all`, the pool can no
     *  longer be used. If you need the pool to exist past completion
     *  of jobs, look to use `thread_pool::wait_all`.
     */
    void join_all( bool WaitForAll = true ) {
        if( !finished ) {
            if( WaitForAll ) {
                wait_all();
            }

            // note that we're done, and wake up any thread that's
            // waiting for a new job
            bailout = true;
            job_available_var.notify_all();

            for( auto &x : threads )
                if( x.joinable() )
                    x.join();
            finished = true;
        }
    }

    /**
     *  Wait for the pool to empty before continuing. 
     *  This does not call `std::thread::join`, it only waits until
     *  all jobs have finshed executing.
     */
    void wait_all() {
        if( jobs_left > 0 ) {
            std::unique_lock<std::mutex> lk( wait_mutex );
            wait_var.wait( lk, [this]{ return this->jobs_left == 0; } );
            lk.unlock();
        }
    }
};

} // namespace concurrent
} // namespace nbsdx

#endif //CONCURRENT_THREADPOOL_H
