/*
 *  utils.cpp
 *
 *  General utilities used throughout the software
 *
 *  Created by Elvio Amparore
 *
 */

#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <cmath>
#include <cfloat>
#include <cassert>
#include <cstring>
#include <limits>
#include <queue>
#include <memory>
#include <stdarg.h>
using namespace std;

#if !defined WIN32 && !defined __MINGW32__
# include <sys/time.h>
# include <sys/resource.h>
# include <unistd.h>
#endif

#include <boost/utility.hpp>

#include "../numeric/numeric.h"
using namespace numerical;

#include "../platform/platform_utils.h"

#include "utils.h"

/*TODO:remove
struct X : public std::vector<int>, boost::noncopyable {};
struct Y : public boost::noncopyable { string name; };

void test() {
	vector<Y> names;
	X xes;
	cout << print_indexed_names(xes, names);
}//*/

//=============================================================================

void verify_thr(const char *expr, const char *file, const size_t line) {
    ostringstream oss;
    oss << "verify(" << expr << ") failed at " << file << ":" << line;
    throw program_exception(oss.str());
}

//=============================================================================

const vec_out_style_t s_pvfmtBracketsSpaced = { "{", " ", ", ", "}" };
const vec_out_style_t s_pvfmtBrackets       = { "{", "", ",", "}" };
const vec_out_style_t s_pvfmtSquaredSpaced  = { "[", " ", ", ", "]" };
const vec_out_style_t s_pvfmtSquared        = { "[", "", ",", "]" };
const vec_out_style_t s_pvfmtPathSep        = { "", "", PATH_SEP_S, "" };

//=============================================================================

std::function<ostream& (ostream &)>
fmtdbl(double v, const char *fmt) {
    return [ = ](ostream & os) -> ostream& {
        char buf[32];
        sprintf(buf, fmt, v);
        return os << buf;
    };
}

//=============================================================================

std::function<ostream& (ostream &)>
print_vec_ublas(const ublas::vector<double> &v) {
    return [&](ostream & os) -> ostream& {
        os << '[' << v.size() << "][";
        for (size_t i = 0; i < v.size(); i++)
            os << (i > 0 ? "," : "") << v(i);
        return os << "]";
    };
}

//=============================================================================

string getTimeStamp(double numSec, bool printMilliSec /*=false*/, bool printHourMinutes) {
    if (std::isnan(numSec))
        return string("");
    ostringstream str;
    double secPart, fractPart;
    fractPart = modf(numSec, &secPart);
    size_t seconds = size_t(secPart);
    if (printHourMinutes) {
        size_t minutes = seconds / 60;
        seconds -= minutes * 60;
        size_t hours = minutes / 60;
        minutes -= hours * 60;
        if (hours > 0)		str << hours << "h ";
        if (minutes > 0)	str << minutes << "m ";
    }
    str << seconds;
    if (printMilliSec) {
        size_t msec = size_t(fractPart * 1000.0);
        str << "." << setw(3) << setfill('0') << right << msec;
    }
    str << "s";
    return str.str();
}

//=============================================================================
//  long_interval_timer implementation
//=============================================================================

// Update frequency of the various console progress lines.
clock_t long_interval_timer::frequency = clock_t(-1);

clock_t long_interval_timer::get_frequency() {
    if (frequency == clock_t(-1)) {
        const char *freq = getenv("FREQUENCY");
        if (freq != nullptr)
            frequency = (atoi(freq) * CLOCKS_PER_SEC) / 10;
        else
            frequency = CLOCKS_PER_SEC / 4;
    }
    return frequency;
}

long_interval_timer::long_interval_timer() {
    reset();
}

/// Resets the timer
void long_interval_timer::reset() {
    startClock = clock();
    lastPrintTime = -get_frequency();
    numPrints = 0;
}

/// Tells if a message has to be printed or not.
long_interval_timer::operator bool() {
    clock_t currClock = clock();
    clock_t elapsedTime = currClock - startClock;
    if (elapsedTime > lastPrintTime + get_frequency()) {
        lastPrintTime = elapsedTime;
        numPrints++;
        return true;
    }
    return false;
}

/// Elapsed time, in seconds
double long_interval_timer::get_elapsed_time() {
    clock_t currClock = clock();
    return double(currClock - startClock) / CLOCKS_PER_SEC;
}

/// Print a time estimate before completing the current task
void long_interval_timer::print_estimate(size_t numDone, size_t total) {
    double elapsedTime = get_elapsed_time();
    double estimatedTime = (elapsedTime / (numDone - 1)) * (total - 1);
    double remainedTime = estimatedTime - elapsedTime;
    cout << getTimeStamp(remainedTime + 0.5, false);
}

