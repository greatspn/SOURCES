
#error THIS FILE IS NO LONGER IN USE
#error ITS CONTENT HAS BEEN MOVED INTO UTILS.H/CPP
#error IT IS KEPT ONLY IN CASE OF BUG FIXES


// #ifndef __CONSOLE_H__
// #define __CONSOLE_H__
// //=============================================================================

// namespace console {

// 	struct escape_sequence;

// #ifdef WIN32
// 	//=========================================================================
// 	// Windows Console API
// 	//=========================================================================

// 	HANDLE get_stdout_handle();
// 	WORD get_default_stdout_attr();

// 	struct escape_sequence {
// 		inline escape_sequence(WORD rmvAttr, WORD addAttr)
// 		/**/ : wRemovedAttr(rmvAttr), wAddedAttr(addAttr),
// 		/**/   dx(0), dy(0), absolute(false) { }

// 		inline escape_sequence(int _dx, int _dy, bool _abs)
// 		/**/ : wRemovedAttr(0), wAddedAttr(0),
// 		/**/   dx(_dx), dy(_dy), absolute(_abs)  { }

// 		WORD  wRemovedAttr, wAddedAttr;
// 		int dx, dy;
// 		bool absolute;
// 	};

// 	template <class charT, class traits>
// 	basic_ostream<charT,traits>& operator<< (basic_ostream<charT,traits>& os,
// 											 const escape_sequence& es)
// 	{
// 		os << flush;
// 		COORD coord;
// 		CONSOLE_SCREEN_BUFFER_INFO csbi;
// 		HANDLE stdout_handle = get_stdout_handle();
// 		if (GetConsoleScreenBufferInfo(stdout_handle, &csbi)) {
// 			WORD newAttr = csbi.wAttributes;
// 			newAttr &= ~es.wRemovedAttr;
// 			newAttr |= es.wAddedAttr;
// 			if (!es.absolute) {
// 				coord.X = SHORT(csbi.dwCursorPosition.X + es.dx);
// 				coord.Y = SHORT(csbi.dwCursorPosition.Y + es.dy);
// 			}
// 			else {
// 				coord.X = es.dx;
// 				coord.Y = es.dy;
// 			}
// 			if (newAttr != csbi.wAttributes)
// 				SetConsoleTextAttribute(stdout_handle, newAttr);
// 			if (coord.X != csbi.dwCursorPosition.X ||
// 				coord.Y != csbi.dwCursorPosition.Y)
// 				SetConsoleCursorPosition(stdout_handle, coord);
// 		};
// 		return os;
// 	}

// #define FOREGROUND_CLR_MASK \
// 	(FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED)
// #define BACKGROUND_CLR_MASK \
// 	(BACKGROUND_BLUE | BACKGROUND_GREEN | BACKGROUND_RED)
// #define CLR_MASK   (FOREGROUND_CLR_MASK | BACKGROUND_BLUE)

// 	inline escape_sequence default_disp()
// 	{  return escape_sequence(0xFFFF, get_default_stdout_attr());  }


// 	inline escape_sequence bold_on()
// 	{  return escape_sequence(0,0);  }

// 	inline escape_sequence dim_on()
// 	{  return escape_sequence(FOREGROUND_INTENSITY,0);  }

// 	inline escape_sequence underline_on()
// 	{  return escape_sequence(0,COMMON_LVB_UNDERSCORE);  }

// 	inline escape_sequence blink_on()
// 	{  return escape_sequence(0,0);  }

// 	inline escape_sequence reverse_on()
// 	{  return escape_sequence(0,COMMON_LVB_REVERSE_VIDEO);  }

// 	inline escape_sequence hidden_on()
// 	{  return escape_sequence(0,0);  }


// 	inline escape_sequence bold_off()
// 	{  return escape_sequence(0,0);  }

// 	inline escape_sequence dim_off()
// 	{  return escape_sequence(0,FOREGROUND_INTENSITY);  }

// 	inline escape_sequence underline_off()
// 	{  return escape_sequence(COMMON_LVB_UNDERSCORE,0);  }

// 	inline escape_sequence blink_off()
// 	{  return escape_sequence(0,0);  }

