/*
 *  common-deff.h
 *  CslTA-Solver
 *
 *  Common Definitions
 *
 */

//-----------------------------------------------------------------------------
#ifndef __COMMON_DEFS_H__
#define __COMMON_DEFS_H__
//-----------------------------------------------------------------------------

#ifdef __cplusplus
#  define PUBLIC_API  extern "C"
#else
#  define PUBLIC_API
#endif

typedef int BOOL;
#define TRUE 1
#define FALSE 0

//-----------------------------------------------------------------------------
// Forward Declarations:
//-----------------------------------------------------------------------------
struct MCObject;
typedef struct MCObject  MCObject;
struct Constant;
typedef struct Constant  Constant;
struct IDList;
typedef struct IDList  IDList;
struct IDValList;
typedef struct IDValList  IDValList;

struct AsmcState;
typedef struct AsmcState  AsmcState;
struct AsmcTransition;
typedef struct AsmcTransition AsmcTransition;
struct ASMC;
typedef struct ASMC  ASMC;

struct DtaSubstList;
typedef struct DtaSubstList  DtaSubstList;
struct DtaLocLabel;
typedef struct DtaLocLabel  DtaLocLabel;
struct DtaLoc;
typedef struct DtaLoc  DtaLoc;
struct NumberOrID;
typedef struct NumberOrID  NumberOrID;
struct DtaGuard;
typedef struct DtaGuard  DtaGuard;
struct DtaActSet;
typedef struct DtaActSet  DtaActSet;
struct DtaEdge;
typedef struct DtaEdge  DtaEdge;
struct DtaCCNamOrd;
typedef struct DtaCCNamOrd  DtaCCNamOrd;
struct DtaStatePropValues;
typedef struct DtaStatePropValues  DtaStatePropValues;
struct DTA;
typedef struct DTA  DTA;

struct APValExpr;
typedef struct APValExpr  APValExpr;
struct CslTaExpr;
typedef struct CslTaExpr  CslTaExpr;
struct CslTaResult;
typedef struct CslTaResult  CslTaResult;
struct DTAParams;
typedef struct DTAParams  DTAParams;

struct DSPN;
typedef struct DSPN  DSPN;

//-----------------------------------------------------------------------------

// Model Checker Object Type
enum MCObjectType {
    MCOT_ASMC,
    MCOT_DTA,
    MCOT_RESULT,
    MCOT_CONSTANT,
    MCOT_ALL_TYPES
};
typedef enum MCObjectType  MCObjectType;
extern const char *MCOT_Names[];

// Binary logic operators
enum BinaryLogicOp {
    BOP_AND, BOP_OR, BOP_IMPLY
};
typedef enum BinaryLogicOp  BinaryLogicOp;
extern const char *BinaryLogicOp_Str[];
extern const char *UnaryLogicOp_Not;

// External Solver Used
enum ExternalDspnSolver {
    EDS_none,
    EDS_SPNica,
    EDS_DSPNexpress,
    EDS_MRPSolver,
    EDS_TestSolver
};
typedef enum ExternalDspnSolver  ExternalDspnSolver;


//-----------------------------------------------------------------------------
// Common Parser API
//-----------------------------------------------------------------------------
PUBLIC_API void    IDList_Delete(IDList *idl);
PUBLIC_API IDList *IDList_New();
PUBLIC_API IDList *IDList_AddID(IDList *idl, const char *newID);
PUBLIC_API IDList *IDList_AddUniqueID(IDList *idl, const char *newID);

PUBLIC_API void       IDValList_Delete(IDValList *idvl);
PUBLIC_API IDValList *IDValList_New();
PUBLIC_API IDValList *IDValList_AddIDVal(IDValList *idvl, const char *newID,
        const char *val);
PUBLIC_API IDValList *IDValList_AddUniqueIDVal(IDValList *idvl, const char *newID,
        const char *val);


//-----------------------------------------------------------------------------
// Command execution API and Global Status API:
//-----------------------------------------------------------------------------
PUBLIC_API void Exec_StoreMCObject(MCObject *obj, const char *name);
PUBLIC_API void Exec_ClearMCObject(const char *name);
PUBLIC_API void Exec_RenameMCObject(const char *oldname, const char *newname);
PUBLIC_API void Exec_SaveMCObjectAsGml(const char *name, const char *filename,
                                       const char *args);
PUBLIC_API void Exec_LoadDspnCoords(const char *result, const char *filename,
                                    const char *args);
PUBLIC_API void Exec_PlanarizeDspn(const char *result, const char *args);
PUBLIC_API void Exec_SaveDspn(const char *result, const char *filenamePrefix,
                              const char *args, const char *formatName);
