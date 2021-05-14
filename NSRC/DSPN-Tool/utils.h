/*
 *  utils.h
 *
 *  Various utilities used in the tool
 *
 *  Created by Elvio Amparore.
 *
 */

//=============================================================================
#ifndef __MY_UTILS_H__
#define __MY_UTILS_H__
//=============================================================================

#ifdef WIN32
#define DIR_SEP     '\\'
#define DIR_SEP_S   "\\"
#define PATH_SEP    ';'
#define PATH_SEP_S  ";"
#else
#define DIR_SEP     '/'
#define DIR_SEP_S   "/"
#define PATH_SEP    ':'
#define PATH_SEP_S  ":"
#endif

//=============================================================================

void verify_thr(const char *expr, const char *file, const size_t line);

void verify(bool x); // make compiler happy

#define verify(x)   if (!(x)) verify_thr(#x, __FILE__, __LINE__)

//=============================================================================

template<class T>
inline void max_assign(T &a, const T &b)  {  a = max(a, b);  }

template<class T>
inline void min_assign(T &a, const T &b)  {  a = min(a, b);  }

template<class Iter, class Val>
inline bool exists(const Iter &begin, const Iter &end, const Val &v) {
    return (end != find(begin, end, v));
}

//=============================================================================

struct identity_functor {
    template<class T>
    inline const T &operator()(const T &v) const   {  return v;  }
};

struct plus_one_functor {
    template<class T>
    inline const T operator()(const T &v) const   {  return v + T(1);  }
};

// get a name out of a vector at index i. Vector nv must have a "name" field.
template<class NV> struct index_of_name_functor {
    const NV &nv;
    index_of_name_functor(const NV &_nv) : nv(_nv) {}
    template<class T>
    inline const std::string &operator()(const T &i) const
    {  return nv.at(size_t(i)).name;  }
};

//=============================================================================
// Print styles for vector:

struct vec_out_style_t {
    const char *openPar, *emptySpc, *sep, *closePar;
};

extern const vec_out_style_t s_pvfmtBracketsSpaced;	//  { 1, 2, 3 }
extern const vec_out_style_t s_pvfmtBrackets;		//  {1,2,3}

extern const vec_out_style_t s_pvfmtSquaredSpaced;	//  [ 1, 2, 3 ]
extern const vec_out_style_t s_pvfmtSquared;		//  [1,2,3]

extern const vec_out_style_t s_pvfmtPathSep;		//  1:2:3  or 1;2;3 (Win32)

//=============================================================================
//   Print functions
//=============================================================================

template<class VecIter, class Func>
std::function<ostream& (ostream &)>
print_vec(VecIter first, VecIter last, Func func,
          const vec_out_style_t &fmt = s_pvfmtBracketsSpaced) {
    return [ = ](ostream & os) -> ostream& {
        auto it = first;
        os << fmt.openPar;
        for (size_t n = 0; it != last; ++it, ++n) {
            os << (n == 0 ? fmt.emptySpc : fmt.sep);
            os << func(*it);
        }
        return os << fmt.emptySpc << fmt.closePar;
    };
}

template<class Vec, class Func>
std::function<ostream& (ostream &)>
print_vec(const Vec &v, Func func = identity_functor(),
          const vec_out_style_t &fmt = s_pvfmtBracketsSpaced) {
    return print_vec(v.begin(), v.end(), func, fmt);
}

template<class Vec>
std::function<ostream& (ostream &)>
print_vec(const Vec &v, const vec_out_style_t &fmt = s_pvfmtBracketsSpaced) {
    identity_functor func;
    return print_vec(v, func, fmt);
}

template<class VecIter>
std::function<ostream& (ostream &)>
print_vec_1based(VecIter first, VecIter last, const vec_out_style_t &fmt = s_pvfmtBracketsSpaced) {
    plus_one_functor func;
    return print_vec(first, last, func);
}

template<class Vec>
std::function<ostream& (ostream &)>
print_vec_1based(const Vec &v, const vec_out_style_t &fmt = s_pvfmtBracketsSpaced) {
    return print_vec_1based(v.begin(), v.end(), fmt);
}

template<class IndexIter, class NamesVec>
std::function<ostream& (ostream &)>
print_indexed_names(IndexIter first, IndexIter last, const NamesVec &namesVec,
                    const vec_out_style_t &fmt = s_pvfmtBracketsSpaced) {
    index_of_name_functor<NamesVec> func(namesVec);
    return print_vec(first, last, func, fmt);
}