// 	inline escape_sequence reverse_off()
// 	{  return escape_sequence(COMMON_LVB_REVERSE_VIDEO,0);  }

// 	inline escape_sequence hidden_off()
// 	{  return escape_sequence(0,0);  }


// 	inline escape_sequence makecolor(WORD clr) {
// 		return escape_sequence(FOREGROUND_CLR_MASK|FOREGROUND_INTENSITY,
// 									  clr);
// 	}

// 	inline escape_sequence color_fgnd_off() {
// 		WORD defClr = get_default_stdout_attr() & (FOREGROUND_CLR_MASK |
// 											       FOREGROUND_INTENSITY);
// 		return makecolor(defClr);
// 	}

// 	inline escape_sequence black_fgnd()
// 	{   return makecolor(0);   }
// 	inline escape_sequence red_fgnd()
// 	{   return makecolor(FOREGROUND_RED);   }
// 	inline escape_sequence green_fgnd()
// 	{   return makecolor(FOREGROUND_GREEN);   }
// 	inline escape_sequence yellow_fgnd()
// 	{   return makecolor(FOREGROUND_RED | FOREGROUND_GREEN);   }
// 	inline escape_sequence blue_fgnd()
// 	{   return makecolor(FOREGROUND_BLUE);   }
// 	inline escape_sequence magenta_fgnd()
// 	{   return makecolor(FOREGROUND_RED | FOREGROUND_BLUE);   }
// 	inline escape_sequence cyan_fgnd()
// 	{   return makecolor(FOREGROUND_BLUE | FOREGROUND_GREEN);   }
// 	inline escape_sequence white_fgnd()
// 	{   return makecolor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);   }

// 	inline escape_sequence black_bgnd()
// 	{   return makecolor(0);   }
// 	inline escape_sequence red_bgnd()
// 	{   return makecolor(BACKGROUND_RED);   }
// 	inline escape_sequence green_bgnd()
// 	{   return makecolor(BACKGROUND_GREEN);   }
// 	inline escape_sequence yellow_bgnd()
// 	{   return makecolor(BACKGROUND_RED | BACKGROUND_GREEN);   }
// 	inline escape_sequence blue_bgnd()
// 	{   return makecolor(BACKGROUND_BLUE);   }
// 	inline escape_sequence magenta_bgnd()
// 	{   return makecolor(BACKGROUND_RED | BACKGROUND_BLUE);   }
// 	inline escape_sequence cyan_bgnd()
// 	{   return makecolor(BACKGROUND_BLUE | BACKGROUND_GREEN);   }
// 	inline escape_sequence white_bgnd()
// 	{   return makecolor(BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE);   }


// 	// Cursor position
// 	inline escape_sequence cur_xy(int x, int y)
// 	{   return escape_sequence(x, y, true);   }

// 	inline escape_sequence cur_up(int lines)
// 	{   return escape_sequence(0, -lines, false);   }

// 	inline escape_sequence cur_down(int lines)
// 	{   return escape_sequence(0, lines, false);   }

// 	inline escape_sequence cur_right(int lines)
// 	{   return escape_sequence(lines, 0, false);   }

// 	inline escape_sequence cur_left(int lines)
// 	{   return escape_sequence(-lines, 0, false);   }

// #else

// 	//=========================================================================
// 	//   Unix Terminal escape sequences
// 	//=========================================================================
// 	static int g_isXTermColor = -1;

// 	struct escape_sequence {
// 		inline escape_sequence(const char *_escSeq) : escSeq(_escSeq) {}
// 		inline escape_sequence(std::string &&_escStr) : escSeq(nullptr), escStr(_escStr) {}
// 		const char *escSeq;
// 		const std::string escStr;
// 	};


