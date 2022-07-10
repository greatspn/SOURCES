#include <stdio.h>
#include <ctype.h>
#include <unistd.h>
#include <limits.h>
#include <string.h>
#include <stdlib.h>
#include <spawn.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

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

#if defined(__CYGWIN__) 

// run a system process using posix_spawn.
// This is strictly needed on cygwin with the portable GreatSPN distribution,
// as the system() C API will not work properly. 
int execp_cmd(const char* exec_name, const char* const* args, int verbose) {
    // spawn the new process
    pid_t pid;
    int status = posix_spawnp(&pid, exec_name, NULL, NULL, 
                              (char* const*)args, get_environ());
    if (verbose) {
        printf("exec: ");
        for (int a=0; args[a]; ++a) {
            if (str_has_spaces(args[a]))
                printf("\"%s\" ", args[a]);
            else
                printf("%s ", args[a]);
        }
        printf("(pid=%d)\n", pid);
        fflush(stdout);
    }
    // verify proper exec
    if (status == 0) {
        do {
            if (waitpid(pid, &status, 0) != -1) {
                if (WEXITSTATUS(status) != 0) {
                    fprintf(stderr, "%s returned %d\n", exec_name, WEXITSTATUS(status));
                    fflush(stderr);
                }
                return WEXITSTATUS(status);
            } else {
                perror("ERROR: creating child process.");
                fflush(stderr);
                return -1;
            }
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    } else {
        fprintf(stderr, "ERROR: posix_spawn[%s]: %s\n", exec_name, strerror(status));
        fflush(stderr);
        return -1;
    }
}

#elif defined(__linux__) || defined(__APPLE__)

// Whenever the system() C API works properly, we can use it. 
// This also allows to use any custom shell changes to PATH.
// NOTE 1: on Cygwin we cannot use system(), because we do not ship /bin/sh
// NOTE 2: on macOS we NEED to use system() instead of posix_spawn(), as in the
//         portable app-image the environmental variables are too basic
//         and do not incorporate the changes in ~/.zshrc or ~/.bashrc .
int execp_cmd(const char* exec_name, const char* const* args, int verbose) {
    // recompose all arguments into a single string, using quotes when needed
    size_t sz = 50, i=0;
    while (args[i]) { // reserve space for the cmd string
        sz += strlen(args[i]) + 2;
        i++;
    }
    char *cmd = (char*)malloc(sz), *p=cmd;
    const char* q;
    i=0;
    while (args[i]) {
        int sp = str_has_spaces(args[i]);
        q=args[i];
        if (i>0) *p++ = ' ';
        if (sp) *p++ = '\"';
        while (*q) {
            if      (*q == '\"') { *p++ = '\\'; *p++ = '\"'; }
            else if (*q == '\'') { *p++ = '\\'; *p++ = '\''; }
            else *p++ = *q;
            q++;
        }
        if (sp) *p++ = '\"';
        i++;
    }
    *p = '\0';
    if (verbose)
        printf("system: %s\n", cmd);

    int status = system(cmd);
    free(cmd);

    return status;
}

#else
#error "Unimplemented! (maybe just need to adapt the above ifdef\'s)"
#endif

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
    return execp_cmd(args[0], args, 1);
}

//=============================================================================

int dot_to_pdf(const char *dot_fname, const char *pdf_fname)
{
    const char* const args[] = { "dot", dot_fname, "-Tpdf", "-o", pdf_fname, NULL };
    return execp_cmd(args[0], args, 1);
}

//=============================================================================

static int 
eps_to_pdf_bbox_cmd(const char* cmd, const char *eps_fname, 
                    const char *pdf_fname, int width, int height) 
{
    // To compute the bounding box
    // gs -sDEVICE=bbox -dNOSAFER -dNOPAUSE -dBATCH -f file.eps
    // To convert EPS to PDF
    // gs -sDEVICE=pdfwrite -dNOSAFER -dDEVICEWIDTHPOINTS=w -dDEVICEHEIGHTPOINTS=h 
    //    -o file.pdf file.eps
    char devW[64], devH[64];
    snprintf(devW, sizeof(devW), "-dDEVICEWIDTHPOINTS=%d", width);
    snprintf(devH, sizeof(devH), "-dDEVICEHEIGHTPOINTS=%d", height);
    const char* const args[] = {
        cmd, "-sDEVICE=pdfwrite", "-dNOSAFER", "-dQUIET",
        devW, devH, "-o", pdf_fname, eps_fname, NULL
    };
    return execp_cmd(args[0], args, 1);
}

//=============================================================================

// convert a eps file into a pdf file, using ghostscript.
// need to have the bounding box size, in points.
// return 0 on success
int eps_to_pdf_bbox(const char *eps_fname, const char *pdf_fname, int width, int height) 
{
#if defined(__CYGWIN__) 
    int ret;
    // In Windows, ghostscript has multiple names, depending if it is the 
    // 32-bit or the 64-bit version. We don't know which version is installed,
    // so try calling both.
    ret = eps_to_pdf_bbox_cmd("gswin32c.exe", eps_fname, pdf_fname, width, height);
    if (ret == 0)
        return ret; // 32-bit ghostscript found, command completed successfully
    return eps_to_pdf_bbox_cmd("gswin64c.exe", eps_fname, pdf_fname, width, height);
#else
    return eps_to_pdf_bbox_cmd("gs", eps_fname, pdf_fname, width, height);
#endif
}

