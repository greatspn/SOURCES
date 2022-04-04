#include <stdio.h>
#include <ctype.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <spawn.h>
#include <sys/wait.h>

#include "platform_utils.h"

//=============================================================================
// if defined(WIN) && (_MSC_VER >= 1900)  use *__p__environ();
#ifdef __APPLE__
#include <crt_externs.h>
char* const* get_environ() { return _NSGetEnviron(); }
#else
extern char **environ;
char* const* get_environ() { return environ; }
#endif

//=============================================================================

static int
str_has_spaces(const char*p) {
    while (*p) {
        if (isspace(*p))
            return 1;
        ++p;
    }
    return 0;
}

//=============================================================================

// #if defined(__CYGWIN__)

// run a system process using posix_spawn.
// This is strictly needed on cygwin with the portable GreatSPN distribution,
// as the system() C API will not work properly. 
int execp_cmd(const char* const* args, int verbose) {
    // spawn the new process
    pid_t pid;
    int status = posix_spawnp(&pid, args[0], NULL, NULL, 
                              (char* const*)args, get_environ());
    // verify proper exec
    if (status == 0) {
        if (verbose) {
            printf("exec: ");
            for (int a=0; args[a]; ++a) {
                if (str_has_spaces(args[a]))
                    printf("\"%s\" ", args[a]);
                else
                    printf("%s ", args[a]);
            }
            printf("(pid=%d)\n", pid);
        }
        do {
            if (waitpid(pid, &status, 0) != -1) {
                if (WEXITSTATUS(status) != 0)
                    fprintf(stderr, "%s returned %d\n", args[0], WEXITSTATUS(status));
                return WEXITSTATUS(status);
            } else {
                perror("ERROR: creating child process.");
                return -1;
            }
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    } else {
        fprintf(stderr, "ERROR: posix_spawn: %s\n", strerror(status));
        return -1;
    }
}

// #elif defined(__linux__) || defined(__APPLE__)

// // Whenever the system() C API works properly, we can use it. 
// // This also allows to use any custom shell changes to PATH.
// NOTE: thid could surprisingly does not work in the portable macOS x86 version.
// int execp_cmd(const char* const* args, int verbose) {
//     // recompose all arguments into a single string, using quotes when needed
//     size_t sz = 50, i=0;
//     while (args[i]) { // reserve space for the cmd string
//         sz += strlen(args[i]) + 2;
//         i++;
//     }
//     char *cmd = (char*)malloc(sz), *p=cmd;
//     const char* q;
//     i=0;
//     while (args[i]) {
//         int sp = str_has_spaces(args[i]);
//         q=args[i];
//         if (i>0) *p++ = ' ';
//         if (sp) *p++ = '\"';
//         while (*q) {
//             if      (*q == '\"') { *p++ = '\\'; *p++ = '\"'; }
//             else if (*q == '\'') { *p++ = '\\'; *p++ = '\''; }
//             else *p++ = *q;
//             q++;
//         }
//         if (sp) *p++ = '\"';
//         i++;
//     }
//     *p = '\0';
//     if (verbose)
//         printf("system: %s\n", cmd);

//     int status = system(cmd);
//     free(cmd);

//     return status;
// }

// #else
// #error "Unimplemented! (maybe just need to adapt the above ifdef\'s)"
// #endif

//=============================================================================

int open_file(const char* filename)
{
#ifdef __CYGWIN__
    const char* const args[] = {"cmd", "/C", "START", filename, NULL};
#elif defined(__linux__) 
    const char* const args[] = {"xdg-open", filename, NULL};
#elif defined(__APPLE__)
    const char* const args[] = {"open", filename, NULL};
#else
#error Unimplemented.
#endif
    return execp_cmd(args, 1);
}

//=============================================================================

int dot_to_pdf(const char *dot_fname, const char *pdf_fname)
{
    const char* const args[] = { "dot", dot_fname, "-Tpdf", "-o", pdf_fname, NULL };
    return execp_cmd(args, 1);
}

//=============================================================================

