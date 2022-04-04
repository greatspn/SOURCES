#ifndef __PLATFORM_UTILS_H__
#define __PLATFORM_UTILS_H__

#ifdef __cplusplus
extern "C" {
#endif

//=============================================================================
// Utilities to simplify cross-platform portability
//=============================================================================

// return the vector of environment variables
char* const* get_environ();

// open a file with the default viewer
// filename doesn't need to be quoted
// return 0 on success
int open_file(const char* filename);

// spawn a new process with the given arguments
// file arguments don't need to be quoted
// return 0 on success
int execp_cmd(const char* const* args, int verbose);

// convert a dot file into a pdf file using Graphviz.
// file arguments don't need to be quoted
// return 0 on success
int dot_to_pdf(const char *dot_fname, const char *pdf_fname);

//=============================================================================

#ifdef __cplusplus
} // extern "C"
#endif

#endif // __PLATFORM_UTILS_H__