// 	template<class ostream_t>
// 	inline ostream_t& operator << (ostream_t& os, const escape_sequence& esp) {
// 		if (g_isXTermColor==-1) {
// 			const char* term_evar = getenv("TERM");
// 			if (term_evar!=nullptr &&
// 				(0==strcmp(term_evar, "xterm") ||
// 				 0==strcmp(term_evar, "xterm-color") ||
// 				 0==strcmp(term_evar, "xterm-256color") ))
// 				g_isXTermColor = 1;
// 			else
// 				g_isXTermColor = 0;
// 		}
// 		if (g_isXTermColor) {
// 			if (esp.escSeq != nullptr)
// 				os << esp.escSeq;
// 			else
// 				os << esp.escStr;
// 		}
// 		return os;
// 	}


// 	inline escape_sequence makeescseq(const char *escSeq)
// 	{   return escape_sequence(escSeq);   }
// 	inline escape_sequence makeescseq(std::string &&escStr)
// 	{   return escape_sequence(std::move(escStr));   }


// 	// Attributes
// 	inline escape_sequence default_disp()   {  return makeescseq("\033[0m");  }
// 	inline escape_sequence bold_on()	    {  return makeescseq("\033[1m");  }
// 	inline escape_sequence dim_on()		    {  return makeescseq("\033[2m");  }
// 	inline escape_sequence underline_on()   {  return makeescseq("\033[4m");  }
// 	inline escape_sequence blink_on()       {  return makeescseq("\033[5m");  }
// 	inline escape_sequence reverse_on()     {  return makeescseq("\033[7m");  }
// 	inline escape_sequence hidden_on()      {  return makeescseq("\033[8m");  }

// 	inline escape_sequence bold_off()	    {  return makeescseq("\033[22m");  }
// 	inline escape_sequence dim_off()	    {  return makeescseq("\033[23m");  }
// 	inline escape_sequence underline_off()  {  return makeescseq("\033[24m");  }
// 	inline escape_sequence blink_off()      {  return makeescseq("\033[25m");  }
// 	inline escape_sequence reverse_off()    {  return makeescseq("\033[27m");  }
// 	inline escape_sequence hidden_off()     {  return makeescseq("\033[28m");  }

// 	// Color Codes
// 	inline escape_sequence color_fgnd_off() {  return makeescseq("\033[29m");  }

// 	inline escape_sequence black_fgnd()     {  return makeescseq("\033[30m");  }
// 	inline escape_sequence red_fgnd()       {  return makeescseq("\033[31m");  }
// 	inline escape_sequence green_fgnd()     {  return makeescseq("\033[32m");  }
// 	inline escape_sequence yellow_fgnd()    {  return makeescseq("\033[33m");  }
// 	inline escape_sequence blue_fgnd()      {  return makeescseq("\033[34m");  }
// 	inline escape_sequence magenta_fgnd()   {  return makeescseq("\033[35m");  }
// 	inline escape_sequence cyan_fgnd()      {  return makeescseq("\033[36m");  }
// 	inline escape_sequence white_fgnd()     {  return makeescseq("\033[37m");  }

// 	inline escape_sequence black_bgnd()     {  return makeescseq("\033[40m");  }
// 	inline escape_sequence red_bgnd()       {  return makeescseq("\033[41m");  }
// 	inline escape_sequence green_bgnd()     {  return makeescseq("\033[42m");  }
// 	inline escape_sequence yellow_bgnd()    {  return makeescseq("\033[43m");  }
// 	inline escape_sequence blue_bgnd()      {  return makeescseq("\033[44m");  }
// 	inline escape_sequence magenta_bgnd()   {  return makeescseq("\033[45m");  }
// 	inline escape_sequence cyan_bgnd()      {  return makeescseq("\033[46m");  }
// 	inline escape_sequence white_bgnd()     {  return makeescseq("\033[47m");  }

// 	// Cursor Position
// 	inline escape_sequence cur_xy(int x, int y) {
// 		ostringstream str;
// 		str << '\033' << '[' << y << ";" << x << "H";
// 		return makeescseq(str.str());
// 	}
// 	inline escape_sequence cur_up(int lines) {
// 		ostringstream str;
// 		str << '\033' << '[' << lines << "A";
// 		return makeescseq(str.str());
// 	}
// 	inline escape_sequence cur_down(int lines) {
// 		ostringstream str;
// 		str << '\033' << '[' << lines << "B";
// 		return makeescseq(str.str());
// 	}
// 	inline escape_sequence cur_right(int lines) {
// 		ostringstream str;
// 		str << '\033' << '[' << lines << "C";
// 		return makeescseq(str.str());
// 	}
// 	inline escape_sequence cur_left(int lines) {
// 		ostringstream str;
// 		str << '\033' << '[' << lines << "D";
// 		return makeescseq(str.str());
// 	}

