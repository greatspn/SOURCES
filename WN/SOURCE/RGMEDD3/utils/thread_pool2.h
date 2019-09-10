#ifndef CONCURRENT_THREADPOOL_H
#define CONCURRENT_THREADPOOL_H

#include <cassert>
#include <thread>
#include <mutex>
#include <list>
#include <functional>
#include <condition_variable>

// A thread pool with a fixed number of threads that may consume
// job activities in parallel. The class stores a queue of the 
// submitted activities. The queue has a maximum number of activities
// pending, before putting the sender to sleep.
class thread_pool {
    
    // number of parallel threads performing the requested jobs
    const size_t thread_count;
    // maximum number of pending jobs in the queue
    const size_t max_queue;
    // The parallel threads
    std::vector<std::thread> threads;
    // The queue of the pending jobs
    std::list<std::function<void(void)>> queue;

    // Condition signaled when the queue becomes empty
    std::condition_variable queue_empty_var;
    // Condition signaled when the queue is full
    std::condition_variable queue_full_var;
    // Condition signaled when the queue is empty and the
    // last job has been completed
    std::condition_variable last_job_completed;
    // Single thread pool mutes
    std::mutex              queue_mutex;
    // Termination state
    bool                    finishing = false;
    bool                    finished = false;
    // Count of the threads that are actualy processing a job
    size_t                  jobs_running = 0;

    // Main loop of the thread pool
    void thread_loop() {
    	std::function<void(void)> fn;
        while (next_job(fn)) {
            fn();
            job_end();
        }
    }

public:
    thread_pool(size_t _count, size_t _max_queue)
        : thread_count(_count), max_queue(_max_queue)
    {
        threads.resize(thread_count);
        for (size_t i = 0; i < thread_count; ++i)
            threads[ i ] = std::thread( [this]{ this->thread_loop(); } );
    }

    thread_pool()
    	: thread_pool(size_t(std::max(1u, std::thread::hardware_concurrency())),
    		          size_t(4 * std::max(1u, std::thread::hardware_concurrency())))
    {}

    ~thread_pool() {
        join_all();
    }

    // Add a new job to the pool. The caller could wait if the queue is full
    void add_job( std::function<void(void)> job ) {
        std::unique_lock<std::mutex> guard(queue_mutex);

        // Wait if the queue is full
        if (queue.size() >= max_queue)
            queue_full_var.wait(guard);

        queue.emplace_back( std::move(job) );
        // Wake up some thread for processing
        if (queue.size() == 1)
        	queue_empty_var.notify_one();
    }

private:
	// Get the next job from the queue. 
	// Return false if the queue is empty and the thread pool
	// is shutting down with join_all().
    bool next_job(std::function<void(void)>& fn) {
        std::unique_lock<std::mutex> guard(queue_mutex);

        if (queue.empty() && !finishing)
        	queue_empty_var.wait(guard);

        if (finishing && queue.empty()) {
        	return false;
        }
        assert(queue.size() > 0);
        bool qfull = (queue.size() == max_queue);
        fn = std::move(queue.front());
        queue.pop_front();

        if (qfull) // Wake up one producer
            queue_full_var.notify_one();

        ++jobs_running;
        return true;
    }

    // A worker thread has completed a job.
    void job_end() {
    	std::lock_guard<std::mutex> guard(queue_mutex);
    	--jobs_running;
    	// Is this the last job before the queue is left empty?
    	if (queue.empty() && jobs_running==0)
    		last_job_completed.notify_all(); 
    }

public:
	// Join with all the threads. Wait before all threads have completed
	// all the sent jobs. After calling join_all(), the pool cannot be used
	// any longer.
    void join_all() {
    	if (finished)
    		return;
    	do {
	    	std::unique_lock<std::mutex> guard(queue_mutex);
	    	finishing = true;

	    	// // Wait for the last job to end
	    	// if (!queue.empty() || jobs_running > 0) {
	    	// 	last_job_completed.wait(guard);
	    	// }

	    	// Wake up all threads and let them quit from the main loop
	    	queue_empty_var.notify_all();
	    	// assert(queue.size() == 0);
	    } while (0);

    	for (auto &x : threads) {
            assert (x.joinable());
            x.join();
    	}
        finished = true;
    }

    // Wait for all sent jobs to be completed. It does not terminate
    // the worker threads.
    void wait_all() {
    	std::unique_lock<std::mutex> guard(queue_mutex);
    	if (!queue.empty() || jobs_running > 0) {
    		last_job_completed.wait(guard);
    	}
    }
};

#endif // CONCURRENT_THREADPOOL_H