/// Prepares a message print
void long_interval_timer::prepare_print() {
    if (numPrints > 1)
        cout << console::one_line_up();
}

/// Call this function to clear any previous message outputs
void long_interval_timer::clear_any() {
    if (numPrints > 0) {
        const char *SPCS39 = "                                       ";
        cout << console::one_line_up() << SPCS39 << SPCS39 << endl;
        cout << console::one_line_up();
        reset();
    }
}


//=============================================================================
//  performance_timer implementation
//=============================================================================

#ifdef WIN32
void performance_timer::start() {
    FILETIME CreationTime, ExitTime;
    GetProcessTimes(GetCurrentProcess(), &CreationTime, &ExitTime,
                    &KernelTimeStart, &UserTimeStart);
}
double performance_timer::filetimediff(const FILETIME &ft1, const FILETIME &ft2) {
    ULARGE_INTEGER u1, u2;
    u1.LowPart  = ft1.dwLowDateTime;
    u1.HighPart = ft1.dwHighDateTime;
    u2.LowPart  = ft2.dwLowDateTime;
    u2.HighPart = ft2.dwHighDateTime;
    return double(u1.QuadPart - u2.QuadPart);
}
void performance_timer::stop() {
    FILETIME UserTimeStop, KernelTimeStop, CreationTime, ExitTime;
    GetProcessTimes(GetCurrentProcess(), &CreationTime, &ExitTime,
                    &KernelTimeStop, &UserTimeStop);
    userSecs   = filetimediff(UserTimeStop, UserTimeStart) / 1.0e7;
    systemSecs = filetimediff(KernelTimeStop, KernelTimeStart) / 1.0e7;
}
#else
void performance_timer::start() {
    pStartRUsage = make_shared<rusage>();
    getrusage(RUSAGE_SELF, pStartRUsage.get());
}
double performance_timer::timevaldiff(const struct timeval &tv1, const struct timeval &tv2) {
    double t1 = tv1.tv_sec + 1.0e-6 * tv1.tv_usec;
    double t2 = tv2.tv_sec + 1.0e-6 * tv2.tv_usec;
    return (t1 - t2);
}
void performance_timer::stop() {
    struct rusage stopRUsage;
    getrusage(RUSAGE_SELF, &stopRUsage);
    userSecs   = timevaldiff(stopRUsage.ru_utime, pStartRUsage->ru_utime);
    systemSecs = timevaldiff(stopRUsage.ru_stime, pStartRUsage->ru_stime);
}
#endif

performance_timer::performance_timer() {
    start();
}

/// Get a representation of the user/system time elapsed from the construction
string performance_timer::get_timer_repr() {
    stop();
    ostringstream str;
    str << "[User " << getTimeStamp(userSecs, true) << ",";
    str << " Sys " << getTimeStamp(systemSecs, true) << "]";
    return str.str();
}


ostream &operator <<(ostream &os, performance_timer &pt) {
    os << pt.get_timer_repr();
    return os;
}

//=============================================================================

LinearIterationPrinter::LinearIterationPrinter(VerboseLevel vl, long_interval_timer *p)
    : verboseLvl(vl), pTimer(p) {
    if (pTimer == nullptr)
        pTimer = &internalTimer;
}

void LinearIterationPrinter::onIteration(AlgoName an, size_t iter, double resid) {
    if (verboseLvl >= VL_BASIC && *pTimer) {
        pTimer->prepare_print();
        cout << an << ": iteration=" << iter << ", err=";
        cout << fmtdbl(resid, "%.5le") << "               " << endl;
    }
}

void LinearIterationPrinter::onConvergence(AlgoName an, size_t iter, double resid) {
    if (verboseLvl >= VL_BASIC) {
        pTimer->clear_any();
        cout << an << ": convergence in " << iter << " iterations, err=";
        cout << fmtdbl(resid, "%.5le");// << ", u*P products=" << uPprods;
        cout << "               " << endl;
    }
}

void LinearIterationPrinter::onMessage(bool isLastMsg, const char *fmt, ...) {
    if (verboseLvl < VL_BASIC)
        return;

    // Format the message
    char message[1024];
    va_list args;
    va_start(args, fmt);
    vsprintf(message, fmt, args);
    va_end(args);

    if (!isLastMsg) {
        if (!*pTimer)
            return;
        pTimer->prepare_print();
    }
    else
        pTimer->clear_any();

    cout << message << "           " << endl;
}