PUBLIC_API void Exec_SaveTRG(const char *result, const char *filenamePrefix);
PUBLIC_API void Exec_EvaluateCslTaExpr(const char *result,
                                       const char *asmcName,
                                       const char *evalState,
                                       CslTaExpr *expr);
PUBLIC_API void Exec_SetOutputDir(const char *dirName);

PUBLIC_API MCObject *GS_RetrieveMCObjectByName(const char *name,
        MCObjectType mcot);
PUBLIC_API void GS_StoreTempDtaParams(DtaCCNamOrd *CCNamOrds, IDList *Acts, IDList *SPs);

PUBLIC_API const char *GS_GetLastError();
PUBLIC_API void		   GS_SetError(const char *e);


PUBLIC_API char *Number_Op(const char *n1, char op, const char *n2);

PUBLIC_API Constant *Constant_New(const char *v);
PUBLIC_API int       Constant_IsDefined(const char *name);
PUBLIC_API char     *Constant_GetOrErr(const char *name);



//-----------------------------------------------------------------------------
#ifdef __cplusplus


void GS_SetError(const string &e);


//-----------------------------------------------------------------------------
// C++ helper functions
//-----------------------------------------------------------------------------

template <class Cont> void safe_delete_ptr_vector(Cont &v) {
    for (typename Cont::iterator it = v.begin(); it != v.end(); ++it)
        if (*it != NULL)
            delete *it;
    v.clear();
}
template <class Cont> void safe_delete_ptr_mapvals(Cont &v) {
    for (typename Cont::iterator it = v.begin(); it != v.end(); ++it)
        if (it->second != NULL)
            delete it->second;
    v.clear();
}
template <class T>  void safe_delete(T &p) {
    if (p != NULL) {
        delete p;
        p = NULL;
    }
}
template <class T>
bool is_value_between(const T &tA, const T &value, const T &tB) {
    return (tA < value && value < tB);
}
template <class T> T square(T val) { return val * val; }

template <class C> string print_vector(const C &c, const char *sep = ", ") {
    ostringstream s;
    typename C::const_iterator it = c.begin();
    size_t cnt = 0;
    while (it != c.end()) {
        if (cnt++ != 0) s << sep;
        s << *it;
        it++;
    }
    return s.str();
}

template <class C1, class C2>
string print_vectors(const C1 &c1, const C2 &c2,
                     const char *sep = ", ", const char *arrow = "->") {
    ostringstream s;
    typename C1::const_iterator it1 = c1.begin();
    typename C2::const_iterator it2 = c2.begin();

    size_t i = 0;
    while (it1 != c1.end() || it2 != c2.end()) {
        if (i++ != 0)
            s << sep;
        if (it1 != c1.end())
            s << "\"" << *it1++ << "\"";
        else s << "\"\"";
        s << arrow;
        if (it2 != c2.end())
            s << "\"" << *it2++ << "\"";
        else s << "\"\"";
    }
    return s.str();
}


//-----------------------------------------------------------------------------
// Base class of the declarable model check objects
//-----------------------------------------------------------------------------
struct MCObject {
    MCObject() {}
    virtual ~MCObject() {}

    virtual MCObjectType GetObjectType() const = 0;
    virtual bool isValidObject(string &reason) const = 0;
    virtual bool WriteAsGmlFile(ostream &os, const char *name,
                                const char *args) = 0;
};


//-----------------------------------------------------------------------------
// Common Helper structures and functions
//-----------------------------------------------------------------------------

// list of unique identifiers
struct IDList {
    list<string> idList;
};

struct IdValuePair {
    string ID;
    size_t val;
    inline IdValuePair(const string &i, size_t v) : ID(i), val(v) { }
};
typedef struct IdValuePair  IdValuePair;

// list of unique identifiers paired with unsigned values
struct IDValList {
    list<IdValuePair> idValList;
};

string htmlize_utf8_str(const string &s);


// Point in 2D space (with double coords)
struct dPoint {
    dPoint() : X(0), Y(0) {}
    dPoint(double _x, double _y) : X(_x), Y(_y) {}
    dPoint(const dPoint &p) : X(p.X), Y(p.Y) {}
    void set(double _x, double _y) { X = _x; Y = _y; }
    void min(const dPoint &p) { X = std::min(X, p.X); Y = std::min(Y, p.Y); }
    void max(const dPoint &p) { X = std::max(X, p.X); Y = std::max(Y, p.Y); }
    void translateRescale(const dPoint &tr, double rs)
    { X += tr.X; Y += tr.Y; X *= rs; Y *= rs; }
    double distSq(const dPoint &p) { return square(X - p.X) + square(Y - p.Y); }
    double dist(const dPoint &p) { return sqrt(distSq(p)); }

