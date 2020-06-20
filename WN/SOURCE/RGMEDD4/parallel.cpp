#include <string.h>
// #include <stdio.h>
// #include <stdlib.h>
// #include <time.h>
#include <iostream>
#include <set>
#include <map>
#include <vector>
#include <cassert>

#include <signal.h>
#include <unistd.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/sem.h>

#include "parallel.h"

using namespace std;


//---------------------------------------------------------------------------------------

// Linux requires the program to define the semun data structure
#ifdef __linux__
union semun {
    int              val;    /* Value for SETVAL */
    struct semid_ds *buf;    /* Buffer for IPC_STAT, IPC_SET */
    unsigned short  *array;  /* Array for GETALL, SETALL */
    struct seminfo  *__buf;  /* Buffer for IPC_INFO */
};
#endif // __linux__


//---------------------------------------------------------------------------------------
// Parallel task execution
//---------------------------------------------------------------------------------------

static std::set<pid_t> g_signaling_pids;

static void handle_process_achievement(int signum, siginfo_t *info, void *context) {
    g_signaling_pids.insert(info->si_pid);
    // printf("pid=%d val=%d\n", info->si_pid, info->si_value.sival_int);
    // fflush(stdout);
}

//---------------------------------------------------------------------------------------

void perror_exit(const char* err) {
    perror(err);
    exit(EXIT_FAILURE);
}

//---------------------------------------------------------------------------------------

// execute @num_tasks in parallel in at most @max_concurr cuncurrent forked processes.
// Parallel processes may run in a race mode: the first who sends a SIGUSR1 
// to the parent is the winner, and all other processes are stopped.
// If this happens, @p_signaling_proc_index stores the index of that process task.
// return -1 if it is the parent process (after completion of all parallel tasks), 
// return the task number for the child process.
int parallel_exec(const size_t max_concurr, 
                  const size_t num_tasks,
                  int *p_signaling_proc_index, 
                  std::vector<int>& out_exitcodes,
                  bool verbose)
{
    out_exitcodes.resize(num_tasks);
    std::fill(out_exitcodes.begin(), out_exitcodes.end(), PROC_NEVER_RUN);
    bool have_signaling_proc = false;
    if (p_signaling_proc_index)
        *p_signaling_proc_index = -1;

    sigset_t mask, orig_mask;
    sigemptyset (&mask);
    sigaddset (&mask, SIGUSR1);

    struct sigaction sa;
    sa.sa_sigaction = handle_process_achievement;
    sa.sa_flags     = SA_SIGINFO;
    sigaction(SIGUSR1, &sa, NULL);


    std::set<pid_t> running_pids;
    std::map<pid_t, size_t> pid2task;
    size_t n_task = 0;

    while (n_task < num_tasks || !running_pids.empty()) {

        // start new processes
        while (n_task < num_tasks && running_pids.size() < max_concurr) {
            pid_t child_pid = fork();
            if (child_pid == 0) { // child
                signal(SIGUSR1, SIG_DFL);
                return n_task;
            }
            else if (child_pid > 0) {
                pid2task.insert(make_pair(child_pid, n_task));
                running_pids.insert(child_pid);
                n_task++;
                if (verbose)
                    cout << child_pid << ": START" << endl;
            }
            else {
                perror_exit("fork");
            }
        }

        // wait for process termination and/or signals
        int exitcode;
        pid_t term_pid = waitpid(0, &exitcode, 0);

        if (-1 == term_pid && EINTR == errno) {
            // received a signal
            // block SIGUSR1
            if (sigprocmask(SIG_BLOCK, &mask, &orig_mask) < 0)
                perror_exit("sigprocmask");

            std::set<pid_t> signaling_pids = g_signaling_pids;
            g_signaling_pids.clear();

            // restore SIGUSR1
            if (sigprocmask(SIG_SETMASK, &orig_mask, NULL) < 0)
                perror_exit("sigprocmask");

            if (!have_signaling_proc && !signaling_pids.empty()) {
                pid_t sig_pid = *signaling_pids.begin();
                if (verbose)
                    cout << sig_pid << ": SIGUSR1" << endl;
                if (running_pids.count(sig_pid)) {
                	// for (int i=0; i<5;i++) {
                	// 	cout << "SLEEP!!!"<<endl;
                	// 	sleep(1);
                	// }
                	// notify the signalling process to continue
                	// the child process should be suspended 
                	// in a sigsuspend() loop
                	kill(sig_pid, SIGUSR1);

                    // kill all other running processes
                    for (pid_t pid : running_pids)
                        if (pid != sig_pid)
                            kill(pid, SIGTERM);

                    // do not start other tasks
                    n_task = num_tasks;
                    have_signaling_proc = true;
                    if (p_signaling_proc_index)
                        *p_signaling_proc_index = pid2task[sig_pid];
                }
            }
        }
        else {
            // child termination
            if (running_pids.count(term_pid)) {
                out_exitcodes[pid2task[term_pid]] = exitcode;
                pid2task.erase(term_pid);
                running_pids.erase(term_pid);
                if (verbose)
                    cout << term_pid << ": TERM exitcode=" << exitcode << endl;
            }
        }
    }

    signal(SIGUSR1, SIG_DFL);    
    return -1;
}