void LinearIterationPrinter::onFailure(AlgoName an, size_t iter, ConvergenceFailureReason cfr) {
    if (verboseLvl >= VL_BASIC) {
        pTimer->clear_any();
        cout << an;
        switch (cfr) {
        case CFR_TOO_MANY_ITERATIONS:
            cout << ": failed to converge in " << iter << " iterations.";
            break;
        case CFR_BREAKDOWN:
            cout << ": solver breakdown.              ";
            break;
        case CFR_BREAKDOWN_SWITCH_TO_POWERMETHOD:
            cout << ": solver breakdown, switch to Power Method.";
            break;
        }
        cout << "                   " << endl;
    }
}

void LinearIterationPrinter::onUnif(double t, double lambda, size_t M,
                                    size_t R, size_t vecMatProdCount) {
    if (verboseLvl >= VL_BASIC) {
        size_t MR = M * R;
        if (vecMatProdCount < MR) {
            if (!*pTimer)
                return;
            pTimer->prepare_print();
        }
        else
            pTimer->clear_any();

        cout << "UNIFORMIZATION(t=" << t << ", Lambda=" << lambda << ", ";
        cout << M << " steps, R=" << R << "): ";

        if (vecMatProdCount < MR)
            cout << vecMatProdCount << "/" << MR;
        else
            cout << MR;
        cout << " Vec*Mat products.              " << endl;
    }
}

void LinearIterationPrinter::onUnifGen(const char *fg, double lambda,
                                       size_t R, size_t vecMatProdCount) {
    if (verboseLvl >= VL_BASIC) {
        if (vecMatProdCount < R) {
            if (!*pTimer)
                return;
            pTimer->prepare_print();
        }
        else
            pTimer->clear_any();

        cout << "UNIFORMIZATION(fg=" << fg << ", Lambda=" << lambda << ", step=" << R << "): ";

        if (vecMatProdCount < R)
            cout << vecMatProdCount << "/" << R;
        else
            cout << R;
        cout << " Vec*Mat products.              " << endl;
    }
}

VerboseLevel LinearIterationPrinter::getVerboseLevel() const {
    return verboseLvl;
}

//=============================================================================

void PrintAsMathematicaMatrix(const ublas::compressed_matrix<double> &M,
                              const char *name) {
    cout << name << " = {";
    for (size_t j = 0; j < M.size2(); j++) {
        if (j > 0)
            cout << ",";
        cout << "{";
        for (size_t i = 0; i < M.size1(); i++) {
            if (i > 0)
                cout << ",";
            cout << M(i, j);
        }
        cout << "}";
    }
    cout << "};" << endl;
}

//=============================================================================





#ifdef WIN32
//=========================================================================
// Windows Console API
//=========================================================================
static HANDLE g_stdout_handle = INVALID_HANDLE_VALUE;
inline HANDLE console::get_stdout_handle() {
    if (g_stdout_handle == INVALID_HANDLE_VALUE) {
        g_stdout_handle = GetStdHandle(STD_OUTPUT_HANDLE);
        get_default_stdout_attr(); // initialize
    }
    return g_stdout_handle;
}
static WORD g_default_stdout_attr = 0xFFFF;
inline WORD console::get_default_stdout_attr() {
    if (g_default_stdout_attr == 0xFFFF) {
        CONSOLE_SCREEN_BUFFER_INFO csbi;
        if (GetConsoleScreenBufferInfo(get_stdout_handle(), &csbi))
            g_default_stdout_attr = csbi.wAttributes;
    }
    return g_default_stdout_attr;
}

#define FOREGROUND_CLR_MASK \
	(FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED)
#define BACKGROUND_CLR_MASK \
	(BACKGROUND_BLUE | BACKGROUND_GREEN | BACKGROUND_RED)
#define CLR_MASK   (FOREGROUND_CLR_MASK | BACKGROUND_BLUE)

#endif // WIN32


//=============================================================================
// Console functionalities
//=============================================================================