//=============================================================================

static int from_GUI = -1;

// Is being invoked from the new Java-based GUI?
int invoked_from_gui() {
    const char *env;
    if (from_GUI == -1) { // Not yet determined
        env = getenv("FROM_GUI");
        from_GUI = (env != NULL && 0 == strcmp(env, "1"));
    }
    return (from_GUI != 0);
}

//=============================================================================

static int test_appimage_dir = 1;
static const char* appimage_dir;

// Is running from the portable app-image directory
const char* get_appimage_dir() {
    if (test_appimage_dir) { // Not yet determined
        appimage_dir = getenv("GREATSPN_APPIMAGE_DIR");
        if (appimage_dir != NULL && strlen(appimage_dir)==0)
            appimage_dir = NULL;
        test_appimage_dir = 0;
    }
    return appimage_dir;
}

//=============================================================================

// search for the path where the GreatSPN subcommand @bin can be found
// returns 0 if the binary is found
int search_greatspn_app(char cmdname[FILENAME_MAX], const char *bin) {
	const char *appimg_dir = get_appimage_dir(), *prefix;
	struct stat st;
	int ii=0;
	while (1) {
		int has_path = 0;
		switch (ii) {
			case 0:
				if (appimg_dir != NULL) {
					// When the application runs inside the appimage, test the appimage path first
					snprintf(cmdname, FILENAME_MAX,
							 "%s" PATH_SEPARATOR "bin" PATH_SEPARATOR "%s", appimg_dir, bin);
					has_path = 1;
				}
				break;

			case 1: // Local path: ./app
				snprintf(cmdname, FILENAME_MAX,
						 "." PATH_SEPARATOR "%s", bin);
				has_path = 1;
				break;

			case 2: // Local path: ./bin/app
				snprintf(cmdname, FILENAME_MAX,
						 "." PATH_SEPARATOR "bin" PATH_SEPARATOR "%s", bin);
				has_path = 1;
				break;

			case 3: // Using the GREATSPN_INSTALLDIR environment variable
				prefix = getenv("GREATSPN_INSTALLDIR");
				if (prefix != NULL) {
					snprintf(cmdname, FILENAME_MAX,
							 "%s" PATH_SEPARATOR "bin" PATH_SEPARATOR "%s", prefix, bin);
					has_path = 1;
				}
				break;

			case 4: // Using the GREATSPN_BINDIR environment variable
				prefix = getenv("GREATSPN_BINDIR");
				if (prefix != NULL) {
					snprintf(cmdname, FILENAME_MAX,
							 "%s" PATH_SEPARATOR "%s", prefix, bin);
					has_path = 1;
				}
				break;

			case 5: // Global Unix path
				snprintf(cmdname, FILENAME_MAX,
						 "/usr/local/GreatSPN/bin/%s", bin);
				has_path = 1;
				break;

#ifdef __CYGWIN__
			case 6: // Global Windows/Cygwin path
				snprintf(cmdname, FILENAME_MAX,
						 "C:\\Program Files\\GreatSPN\\app\\portable_greatspn\\bin\\%s", bin);
				has_path = 1;
				break;
#endif
			default:
				return -1; // Could not find a suitable path
		}

		if (has_path) {
			int ret = stat(cmdname, &st);
            //printf("TEST: %s   STAT=%d\n", cmdname, ret);
			if (0 == ret && S_ISREG(st.st_mode))
				return 0; // application find at @cmdname
		}

		ii++;
	}
}

//=============================================================================

// generate a temporary filename, using either the default or the extra TEMP dir
int portable_mkstemp(char buffer[FILENAME_MAX], const char* pattern) {
    char tempdir[FILENAME_MAX];
    struct stat st;
    const char *prefix;
    int ii = 0;
    while (1) {
        int has_tempdir = 0;
        switch (ii) {
            case 0: // Get the GreatSPN temporary directory, if defined
                prefix = getenv("GREATSPN_TEMP_DIR");
                if (prefix != NULL) {
                    strncpy(tempdir, prefix, FILENAME_MAX);
                    has_tempdir = 1;
                }
                break;

            case 1: // Get default Unix temporary directory
                strncpy(tempdir, "/tmp", FILENAME_MAX);
                has_tempdir = 1;
                break;
            
            case 2: // Use current directory
                strncpy(tempdir, ".", FILENAME_MAX);
                has_tempdir = 1;
                break;
            
            default:
                return -1;
        }

        if (has_tempdir) {
            int ret = stat(tempdir, &st);
            //printf("TMPTEST: %s   STAT=%d\n", tempdir, ret);
			if (0 == ret && S_ISDIR(st.st_mode)) {
                // We have a valid temporary directory
                snprintf(buffer, FILENAME_MAX, "%s" PATH_SEPARATOR "%s", tempdir, pattern);
                return mkstemp(buffer);
            }
        }

        ii++;
    }


    const char *tmpdir_env = getenv("GREATSPN_TEMP_DIR");
    if (tmpdir_env != NULL) {
        snprintf(buffer, FILENAME_MAX, "%s%s", tmpdir_env, pattern);
    }
    else {
        snprintf(buffer, FILENAME_MAX, "/tmp/%s", pattern); // assume the default Unix /tmp folder
    }
    return mkstemp(buffer);
}

//=============================================================================
