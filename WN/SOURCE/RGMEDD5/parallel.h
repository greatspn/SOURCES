#ifndef PARALLEL_H_
#define PARALLEL_H_

//---------------------------------------------------------------------------------------

const int PROC_NEVER_RUN = -10000;

// execute all the given tasks in parallel.
// if a process sends a SIGUSR1 signal to the parent, it is considered 
// the winner and all other processes are killed.
// return -1 if it is the parent process, the task number for the child process
int parallel_exec(const size_t max_concurr, 
                  const size_t num_tasks,
                  int *p_signaling_proc_index, 
                  std::vector<int>& out_exitcodes,
                  std::function<void(int, int)> pfn_procEnd,
                  bool verbose);

// notify the parent process with a SIGUSR1 and wait for its response to continue
// this method must be called by the parallel signalling process.
void notify_parent_and_continue();

//---------------------------------------------------------------------------------------

struct proc_wback_t {
    ssize_t task_id;    // task identifier assigned to the subprocess
    int     write_fd;   // file descriptor where the subprocess writes the result
};

// execute @num_tasks in parallel in at most @max_concurr cuncurrent forked processes.
// return -1 if it is the parent process (after completion of all parallel tasks), 
// return the task number for the child process.
// NOTE: the child subprocess must close the write end
// NOTE: the server process must free() all allocated blocks in out_memblocks[]
proc_wback_t 
parallel_exec_wback(const size_t max_concurr, 
                    const ssize_t num_tasks,
                    std::vector<int>& out_exitcodes,
                    std::vector<size_t>& out_lengths,
                    std::vector<void*>& out_memblocks,
                    size_t buffer_size, bool verbose);

//---------------------------------------------------------------------------------------

// constraint the memory of the process to a specified number of megabytes. 
// When MB<=0, release the imposed constraint.
void constraint_address_space(int MB);


//---------------------------------------------------------------------------------------

// Simple inter-process semaphore interface for basic synchronization

int semaphore_create();
void semaphore_close(int sem_id);
void semaphore_acquire(int sem_id);
void semaphore_release(int sem_id);

struct semaphore_sentinel {
	int sem_id = -1;
	semaphore_sentinel(int _s) : sem_id(_s) 
	{ semaphore_acquire(sem_id); }
	~semaphore_sentinel() { semaphore_release(sem_id); }
};

//---------------------------------------------------------------------------------------
#endif // PARALLEL_H_
