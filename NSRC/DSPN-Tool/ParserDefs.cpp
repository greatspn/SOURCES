
#include <cassert>
#include <cstdio>
#include <iostream>
#include <memory>
#include <boost/noncopyable.hpp>
using namespace std;

#include "ParserDefs.h"

//=============================================================================

#define SPECIAL_STRINGSTREAM_FILENO   -1234

//=============================================================================

size_t STRINGSTREAM::stream_fread(void *ptr, size_t size, size_t nmemb) {
    assert(size == 1); // the size>1 case is not yet implemented
    pStream->read((char *)ptr, size * nmemb);
    streamsize readcnt = pStream->gcount();
    if (pStream->bad())
        return 0;
    else return readcnt;
}

//=============================================================================

size_t STRINGSTREAM::stream_fwrite(const void *ptr, size_t size, size_t nmemb) {
    assert(size == 1); // the size>1 case is not yet implemented
    pStream->write((const char *)ptr, size * nmemb);
    if (pStream->bad())
        return 0;
    else return 1;
}

//=============================================================================

void STRINGSTREAM::stream_clearerr() {
    pStream->clear();
}

//=============================================================================

int STRINGSTREAM::stream_ferror() {
    return pStream->bad();
}

//=============================================================================

int STRINGSTREAM::stream_getc() {
    char c;
    pStream->get(c);
    if (pStream->eof())
        return EOF;
    return c;
}

//=============================================================================

int STRINGSTREAM::stream_fileno() {
    return SPECIAL_STRINGSTREAM_FILENO;
}

//=============================================================================

int STRINGSTREAM::stream_isatty(int filenumber) {
    assert(filenumber == SPECIAL_STRINGSTREAM_FILENO);
    // returns true if in interactive mode
    return false;
}

//=============================================================================