//---------------------------------------------------------------------------------------

static volatile sig_atomic_t g_has_received_notification_to_continue = 0;

void handle_sigusr1_continue(int) {
    g_has_received_notification_to_continue = 1;
}

// notify the parent process with a SIGUSR1 and wait for its response to continue
// this method must be called by the parallel signalling process.
void notify_parent_and_continue() {
	// cout << "notify_parent_and_continue" << endl;
    // notify completion to the parent process, 
    // and wait for its signal to continue (to avoid race conditions)
    sigset_t mask, oldmask;
    sigemptyset (&mask);
    sigaddset (&mask, SIGUSR1);
    // get ready to receive response from parent
    signal(SIGUSR1, handle_sigusr1_continue);
    sigprocmask (SIG_BLOCK, &mask, &oldmask);
    g_has_received_notification_to_continue = 0;
    // notify parent
    kill(getppid(), SIGUSR1);
    // wait for parent'signal to continue running
    while (0 == g_has_received_notification_to_continue)
        sigsuspend (&oldmask);

    sigprocmask (SIG_UNBLOCK, &mask, NULL);
    signal(SIGUSR1, SIG_DFL);	
}

//---------------------------------------------------------------------------------------

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
                    size_t buffer_size, bool verbose)
{
    out_exitcodes.resize(num_tasks);
    std::fill(out_exitcodes.begin(), out_exitcodes.end(), PROC_NEVER_RUN);
    out_lengths.resize(num_tasks);
    std::fill(out_lengths.begin(), out_lengths.end(), 0);
    out_memblocks.resize(num_tasks);
    std::fill(out_memblocks.begin(), out_memblocks.end(), nullptr);


    std::set<pid_t> running_pids;
    std::map<pid_t, ssize_t> pid2task;
    std::map<pid_t, int> pid2rdfd;
    char rd_buffer[buffer_size];

    auto read_from_fd = [&](int fd, int pid) -> bool {
        ssize_t task_id = pid2task[pid];
        size_t num_bytes = read(fd, rd_buffer, buffer_size);

        if (num_bytes > 0) {
            out_memblocks[task_id] = realloc(out_memblocks[task_id], 
            							     out_lengths[task_id] + num_bytes);
            memcpy((char*)out_memblocks[task_id] + out_lengths[task_id],
                   rd_buffer, num_bytes);
            out_lengths[task_id] += num_bytes;
            if (verbose)
                cout << "Reading " << num_bytes << " for pid " << pid 
                     << ", block_size=" << out_lengths[task_id] << endl;
        }
        else if (num_bytes == 0) { // reading EOF
            close(fd);
            pid2rdfd.erase(pid);
        }
        return num_bytes;
    };


    ssize_t n_task = 0;
    while (n_task < num_tasks || !running_pids.empty()) {

        // start new processes
        while (n_task < num_tasks && running_pids.size() < max_concurr) {
            int pipefd[2]; // read, write
            if (pipe(pipefd)) // create the pipe
                perror_exit("pipe");
            pid_t child_pid = fork();
            if (child_pid == 0) { // child
                close(pipefd[0]); // close the read-end
                return proc_wback_t{.task_id = n_task, 
                                    .write_fd = pipefd[1]};
            }
            else if (child_pid > 0) {
                close(pipefd[1]); // close the write-end
                pid2task.insert(make_pair(child_pid, n_task));
                pid2rdfd.insert(make_pair(child_pid, pipefd[0]));
                running_pids.insert(child_pid);
                n_task++;
                if (verbose)
                    cout << child_pid << ": START" << endl;
            }
            else {
                perror_exit("fork");
            }
        }

        // wait for read and/or process terminations
        if (!pid2rdfd.empty()) {
            fd_set rd_set;
            int maxfd = 0;
            FD_ZERO(&rd_set);
            for (pid_t pid : running_pids) {
            	if (pid2rdfd.count(pid)) {
	                int fd = pid2rdfd[pid];
	                FD_SET(fd, &rd_set);
	                maxfd = max(fd, maxfd);
	            }
            }
            int ready = select(maxfd + 1, &rd_set, nullptr, nullptr, nullptr);
            if (ready < 1) {
                if (errno == EINTR) {
                    // signal
                }
                else perror_exit("select");
            }
            else { 
                // read from the ready descriptors
                for (pid_t pid : running_pids) {
                	if (pid2rdfd.count(pid)) {
	                    int fd = pid2rdfd[pid];
	                    if (FD_ISSET(fd, &rd_set)) {
	                        int task_id = pid2task[pid];
	                        read_from_fd(fd, pid);
	                    }
	                }
                }
            }
        }

        // wait for process termination and/or signals
        while (true) {
            int exitcode;
            pid_t term_pid = waitpid(-1, &exitcode, WNOHANG);
            if (term_pid <= 0)
                break; // no more child to wait for

            // child termination
            if (running_pids.count(term_pid)) {
                running_pids.erase(term_pid);
                out_exitcodes[pid2task[term_pid]] = exitcode;
                while (pid2rdfd.count(term_pid)) {
                    if (read_from_fd(pid2rdfd[term_pid], term_pid) > 0) {
		                if (verbose)
		                    cout << term_pid << " final readings." << endl;                    	
                    }
                }
                pid2task.erase(term_pid);
                if (verbose)
                    cout << term_pid << ": TERM exitcode=" << exitcode << endl;
            }
        }
    }

    return proc_wback_t{.task_id = -1, .write_fd = 0};
}