template<class IndexVec, class NamesVec>
std::function<ostream& (ostream &)>
print_indexed_names(const IndexVec &indVec, const NamesVec &namesVec,
                    const vec_out_style_t &fmt = s_pvfmtBracketsSpaced) {
    return print_indexed_names(indVec.begin(), indVec.end(), namesVec, fmt);
}

//=============================================================================

// Print a ublas vector
std::function<ostream& (ostream &)>
print_vec_ublas(const ublas::vector<double> &v);

// Print a double value with a printf-like format
std::function<ostream& (ostream &)>
fmtdbl(double v, const char *fmt);

//=============================================================================
// General-purpose function-object printer
//=============================================================================

inline ostream &
operator<<(ostream &stream, const std::function<ostream& (ostream &)> &fnObj) {
    return fnObj(stream);
}

//=============================================================================
// Simple tokenizer class with row/column count and support for comment skip.
//=============================================================================

struct simple_tokenizer;
typedef void (*skip_spaces_fn)(simple_tokenizer &);

void skip_nothing(simple_tokenizer &);
void skip_spaces(simple_tokenizer &);
void skip_spaces_comments(simple_tokenizer &);
int isnotspace(int ch);


struct simple_tokenizer {
    inline simple_tokenizer(istream *_is, const skip_spaces_fn _sfn = skip_spaces)
        : is(_is), skip_sp(_sfn) { }

    // Read an exact token, throw an exception if the token is different
    simple_tokenizer &readtok(const char *token);

    // Extract a token (and return it) using a simple char-based rule
    simple_tokenizer &gettok(string &buf, int(*lexerRule)(int) = isnotspace,
                             size_t maxChars = std::numeric_limits<size_t>::max());

    // Read a simple token (int, double, string, ...) from the current row
    template<typename T>
    inline simple_tokenizer &read(T &obj) {
        (*is) >> obj;
        ncol += is->gcount();
        return *this;
    }

    // skip n lines
    simple_tokenizer &skiplines(size_t numLines = 1);

    // read a single line (without the '\n') and store it into line,
    // drop the '\n' from the stream - no space skipping is done.
    simple_tokenizer &getline(string &line);

    inline bool get(int &c)  {
        c = is->get();
        if (c == '\n') { nrow++; ncol = 0; }
        else { ncol++; }
        return is->good();
    }

    inline int peek() { return is->peek(); }

    inline bool unget() { ncol--; is->unget(); return is->good(); }

    // Format a string with the current row/column position
    string pos() const;

    inline operator bool() { return (is->good()); }

private:
    istream *is;				/// The input stream read by the tokenizer
    size_t nrow = 0, ncol = 0;	/// Current position in the stream
    skip_spaces_fn skip_sp;		/// Function used to skip spaces between tokens

    friend ostream &operator << (ostream &os, const simple_tokenizer &st);
};

// Print the position of the tokenizer in the stream
ostream &operator << (ostream &os, const simple_tokenizer &st);

//=============================================================================



//=============================================================================
// Console functionalities
//=============================================================================
namespace console {

#ifdef WIN32
// Windows Console API

HANDLE get_stdout_handle();
WORD get_default_stdout_attr();

#endif

enum escape_command {
    // Attributes
    EC_DEFAULT_DISP,

    EC_BOLD_ON,       EC_BOLD_OFF,
    EC_DIM_ON,        EC_DIM_OFF,
    EC_UNDERLINE_ON,  EC_UNDERLINE_OFF,
    EC_BLINK_ON,      EC_BLINK_OFF,
    EC_REVERSE_ON,    EC_REVERSE_OFF,
    EC_HIDDEN_ON,     EC_HIDDEN_OFF,

    // Color Codes
    EC_COLOR_FGND_OFF,

    EC_BLACK_FGND,    EC_BLACK_BGND,
    EC_RED_FGND,      EC_RED_BGND,
    EC_GREEN_FGND,    EC_GREEN_BGND,
    EC_YELLOW_FGND,   EC_YELLOW_BGND,
    EC_BLUE_FGND,     EC_BLUE_BGND,
    EC_MAGENTA_FGND,  EC_MAGENTA_BGND,
    EC_CYAN_FGND,     EC_CYAN_BGND,
    EC_WHITE_FGND,    EC_WHITE_BGND,