namespace console {

//=============================================================================
static void print_XTerm_escape_sequence(std::ostream &os, const console::escape_seq &esc) {
    using namespace console;
    switch (esc.cmd) {
    // Attributes
    case EC_DEFAULT_DISP:    os << "\033[0m";	break;
    case EC_BOLD_ON:         os << "\033[1m";	break;
    case EC_DIM_ON:          os << "\033[2m";	break;
    case EC_UNDERLINE_ON:    os << "\033[4m";	break;
    case EC_BLINK_ON:        os << "\033[5m";	break;
    case EC_REVERSE_ON:      os << "\033[7m";	break;
    case EC_HIDDEN_ON:       os << "\033[8m";	break;

    case EC_BOLD_OFF:        os << "\033[22m";	break;
    case EC_DIM_OFF:         os << "\033[23m";	break;
    case EC_UNDERLINE_OFF:   os << "\033[24m";	break;
    case EC_BLINK_OFF:       os << "\033[25m";	break;
    case EC_REVERSE_OFF:     os << "\033[27m";	break;
    case EC_HIDDEN_OFF:      os << "\033[28m";	break;

    // Color Codes
    case EC_COLOR_FGND_OFF:  os << "\033[29m";	break;

    case EC_BLACK_FGND:      os << "\033[30m";	break;
    case EC_RED_FGND:        os << "\033[31m";	break;
    case EC_GREEN_FGND:      os << "\033[32m";	break;
    case EC_YELLOW_FGND:     os << "\033[33m";	break;
    case EC_BLUE_FGND:       os << "\033[34m";	break;
    case EC_MAGENTA_FGND:    os << "\033[35m";	break;
    case EC_CYAN_FGND:       os << "\033[36m";	break;
    case EC_WHITE_FGND:      os << "\033[37m";	break;

    case EC_BLACK_BGND:      os << "\033[40m";	break;
    case EC_RED_BGND:        os << "\033[41m";	break;
    case EC_GREEN_BGND:      os << "\033[42m";	break;
    case EC_YELLOW_BGND:     os << "\033[43m";	break;
    case EC_BLUE_BGND:       os << "\033[44m";	break;
    case EC_MAGENTA_BGND:    os << "\033[45m";	break;
    case EC_CYAN_BGND:       os << "\033[46m";	break;
    case EC_WHITE_BGND:      os << "\033[47m";	break;

    // Cursor Position
    case EC_CUR_XY:
        os << '\033' << '[' << esc.dy << ";" << esc.dx << "H";
        break;
    case EC_CUR_UP:
        os << '\033' << '[' << esc.dy << "A";
        break;
    case EC_CUR_DOWN:
        os << '\033' << '[' << esc.dy << "B";
        break;
    case EC_CUR_RIGHT:
        os << '\033' << '[' << esc.dx << "C";
        break;
    case EC_CUR_LEFT:
        os << '\033' << '[' << esc.dx << "D";
        break;

    // Complex combined commands
    case EC_BEG_ERROR:     os << console::bold_on() << console::red_fgnd();            return;
    case EC_END_ERROR:     os << console::bold_off() << console::default_disp();       return;
    case EC_BEG_TITLE:     os << console::bold_on() << console::underline_on();        return;
    case EC_END_TITLE:     os << console::bold_off() << console::underline_off();      return;
    case EC_BEG_SUCCESS:
        os << console::bold_on() << console::underline_on()
           << console::green_fgnd();
        return;
    case EC_END_SUCCESS:
        os << console::bold_off() << console::underline_off()
           << console::default_disp();
        return;
    case EC_BEG_EMPH:      os << console::underline_on() << console::yellow_fgnd();     return;
    case EC_END_EMPH:      os << console::underline_off() << console::default_disp();   return;
    case EC_ONE_LINE_UP:   os << console::cur_up(1);                	                return;

    default:
        throw program_exception("Internal error: illegal escape sequence code.");
    }
}

#ifdef WIN32

//=============================================================================
static void print_Win32_escape_sequence(std::ostream &os, const console::escape_seq &esc) {
    // Console parameters
    WORD  wRemovedAttr, wAddedAttr;
    int dx = 0, dy = 0;
    bool absolute = false;

    // Simple setup for color changes
    auto makecolor = [&](WORD clr) {
        wRemovedAttr = FOREGROUND_CLR_MASK | FOREGROUND_INTENSITY;
        wAddedAttr = clr;
    };

    // Decode the console command
    switch (esc.cmd) {
    case EC_DEFAULT_DISP:
        wRemovedAttr = 0xFFFF;  wAddedAttr = get_default_stdout_attr();
        break;

    case EC_BOLD_ON:
        wRemovedAttr = 0;  wAddedAttr = 0;
        break;
    case EC_DIM_ON:
        wRemovedAttr = FOREGROUND_INTENSITY;  wAddedAttr = 0;
        break;
    case EC_UNDERLINE_ON:
        wRemovedAttr = 0;  wAddedAttr = COMMON_LVB_UNDERSCORE;
        break;
    case EC_BLINK_ON:
        wRemovedAttr = 0;  wAddedAttr = 0;
        break;
    case EC_REVERSE_ON:
        wRemovedAttr = 0;  wAddedAttr = COMMON_LVB_REVERSE_VIDEO;
        break;
    case EC_HIDDEN_ON:
        wRemovedAttr = 0;  wAddedAttr = 0;
        break;

    case EC_BOLD_OFF:
        wRemovedAttr = 0;  wAddedAttr = 0;
        break;
    case EC_DIM_OFF:
        wRemovedAttr = 0;  wAddedAttr = FOREGROUND_INTENSITY;
        break;
    case EC_UNDERLINE_OFF:
        wRemovedAttr = COMMON_LVB_UNDERSCORE;  wAddedAttr = 0;
        break;
    case EC_BLINK_OFF:
        wRemovedAttr = 0;  wAddedAttr = 0;
        break;
    case EC_REVERSE_OFF:
        wRemovedAttr = COMMON_LVB_REVERSE_VIDEO;  wAddedAttr = 0;
        break;
    case EC_HIDDEN_OFF:
        wRemovedAttr = 0;  wAddedAttr = 0;
        break

    case EC_COLOR_FGND_OFF:
        makecolor(get_default_stdout_attr() & (FOREGROUND_CLR_MASK | FOREGROUND_INTENSITY));
        break;

    case EC_BLACK_FGND:    makecolor(0);   														break;
    case EC_RED_FGND:      makecolor(FOREGROUND_RED);   										break;
    case EC_GREEN_FGND:    makecolor(FOREGROUND_GREEN);   										break;
    case EC_YELLOW_FGND:   makecolor(FOREGROUND_RED | FOREGROUND_GREEN);   						break;
    case EC_BLUE_FGND:     makecolor(FOREGROUND_BLUE);   										break;
    case EC_MAGENTA_FGND:  makecolor(FOREGROUND_RED | FOREGROUND_BLUE);   						break;
    case EC_CYAN_FGND:     makecolor(FOREGROUND_BLUE | FOREGROUND_GREEN);   					break;
    case EC_WHITE_FGND:    makecolor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);   	break;

    case EC_BLACK_BGND:    makecolor(0);   														break;
    case EC_RED_BGND:      makecolor(BACKGROUND_RED);   										break;
    case EC_GREEN_BGND:    makecolor(BACKGROUND_GREEN);   										break;
    case EC_YELLOW_BGND:   makecolor(BACKGROUND_RED | BACKGROUND_GREEN);   						break;
    case EC_BLUE_BGND:     makecolor(BACKGROUND_BLUE);   										break;
    case EC_MAGENTA_BGND:  makecolor(BACKGROUND_RED | BACKGROUND_BLUE);   						break;
    case EC_CYAN_BGND:     makecolor(BACKGROUND_BLUE | BACKGROUND_GREEN);   					break;
    case EC_WHITE_BGND:    makecolor(BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE);   	break;

    case EC_CUR_XY:
        wRemovedAttr = wAddedAttr = 0;
        dx = esc.dx;
        dy = esc.dy;
        absolute = true;
        break;

    case EC_CUR_UP:
        wRemovedAttr = wAddedAttr = 0;
        dy = -esc.dy;
        break;

    case EC_CUR_DOWN:
        wRemovedAttr = wAddedAttr = 0;
        dy = esc.dy;
        break;

    case EC_CUR_RIGHT:
        wRemovedAttr = wAddedAttr = 0;
        dx = esc.xy;
        break;

    case EC_CUR_LEFT:
        wRemovedAttr = wAddedAttr = 0;
        dx = -esc.xy;
        break;

    // Complex combined commands
    case EC_BEG_ERROR:     os << console::red_fgnd();     return;
    case EC_END_ERROR:     os << console::white_fgnd();   return;
    case EC_BEG_TITLE:     os << console::dim_off();      return;
    case EC_END_TITLE:     os << console::dim_on();       return;
    case EC_BEG_SUCCESS:   os << console::green_fgnd();   return;
    case EC_END_SUCCESS:   os << console::white_fgnd();   return;
    case EC_BEG_EMPH:      os << console::yellow_fgnd();  return;
    case EC_END_EMPH:      os << console::white_fgnd();   return;
    case EC_ONE_LINE_UP:   os << console::cur_up(1);      return;

    default:
        throw program_exception("Internal error: illegal escape sequence code.");
    }

