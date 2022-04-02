#ifndef __PLATFORM_UTILS_H__
#define __PLATFORM_UTILS_H__

#ifdef __cplusplus
extern "C" {
#endif

//=============================================================================
// Utilities to simplify cross-platform portability
//=============================================================================

// return the vector of environment variables
char * const* get_environ();

// open a file with the default viewer
// return 0 on success
int open_file(const char* filename);

// spawn a new process with the given arguments
int execp_cmd(const char* const* args, int verbose);

// convert a dot file into a pdf file using Graphviz
int dot_to_pdf(const char *dot_fname, const char *pdf_fname);

//=============================================================================

#ifdef __cplusplus
} // extern "C"
#endif

#endif // __PLATFORM_UTILS_H__