    // Cursor Position
    EC_CUR_XY,
    EC_CUR_UP,
    EC_CUR_DOWN,
    EC_CUR_RIGHT,
    EC_CUR_LEFT,

    // Complex combined commands
    EC_BEG_ERROR,
    EC_END_ERROR,
    EC_BEG_TITLE,
    EC_END_TITLE,
    EC_BEG_SUCCESS,
    EC_END_SUCCESS,
    EC_BEG_EMPH,
    EC_END_EMPH,
    EC_ONE_LINE_UP,
};

struct escape_seq {
    inline escape_seq(escape_command _cmd, int _dx = 0, int _dy = 0)
        : cmd(_cmd), dx(_dx), dy(_dy) { }

    escape_command cmd; // Console command (escape sequence)
    int dx, dy;         // x,y positions or line count
};

// Attributes
inline escape_seq default_disp()   {  return escape_seq(EC_DEFAULT_DISP);  }
inline escape_seq bold_on()	       {  return escape_seq(EC_BOLD_ON);  }
inline escape_seq dim_on()		   {  return escape_seq(EC_DIM_ON);  }
inline escape_seq underline_on()   {  return escape_seq(EC_UNDERLINE_ON);  }
inline escape_seq blink_on()       {  return escape_seq(EC_BLINK_ON);  }
inline escape_seq reverse_on()     {  return escape_seq(EC_REVERSE_ON);  }
inline escape_seq hidden_on()      {  return escape_seq(EC_HIDDEN_ON);  }

inline escape_seq bold_off()	   {  return escape_seq(EC_BOLD_OFF);  }
inline escape_seq dim_off()	       {  return escape_seq(EC_DIM_OFF);  }
inline escape_seq underline_off()  {  return escape_seq(EC_UNDERLINE_OFF);  }
inline escape_seq blink_off()      {  return escape_seq(EC_BLINK_OFF);  }
inline escape_seq reverse_off()    {  return escape_seq(EC_REVERSE_OFF);  }
inline escape_seq hidden_off()     {  return escape_seq(EC_HIDDEN_OFF);  }

// Color Codes
inline escape_seq color_fgnd_off() {  return escape_seq(EC_COLOR_FGND_OFF);  }

inline escape_seq black_fgnd()     {  return escape_seq(EC_BLACK_FGND);  }
inline escape_seq red_fgnd()       {  return escape_seq(EC_RED_FGND);  }
inline escape_seq green_fgnd()     {  return escape_seq(EC_GREEN_FGND);  }
inline escape_seq yellow_fgnd()    {  return escape_seq(EC_YELLOW_FGND);  }
inline escape_seq blue_fgnd()      {  return escape_seq(EC_BLUE_FGND);  }
inline escape_seq magenta_fgnd()   {  return escape_seq(EC_MAGENTA_FGND);  }
inline escape_seq cyan_fgnd()      {  return escape_seq(EC_CYAN_FGND);  }
inline escape_seq white_fgnd()     {  return escape_seq(EC_WHITE_FGND);  }

inline escape_seq black_bgnd()     {  return escape_seq(EC_BLACK_BGND);  }
inline escape_seq red_bgnd()       {  return escape_seq(EC_RED_BGND);  }
inline escape_seq green_bgnd()     {  return escape_seq(EC_GREEN_BGND);  }
inline escape_seq yellow_bgnd()    {  return escape_seq(EC_YELLOW_BGND);  }
inline escape_seq blue_bgnd()      {  return escape_seq(EC_BLUE_BGND);  }
inline escape_seq magenta_bgnd()   {  return escape_seq(EC_MAGENTA_BGND);  }
inline escape_seq cyan_bgnd()      {  return escape_seq(EC_CYAN_BGND);  }
inline escape_seq white_bgnd()     {  return escape_seq(EC_WHITE_BGND);  }

// Cursor Position
inline escape_seq cur_xy(int x, int y)  {  return escape_seq(EC_CUR_XY, x, y);  }
inline escape_seq cur_up(int lines)     {  return escape_seq(EC_CUR_UP, 0, lines);  }
inline escape_seq cur_down(int lines)   {  return escape_seq(EC_CUR_DOWN, 0, lines);  }
inline escape_seq cur_right(int lines)  {  return escape_seq(EC_CUR_RIGHT, lines, 0);  }
inline escape_seq cur_left(int lines)   {  return escape_seq(EC_CUR_LEFT, lines, 0);  }

// Complex commands
inline escape_seq beg_error()      {  return escape_seq(EC_BEG_ERROR);  }
inline escape_seq end_error()      {  return escape_seq(EC_END_ERROR);  }
inline escape_seq beg_title()      {  return escape_seq(EC_BEG_TITLE);  }
inline escape_seq end_title()      {  return escape_seq(EC_END_TITLE);  }
inline escape_seq beg_success()    {  return escape_seq(EC_BEG_SUCCESS);  }
inline escape_seq end_success()    {  return escape_seq(EC_END_SUCCESS);  }
inline escape_seq beg_emph()       {  return escape_seq(EC_BEG_EMPH);  }
inline escape_seq end_emph()       {  return escape_seq(EC_END_EMPH);  }
inline escape_seq one_line_up()    {  return escape_seq(EC_ONE_LINE_UP);  }

}; // namespace console