    // Execute the Win32 Console command
    os << flush;
    COORD coord;
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    HANDLE stdout_handle = get_stdout_handle();
    if (GetConsoleScreenBufferInfo(stdout_handle, &csbi)) {
        WORD newAttr = csbi.wAttributes;
        newAttr &= ~es.wRemovedAttr;
        newAttr |= es.wAddedAttr;
        if (!es.absolute) {
            coord.X = SHORT(csbi.dwCursorPosition.X + es.dx);
            coord.Y = SHORT(csbi.dwCursorPosition.Y + es.dy);
        }
        else {
            coord.X = es.dx;
            coord.Y = es.dy;
        }
        if (newAttr != csbi.wAttributes)
            SetConsoleTextAttribute(stdout_handle, newAttr);
        if (coord.X != csbi.dwCursorPosition.X ||
                coord.Y != csbi.dwCursorPosition.Y)
            SetConsoleCursorPosition(stdout_handle, coord);
    }
}

#endif // WIN32
}; // namespace console

//=============================================================================
// Should print using XTerm escape sequences??
static int g_useXTermEscapeSequences = -1;

std::ostream &operator << (std::ostream &os, const console::escape_seq &esc) {
    if (g_useXTermEscapeSequences == -1) {
        const char *term_evar = getenv("TERM");
        if (invoked_from_gui() ||
                (term_evar != nullptr &&
                 (0 == strcmp(term_evar, "xterm") ||
                  0 == strcmp(term_evar, "xterm-color") ||
                  0 == strcmp(term_evar, "xterm-256color"))
                ))
            g_useXTermEscapeSequences = 1;
        else
            g_useXTermEscapeSequences = 0;
    }
    if (g_useXTermEscapeSequences) {
        // XTerm color command
        console::print_XTerm_escape_sequence(os, esc);
    }
#ifdef WIN32
    else {
        // Using Win32 Console API
        console::print_Win32_escape_sequence(os, esc);
    }
#endif // WIN32
    return os;
}