    double X;
    double Y;
};
typedef struct dPoint  dPoint;
typedef std::vector<dPoint>  dPolyLine;


// Constant value (as string)
struct Constant : public MCObject {
    Constant(const char *v) : value(v) {}
    virtual ~Constant() {}

    virtual MCObjectType GetObjectType() const	{ return MCOT_CONSTANT; }
    virtual bool isValidObject(string &reason) const	{ return true; }
    virtual bool WriteAsGmlFile(ostream &os, const char *name,
                                const char *args)		{ return false; }

    string value;
};

//-----------------------------------------------------------------------------
// SymbolTable - container that maps strings to sequential IDs
//-----------------------------------------------------------------------------
typedef unsigned int SymbolID;
typedef SymbolID ActionID;
typedef SymbolID AtomicPropID;
typedef SymbolID StatePropID;
typedef SymbolID ClockValID;

class SymbolTable {
public:
    bool HasSymbol(const string &name) const;
    SymbolID GetIDFromName(const string &name) const;
    SymbolID AddNewSymbol(const string &name);
    const string &GetSymbolName(SymbolID symId) const;
    void RemapSymbolIDs(const vector<size_t> &RemapTable);

    inline const size_t NumSymbols() const
    {   return symbols.size();   }
    inline bool IsValidID(SymbolID symId) const
    {   return /*symId>=0 &&*/ symId < symbols.size();   }

    void operator = (const SymbolTable &st);

    friend ostream &operator << (ostream &os, const SymbolTable &sm);

private:
    vector<string>  symbols;	// mapped symbols
};
ostream &operator << (ostream &os, const SymbolTable &sm);



//-----------------------------------------------------------------------------
// Pipe between 2 applications
//-----------------------------------------------------------------------------
#ifdef WIN32
struct WINPOPEN_T;
#endif
class Pipe : public noncopyable {
public:
    Pipe();
    ~Pipe();

    bool Open(const char *cmd, const char *_exitStmt,
              FILE *_outRd, FILE *_outWr);
    void Close();
    operator bool () const;

    bool Write(const char *);
    bool Write(const string &str)  {  return Write(str.c_str());  }

    bool ReadLine(char *line, size_t max, bool skipEmpty);
    bool ReadDouble(double *pVal);
    bool SkipUntil(const char *endLine);

protected:
#ifdef WIN32
    WINPOPEN_T  *wpt;		// Win32 pipe objects
#else
    FILE		*pfile;		// pipe between the 2 applications
#endif
    const char  *exitStmt;	// exit command sent before closing the pipe
    FILE		*outRd, *outWr; // read and write log of the communication
};



//-----------------------------------------------------------------------------
// Global Tool State and Configuration
//-----------------------------------------------------------------------------
struct GlobalStatus {
    GlobalStatus();
    ~GlobalStatus();

    // Global namespace of MCObjects (ASMCs, DTAs, RESULTs, CONSTs, ...)
    map<string, MCObject *>  mcobjects; // Objects already defined

    // Temporary IDList for DTA declarations
    DtaCCNamOrd *dtaCCNamOrds;
    IDList *dtaActs, *dtaSPExprs;;

    // Last error (no error if empty)
    string lastError;

    // Output directory name (changed with SET_OUTPUT_DIR command)
    string outputDir;
    string GetOutputFileName(const char *name) const;

    // ---- Configuration ----
    ExternalDspnSolver extSolverUsed;

    // Mathematica Kernel (when using SPNica solver)
    string mathKernelName;
    string mathRdLogFile, mathWrLogFile;
    bool writeMathKernelOutput;
    Pipe &GetMathKernel();

    // DSPNexpress configuration
    string dspnExpressPath;
    string readUrgPath;

    // MRP-Solver configuration
    string mrpPath;
    string mrpLogFile;
    string extraArgs;

    // Test Solver confiuration
    string testSolverDspnName;

private:
    Pipe   mathKernel;
};
extern GlobalStatus  g_status;

//-----------------------------------------------------------------------------
//  Other Platform-dependent code
//-----------------------------------------------------------------------------

#ifdef WIN32
# define  NO_CMD_OUTPUT   " > NUL 2> NUL"
# define  TEMPNAM_DIR     "C:\\Temp"
#else
# define  NO_CMD_OUTPUT   " > /dev/null 2> /dev/null "
# define  TEMPNAM_DIR     NULL
#endif


//-----------------------------------------------------------------------------
#endif  // __cplusplus
#endif  // __COMMON_DEFS_H__

