// Print an escape_seq object
std::ostream &operator << (std::ostream &os, const console::escape_seq &esc);


//=============================================================================

// Prints to a string a text representation of a timestamp
string getTimeStamp(double numSec, bool printMilliSec = false, bool printHourMinutes = false);

//=============================================================================

// Timer class that is used to print messages with a limited frequency
class long_interval_timer : boost::noncopyable {
    clock_t		startClock;			///< Start clock time
    clock_t		lastPrintTime;		///< Last time a message has been printed
    int			numPrints;			///< Number of printed messages
    static clock_t frequency;       ///< Printing frequency

    static clock_t get_frequency();
public:

    long_interval_timer();

    /// Resets the timer
    void reset();

    /// Tells if a message has to be printed or not.
    operator bool();

    /// Elapsed time, in seconds
    double get_elapsed_time();

    /// Print a time estimate before completing the current task
    void print_estimate(size_t numDone, size_t total);

    /// Prepares a message print
    void prepare_print();

    /// Call this function to clear any previous message outputs
    void clear_any();
};

//=============================================================================

struct rusage;

/// Performance timer: used to track the exact CPU time used by the
/// application, classified in elapsed user and system time.
class performance_timer : boost::noncopyable {
#ifdef WIN32
    FILETIME		UserTimeStart, KernelTimeStart;

    void start();
    double filetimediff(const FILETIME &ft1, const FILETIME &ft2);
    void stop();
#else
    std::shared_ptr<struct rusage>  pStartRUsage;

    void start();
    double timevaldiff(const struct timeval &tv1, const struct timeval &tv2);
    void stop();
#endif
    /// Common fields: user and system time elapsed
    double			userSecs;
    double			systemSecs;

public:
    /// Construct a new timer and start counting
    performance_timer();

    /// Get a representation of the user/system time elapsed from the construction
    string get_timer_repr();
};

ostream &operator <<(ostream &os, performance_timer &pt);

//=============================================================================

// Message printer for linear solver methods
class LinearIterationPrinter : public numerical::NullIterPrintOut {
public:
    LinearIterationPrinter(VerboseLevel vl, long_interval_timer *p = nullptr);

    // Print a waiting message between iterations
    virtual void onIteration(AlgoName, size_t iter, double resid) override;
    virtual void onConvergence(AlgoName, size_t iter, double resid) override;
    virtual void onFailure(AlgoName, size_t iter, ConvergenceFailureReason cfr) override;
    virtual void onUnif(double t, double lambda, size_t M, size_t R, size_t vecMatProdCount) override;
    virtual void onUnifGen(const char* fg, double lambda, size_t R, size_t vecMatProdCount) override;
    virtual void onMessage(bool isLastMsg, const char *fmt, ...);
    virtual VerboseLevel getVerboseLevel() const override;

private:
    VerboseLevel		  verboseLvl; // Print/silent mode
    long_interval_timer  *pTimer;	  // Message timer
    long_interval_timer   internalTimer;
};


//=============================================================================

// Taken from boost, since it is missing in the C++11 std
template<class T, class U> std::shared_ptr<T>
shared_polymorphic_downcast(std::shared_ptr<U> const &r) {
    assert(dynamic_cast<T *>(r.get()) == r.get());
    return std::static_pointer_cast<T>(r);
}

//=============================================================================

void PrintAsMathematicaMatrix(const ublas::compressed_matrix<double> &M,
                              const char *name);

void PrintUblasVector(ofstream &ofs, const ublas::vector<double> &vec);

//=============================================================================
#endif   // __MY_UTILS_H__