//=============================================================================




//=============================================================================
// Simple tokenizer implementation
//=============================================================================

simple_tokenizer &simple_tokenizer::readtok(const char *token) {
    skip_sp(*this);
    const char *prt = token;
    int ch;
    while (*prt != '\0' && get(ch)) {
        if (ch == *prt)
            prt++;
        else {
            ostringstream reason;
            reason << (*this) << "Expected token \"" << token << "\"." << endl;
            throw program_exception(reason.str());
        }
    }
    return *this;
}

//=============================================================================

simple_tokenizer &simple_tokenizer::gettok(string &buf, int(*lexerRule)(int), size_t maxChars) {
    skip_sp(*this);

    int ch;
    buf = "";
    while (maxChars-- > 0 && peek() != EOF && lexerRule(peek())) {
        get(ch);
        buf += (char)ch;
    }
    return *this;
}

//=============================================================================

simple_tokenizer &simple_tokenizer::skiplines(size_t numLines) {
    while (numLines-- > 0) {
        is->ignore(numeric_limits<streamsize>::max(), '\n');
        nrow++;
        ncol = 0;
    }
    return *this;
}

//=============================================================================

int is_not_endline(int ch) { return (ch != '\n'); }

simple_tokenizer &simple_tokenizer::getline(string &line) {
    skip_spaces_fn cur_skip_sp = skip_nothing;
    std::swap(cur_skip_sp, skip_sp);
    gettok(line, is_not_endline);
    int ch;
    if (peek() == '\n')
        get(ch);
    std::swap(cur_skip_sp, skip_sp);
    return *this;
}

//=============================================================================

string simple_tokenizer::pos() const {
    ostringstream oss;
    oss << (*this);
    return oss.str();
}

//=============================================================================

ostream &operator << (ostream &os, const simple_tokenizer &st) {
    return os << "line " << (st.nrow + 1) << "(" << (st.ncol + 1) << "): ";
}

//=============================================================================

int isnotspace(int ch) {
    return !isspace(ch);
}

//=============================================================================

void skip_nothing(simple_tokenizer &) { }

//=============================================================================

void skip_spaces(simple_tokenizer &st) {
    while (st.peek() != EOF && isspace(st.peek())) {
        int ch;
        st.get(ch);
    }
}

//=============================================================================

void skip_spaces_comments(simple_tokenizer &st) {
    while (true) {
        int ch = st.peek(), ch2;
        if (ch == '/') {
            st.get(ch);
            ch2 = st.peek();
            if (ch2 == '/') {
                // Skip characters until we reach a newline or EOF
                do { st.get(ch); }
                while (ch != EOF && ch != '\n');
            }
            else {
                st.unget();  // unget the first '/'
                return;
            }
        }
        else if (ch == EOF || !isspace(ch))
            return;
        else
            st.get(ch);
    }
}

//=============================================================================











//=============================================================================