//---------------------------------------------------------------------------------------

static bool s_mem_constrained = false;
static rlim_t s_prev_rlim_cur, s_prev_rlim_max;

// constraint the memory of the process to a specified number of megabytes. 
// When MB<=0, release the imposed constraint.
void constraint_address_space(int MB) {
    // cout << getpid() << ": constraint_address_space " << MB << endl;
    struct rlimit current;
    int result = getrlimit(RLIMIT_AS, &current);
    if (result != 0) {
        perror("getrlimit");
        exit(EXIT_FAILURE);
    }

    if (MB > 0) { // add memory constraint
        assert(s_mem_constrained == false);
        s_prev_rlim_cur = current.rlim_cur;
        s_prev_rlim_max = current.rlim_max;
        rlim_t new_lim = rlim_t(MB) * 1024 * 1024;
        current.rlim_cur = new_lim;
        current.rlim_max = RLIM_INFINITY;
        s_mem_constrained = true;
    }
    else { // restore previous limit
        assert(s_mem_constrained == true);
        current.rlim_cur = s_prev_rlim_cur;
        current.rlim_max = RLIM_INFINITY;
        s_mem_constrained = false;
    }

    result = setrlimit(RLIMIT_AS, &current);
    if (result != 0) {
        perror("setrlimit");
        exit(1);
    }
}

//---------------------------------------------------------------------------------------






//---------------------------------------------------------------------------------------

int semaphore_create() {
	int sem_id = semget(IPC_PRIVATE, 1, IPC_CREAT|0666);
	if (sem_id < 0)
		perror_exit("semget");

	semun arg;
	arg.val = 1;
	if (semctl(sem_id, 0, SETVAL, arg) == -1)
		perror_exit("semctl");

	return sem_id;
}

void semaphore_close(int sem_id) {
	semctl(sem_id, 0, IPC_RMID);
}

void semaphore_acquire(int sem_id) {
	if (sem_id < 0)
		return; // do nothing
	struct sembuf s;

	s.sem_num = 0;
	s.sem_op = -1;
	s.sem_flg = 0;

	if (semop(sem_id, &s, 1) == -1)
		perror_exit("semop");
}

void semaphore_release(int sem_id) {
	if (sem_id < 0)
		return; // do nothing
	struct sembuf s;

	s.sem_num = 0;
	s.sem_op = 1;
	s.sem_flg = 0;

	if (semop(sem_id, &s, 1) == -1)
		perror_exit("semop");
}

//---------------------------------------------------------------------------------------

























