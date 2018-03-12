#ifndef err_incl
#define err_incl

/* ERROR MESSAGE PACKAGE */
/*
  Copyright notice: This is a test version. Please do not distribute nor trust
  too much on this. I hope to deliver a public version (perhaps GNU license)
  in the future. Antti Valmari 9.9.2010
*/
/*
  An error message is issued via a sequence of calls of the form M*R, where
  each M is either err::msg( param ) or err::msf( param ), and R is either
  err::ready() or err::ready( bool ). If there are no M's, then the sequence
  does nothing. Otherwise it prints the param's to standard error and may
  abort the program.

  err::msg does and err::msf does not print a space before the param, if there
  was a previous param.

  err::ready() and err::ready( true ) do and err::ready( false ) does not
  abort the program after a non-empty sequence of M's.

  err::is_on() returns true if and only if an error message has been started
  but not finished.

  err xxx( const char *param ) registers a string that will be printed towards
  the beginning of each error message issued during the lifetime of xxx. In
  this way context information can be added to error messages. If many such
  xxx are simultaneously alive, the oldest string is printed first.

  change_context( const char *param ) changes the most recent above-registered
  alive string to param.

  The design aims at informative error messages and ease of use without
  non-negligible additional runtime cost in the absence of errors. In
  particular, err::ready can be safely called even if it is not certain that
  an error situation has actually started.
*/


#include <iostream>
#include <cstdlib>

class err{

  static bool _is_on;   // error message has been started but not finished

  /* err objects constitute a linked list and each stores one message part */
  static err *ctx_beg, *ctx_end;
  err *prev, *next;
  const char *ctx_msg;

  /* Common tasks of error messages that belong together */
  static void start( bool spacy ){
    if( _is_on ){
      if( spacy ){ std::cerr << ' '; }
    }else{
      _is_on = true;
      std::cout.flush();    // yes, not cerr but cout
      std::cerr << "\n??? Error: ";
      err *cp = ctx_beg;
      while( cp ){ std::cerr << '[' << cp->ctx_msg << "] "; cp = cp->next; }
    }
  }

public:

  /* Constructor: add a new message part */
  err( const char *ctx_msg ): ctx_msg( ctx_msg ) {
    if( !ctx_msg ){ msg( "error context missing" ); ready(); }
    next = 0; prev = ctx_end;
    if( ctx_end ){ ctx_end->next = this; }else{ ctx_beg = this; }
    ctx_end = this;
  }

  /* Destructor: remove message part */
  ~err(){
    if( next ){ next->prev = prev; }else{ ctx_end = prev; }
    if( prev ){ prev->next = next; }else{ ctx_beg = next; }
    ready();    // catches a forgotten end of error message
  }

  /* Change error context */
  static void change_context( const char *cp ){
    if( !ctx_end ){ msg( "old error context missing" ); ready(); }
    if( !cp ){ msg( "new error context missing" ); ready(); }
    ctx_end->ctx_msg = cp;
  }

  /* Routines for printing individual message parts */
  static void msf( const char *par ){ start( false ); std::cerr << par; }
  static void msg( const char *par ){ start( true  ); std::cerr << par; }
  static void msf( long        par ){ start( false ); std::cerr << par; }
  static void msg( long        par ){ start( true  ); std::cerr << par; }

  /* Tests whether the error message has been started but not finished. */
  inline static bool is_on(){ return _is_on; }

  /* If error status is on, completes the error report. If do_abort = true, it
    also terminates the program if error status is on. */
  static void ready( bool do_abort = true ){
    if( !_is_on ){ return; }
    std::cerr << '\n';
    if( do_abort ){ exit(1); }
    _is_on = false;
  }

};
bool err::_is_on = false;
err *err::ctx_beg = 0, *err::ctx_end = 0;

#endif