void PrintUblasVector(ofstream &ofs, const ublas::vector<double> &vec) {
    ofs << '[' << vec.size() << "](";
    for (size_t m = 0; m < vec.size(); m++) {
        ofs << fmtdbl(vec[m], "%.10lf") << (m == vec.size() - 1 ? ")" : ",");
    }
    ofs << "\n" << flush;
    if (!ofs)
        throw program_exception("Couldn't write vector to file.");
}

//=============================================================================





// //=============================================================================

// #warning using experimental Lexer
// #include "newparser.lyy.h"

// #define TOKEN_POSTINCR -1
// #define TOKEN_POSTDECR -1
// #define TOKEN_IMPLY -1
// #define TOKEN_TWODOTS -1
// #define TOKEN_QUESTION -1
// #define TOKEN_FORALL -1
// #define TOKEN_EXISTS -1
// #define TOKEN_NEXT -1
// #define TOKEN_GLOBALLY -1
// #define TOKEN_FUTURE -1

// inline bool isStartId(int ch) { return isalpha(ch) || (ch == '_'); }
// inline bool isIdChar(int ch) { return isalnum(ch) || (ch == '_'); }

// // Keyword Identifiers
// // WARNING: This list must be sorted and in case sensitive order,
// //          because it is looked at with a binary search.
// static struct {
// 	const char *text;
// 	int token;
// } g_sortedTokenIds[] = {
// 	{ "Abs",       TOKEN_ABS_FN },
// 	{ "Acos",      TOKEN_ARCCOS_FN },
// 	// { "Act",       TOKEN_ACT },
// 	{ "Asin",      TOKEN_ARCSIN_FN },
// 	{ "Atan",      TOKEN_ARCTAN_FN },
// 	{ "Binomial",  TOKEN_BINOMIAL_FN },
// 	// { "Card",      TOKEN_MULTISET_CARD },
// 	{ "Ceil",      TOKEN_CEIL_FN },
// 	{ "Cos",       TOKEN_COS_FN },
// 	{ "Exp",       TOKEN_EXP_FN },
// 	{ "Factorial", TOKEN_FACTORIAL_FN },
// 	{ "False",     TOKEN_FALSE },
// 	{ "Floor",     TOKEN_FLOOR_FN },
// 	// { "Fract",     TOKEN_FRACT_FN },
// 	{ "If",        TOKEN_IF_FN },
// 	// { "Infinite",  TOKEN_INFINITY },
// 	{ "Log",       TOKEN_LOG_FN },
// 	{ "Max",       TOKEN_MAX_FN },
// 	{ "Min",       TOKEN_MIN_FN },
// 	{ "Mod",       TOKEN_MOD_FN },
// 	{ "PROB_TA",   TOKEN_PROB_TA },
// 	{ "Pow",       TOKEN_POW_FN },
// 	// { "Round",     TOKEN_ROUND_FN },
// 	{ "Sin",       TOKEN_SIN_FN },
// 	{ "Sqrt",      TOKEN_SQRT_FN },
// 	{ "Tan",       TOKEN_TAN_FN },
// 	{ "True",      TOKEN_TRUE },
// 	// { "clock",     TOKEN_CLOCK },
// 	// { "in",        TOKEN_CONTAINS },
// 	{ "ever",      TOKEN_EVER }, // remove ??
// 	{ "when",      TOKEN_WHEN }, // remove ??
// };

// const int LEX_FLAG_CTL = 1;

// static int Lex(istream& stream, string& buf, const int lexerFlags) {
// 	buf = "";
// 	// SKIP WHITESPACES
// 	while (stream && isspace(stream.peek()))
// 		stream.get();

// 	// End of stream
// 	if (!stream)
// 		return TOKEN_END;

// 	int c1 = stream.get();
// 	int c2 = stream.peek();
// 	assert(c1 != EOF);
// 	buf += (char)c1;

// 	// NUMBER CASE
// 	if (isdigit(c1) || (c1 == '-' && isdigit(c2))) {
// 		while (stream && isdigit(stream.peek()))
// 			buf += (char)stream.get();

// 		// Real number
// 		if ('.' == stream.peek()) {
// 			buf += (char)stream.get();

// 			while (stream && isdigit(stream.peek()))
// 				buf += (char)stream.get();

// 			if (tolower(stream.peek()) == 'e') {
// 				// Exponent part
// 				buf += (char)stream.get();
// 				if (stream.peek() == '+' || stream.peek() == '-') {
// 					buf += (char)stream.get();