// #endif

// };

// //=============================================================================
// #endif  // __CONSOLE_H__
















// //=============================================================================
// // Sequence of escape characters written in a single command
// struct escape_sequence_group {
// 	virtual ~escape_sequence_group() { }
// 	virtual void print_group(ostream& os) const =0;
// };

// template <class charT, class traits>
// basic_ostream<charT,traits>& operator<< (basic_ostream<charT,traits>& os,
// 										 const escape_sequence_group& esg)
// {
// 	esg.print_group(os);
// 	return os;
// }


// // Escape sequence groups for the special output command *_begin() and *_end()
// // These command are slightly different on Windows and Unix.

// #ifdef WIN32  // Windows version of the console commands

// #include <windows.h>
// #define Arc  ARC
// #undef max
// #undef min

// // #include "Console.h"

// struct beg_error : escape_sequence_group {
// 	virtual void print_group(ostream& os) const
// 	{  os << console::red_fgnd();  }
// };
// struct end_error : escape_sequence_group {
// 	virtual void print_group(ostream& os) const
// 	{  os << console::white_fgnd();  }
// };
// struct beg_title : escape_sequence_group {
// 	virtual void print_group(ostream& os) const
// 	{  os << console::dim_off();  }
// };
// struct end_title : escape_sequence_group {
// 	virtual void print_group(ostream& os) const
// 	{  os << console::dim_on();  }
// };
// struct beg_success : escape_sequence_group {
// 	virtual void print_group(ostream& os) const
// 	{  os << console::green_fgnd();  }
// };
// struct end_success : escape_sequence_group {
// 	virtual void print_group(ostream& os) const
// 	{  os << console::white_fgnd();  }
// };
// struct beg_emph : escape_sequence_group {
// 	virtual void print_group(ostream& os) const
// 	{  os << console::yellow_fgnd();  }
// };
// struct end_emph : escape_sequence_group {
// 	virtual void print_group(ostream& os) const
// 	{  os << console::white_fgnd();  }
// };
// struct one_line_up : escape_sequence_group {
// 	virtual void print_group(ostream& os) const
// 	{  os << console::cur_up(1);  }
// };

// #else   // Unix version of the console commands

// // #include "Console.h"

// struct beg_error : escape_sequence_group {
// 	virtual void print_group(ostream& os) const
// 	{  os << console::bold_on() << console::red_fgnd();  }
// };
// struct end_error : escape_sequence_group {
// 	virtual void print_group(ostream& os) const
// 	{  os << console::bold_off() << console::default_disp();  }
// };
// struct beg_title : escape_sequence_group {
// 	virtual void print_group(ostream& os) const
// 	{  os << console::bold_on() << console::underline_on();  }
// };
// struct end_title : escape_sequence_group {
// 	virtual void print_group(ostream& os) const
// 	{  os << console::bold_off() << console::underline_off();  }
// };
// struct beg_success : escape_sequence_group {
// 	virtual void print_group(ostream& os) const
// 	{  os << console::bold_on() << console::underline_on()
// 		  << console::green_fgnd();  }
// };
// struct end_success : escape_sequence_group {
// 	virtual void print_group(ostream& os) const
// 	{  os << console::bold_off() << console::underline_off()
// 		  << console::default_disp();  }
// };
// struct beg_emph : escape_sequence_group {
// 	virtual void print_group(ostream& os) const
// 	{  os << console::underline_on() << console::yellow_fgnd();  }
// };
// struct end_emph : escape_sequence_group {
// 	virtual void print_group(ostream& os) const
// 	{  os << console::underline_off() << console::default_disp();  }
// };
// struct one_line_up : escape_sequence_group {
// 	virtual void print_group(ostream& os) const
// 	{  os << console::cur_up(1);  }
// };

// #endif   // WIN32





