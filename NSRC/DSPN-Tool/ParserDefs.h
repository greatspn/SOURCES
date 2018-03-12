/*
 *  ParserDefs.h
 *
 *  Helper definitions for the parser/lexer interface
 *
 *  Created by Elvio Amparore
 *
 */
//=============================================================================
#ifndef __PARSER_DEFS_H__
#define __PARSER_DEFS_H__
//=============================================================================

struct PN;
struct DTA;
class ParserObj;

#ifndef UNUSED
# define UNUSED(x) ((void)(x))
#endif

//=============================================================================
//   Lexer helper class or managing input streams from std::iostream objects
//=============================================================================

class STRINGSTREAM {
    iostream   *pStream;
    bool		isInteractive;

public:
    inline STRINGSTREAM(iostream *pIOs, bool isInt)
    /**/ : pStream(pIOs), isInteractive(isInt) { UNUSED(isInteractive); }

    size_t stream_fread(void *ptr, size_t size, size_t nmemb);
    size_t stream_fwrite(const void *ptr, size_t size, size_t nmemb);
    void   stream_clearerr();
    int    stream_ferror();
    int    stream_getc();
    int    stream_fileno();

    static int stream_isatty(int filenumber);
};

//=============================================================================
//   Opaque data type of the scanner
//=============================================================================

struct ExtraLexerArgs {
    inline ExtraLexerArgs(const PN *pn, const DTA *dta, int initTok) 
    : pPN(pn), pDTA(dta), initialToken(initTok) { }

    const PN		*pPN;	// PetriNet used by the parser (for name lookups)
    const DTA		*pDTA;	// DTA object (for atomic proosition lookups)
    int              initialToken; // what is being parsed;
};

typedef void *yyscan_t;
#define YY_EXTRA_TYPE   const ExtraLexerArgs*
struct Token;

//=============================================================================
//   Reentrant lexer API
//=============================================================================

int 		  yylex_init(yyscan_t *scanner);
int 		  yylex_destroy(yyscan_t yyscanner);

char		 *yyget_text(yyscan_t scanner);
//int 		  yyget_leng(yyscan_t scanner);
STRINGSTREAM *yyget_in(yyscan_t scanner);
STRINGSTREAM *yyget_out(yyscan_t scanner);
int 		  yyget_lineno(yyscan_t scanner);
YY_EXTRA_TYPE yyget_extra(yyscan_t scanner);
int  		  yyget_debug(yyscan_t scanner);

void 		  yyset_debug(int flag, yyscan_t scanner);
void 		  yyset_in(STRINGSTREAM *in_str , yyscan_t scanner);
void 		  yyset_out(STRINGSTREAM *out_str , yyscan_t scanner);
void 		  yyset_lineno(int line_number , yyscan_t scanner);
void 		  yyset_extra(YY_EXTRA_TYPE user_defined , yyscan_t scanner);

int 		  yylex(Token *lvalp, yyscan_t pLexScanner);

//=============================================================================

struct PnLexerRef {
    inline PnLexerRef(STRINGSTREAM *inLexerStream, YY_EXTRA_TYPE extraArgs) {
        yylex_init(&pLexScanner);
        yyset_in(inLexerStream, pLexScanner);
        yyset_extra(extraArgs, pLexScanner);
    }
    inline ~PnLexerRef() { yylex_destroy(pLexScanner); }

    inline int GetNextToken(Token *lvalp)	{ return yylex(lvalp, pLexScanner); }

    yyscan_t pLexScanner;
};

//=============================================================================
//   Reentrant parser API
//=============================================================================

// Structure for terminal tokens
struct Token {
    int 	integer;
    double 	real;
    string  text;
    // string 	rate_par_name;
    // string 	mark_par_name;
    // string 	place_name;
    // string 	transition_name;
    // string 	ap_name;
    // string 	action_name;
    // string 	clockval_name;
};

enum ParserStatus {
    PS_ACCEPTED, PS_FAILED, PS_RUNNING
};

// Additional arguments passed to the parser
struct ExtraParserArgs : boost::noncopyable {
    inline ExtraParserArgs(const PN *pn, const DTA *dta)
        : status(PS_RUNNING), pPN(pn), pDTA(dta) { }

    ParserStatus			status;		// Status of the parser
    shared_ptr<ParserObj> 	pOutObj;	// Parsed object (the output of the parser)
    const PN				*pPN;		// PetriNet used by the parser (for name lookups)
    const DTA				*pDTA;		// DTA object (for atomic proosition lookups)
};

void *PnParserAlloc(void *(*)(size_t));

void  PnParserFree(void *, 			 /* The parser to be deleted */
                   void (*)(void *)); /* Function used to reclaim memory */

void  PnParser(void *,             	/* The parser */
               int,            		/* The major token code number */
               Token *, 		  		/* The value for the token */
               ExtraParserArgs *);  /* Optional %extra_argument parameter */

void PnParserTrace(FILE *TraceFILE, char *zTracePrompt);

struct PnParserRef {
    inline PnParserRef() { pParser = PnParserAlloc(nullptr); }
    inline ~PnParserRef() { PnParserFree(pParser, nullptr); }
    inline void Parse(int minor, Token *value, ExtraParserArgs *args)
    {   PnParser(pParser, minor, value, args);   }
    inline void Trace(FILE *TraceFILE, char *zTracePrompt)
    {	PnParserTrace(TraceFILE, zTracePrompt);   }

    void *pParser;	// Holds the pointer to the Lemon-generated parser
};

//=============================================================================
#endif   /* __PARSER_DEFS_H__ */




