// 					while (stream && isdigit(stream.peek()))
// 						buf += (char)stream.get();
// 				}
// 			}
// 			return TOKEN_REALNUM;
// 		}
// 		return TOKEN_INTEGER;
// 	}
// 	// IDENTIFIER CASE
// 	else if (isStartId(c1)) {
// 		// Special case for P{  E{  X{  tokens
// 		if (c2 == '{') {
// 			switch (c1) {
// 				case 'P':  {  buf += (char)stream.get();  return TOKEN_OPEN_P;  }
// 				case 'E':  {  buf += (char)stream.get();  return TOKEN_OPEN_E;  }
// 				case 'X':  {  buf += (char)stream.get();  return TOKEN_OPEN_X;  }
// 			}
// 		}

// 		// Read the identifier
// 		while (stream && isIdChar(stream.peek()))
// 			buf += (char)stream.get();

// 		if (LEX_FLAG_CTL & lexerFlags) {
// 			if (buf.length() == 1) {
// 				switch (c1) {
// 					case 'A':	return TOKEN_FORALL;
// 					case 'E':	return TOKEN_EXISTS;
// 					case 'X':	return TOKEN_NEXT;
// 					case 'G':	return TOKEN_GLOBALLY;
// 					case 'F':	return TOKEN_FUTURE;
// 				}
// 			}
// 		}

// 		// Distinguish keywords
// 		const int max = sizeof(g_sortedTokenIds) / sizeof(g_sortedTokenIds[0]);
// 		int start = 0, end = max - 1;
// 		while (start < end) {
// 			int mid = (start + end) / 2;
// 			int cmp = strcmp(buf.c_str(), g_sortedTokenIds[mid].text);
// 			if (cmp == 0)
// 				return g_sortedTokenIds[mid].token;
// 			else if (cmp < 0)
// 				end = mid - 1;
// 			else  // cmp > 0
// 				start = mid + 1;
// 		}

// 		return TOKEN_ID;
// 	}
// 	// OPERATOR CASE
// 	else if (ispunct(c1)) {
// 		switch (c1) {
// 			case '<':
// 				if (c2 == '=') {  buf += (char)stream.get();  return TOKEN_LESS_EQ;  }
// 				// if (c2 == '>') {  buf += (char)stream.get();  return TOKEN_OPERATOR;  }
// 				return TOKEN_LESS;

// 			case '>':
// 				if (c2 == '=') {  buf += (char)stream.get();  return TOKEN_GREATER_EQ;  }
// 				return TOKEN_GREATER;

// 			case '=':
// 				if (c2 == '=') {  buf += (char)stream.get();  return TOKEN_EQ;  }
// 				return TOKEN_EQ;

// 			case '+':
// 				if (c2 == '+') {  buf += (char)stream.get();  return TOKEN_POSTINCR;  }
// 				return TOKEN_PLUS;

// 			case '-':
// 				if (c2 == '-') {  buf += (char)stream.get();  return TOKEN_POSTDECR;  }
// 				if (c2 == '>') {  buf += (char)stream.get();  return TOKEN_IMPLY;  }
// 				return TOKEN_MINUS;

// 			case '!':
// 				// if (c2 == '=') {  buf += (char)stream.get();  return TOKEN_NOT_EQ;  }
// 				return TOKEN_NOT;

// 			case '/':
// 				if (c2 == '=') {  buf += (char)stream.get();  return TOKEN_NOT_EQ;  }
// 				return TOKEN_DIV;

// 			case '.':
// 				if (c2 == '.') {  buf += (char)stream.get();  return TOKEN_TWODOTS;  }
// 				return TOKEN_INVALID;

// 			case '&':
// 				if (c2 == '&') {  buf += (char)stream.get();  return TOKEN_AND;  }
// 				return TOKEN_INVALID;

// 			case '|':
// 				if (c2 == '|') {  buf += (char)stream.get();  return TOKEN_OR;  }
// 				return TOKEN_INVALID;

// 			case '*':	return TOKEN_TIMES;
// 			case '~':	return TOKEN_NOT;
// 			case '#':	return TOKEN_SHARP;
// 			case '?':	return TOKEN_QUESTION;
// 			case '(':	return TOKEN_OP_PAREN;
// 			case ')':	return TOKEN_CL_PAREN;
// 			case '[':	return TOKEN_OP_SQPAR;
// 			case ']':	return TOKEN_CL_SQPAR;
// 			case '{':	return TOKEN_OP_BRACK;
// 			case '}':	return TOKEN_CL_BRACK;
// 			case ':':	return TOKEN_COLON;
// 			case ';':	return TOKEN_SEMICOLON;
// 			case ',':	return TOKEN_COMMA;
// 		}
// 	}

// 	return TOKEN_INVALID;
// }

// //=============================================================================






















