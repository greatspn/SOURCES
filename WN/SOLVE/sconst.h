
/***   Adjustable constants   ***/
#ifdef MINIMUM
#define MAX_TRANS 1023 /*  2**8-1 <= MAX_TRANS <= 2**16-1  */
#define MAX_ROW 10240 /*  10*MAX_TRANS <= MAX_ROW  */
#define MAX_STACK 64 /*  7 > MAX_STACK <= 64  */
#define MAX_MARKS 524288  /*  2**15 <= MAX_MARKS < 2**22  */

#define MAX_ITEMS 4194305 /*  10*MAX_TRANS < MAX_ITEMS < 2**16  */
#define L2_MAX_ITEMS 256 /*  2**L2_MAX_ITEMS > MAX_ITEMS */

#define MRKSLOT 1024
#define MAX_ARR 512  /*  MAX_ARR * MRKSLOT == MAX_MARKS  */
#endif

#define MAX_STACK 64 /*  7 > MAX_STACK <= 64  */

/* Usate in swn_stndrd */
#ifdef TRASH
#define MAX_MARKS 524288  /*  2**15 <= MAX_MARKS < 2**22  */
#define MAX_ARR 512  /*  MAX_ARR * MRKSLOT == MAX_MARKS  */
#define MAX_TRANS 256 /*  2**8-1 <= MAX_TRANS <= 2**16-1  */
#endif

#define MAX_ROW 2561 /*  10*MAX_TRANS <= MAX_ROW  */

#define MAX_ITEMS  4194304/*  10*MAX_TRANS < MAX_ITEMS < 2**16  */
#define L2_MAX_ITEMS 256 /*  2**L2_MAX_ITEMS > MAX_ITEMS */

#define MRKSLOT 64
/* Usate in swn_stndrd */


#define LINEMAX 256
#define TAG_SIZE 64

/***   Internal constants   ***/

#define FALSE 0
#define TRUE 1

#define TO_PLACE 'o'
#define TO_TRANS 'i'
#define INHIBITOR 'h'

#define DETERM 127

#define short_SIZE (unsigned)(sizeof(short))
#define int_SIZE (unsigned)(sizeof(int))
#define long_SIZE (unsigned)(sizeof(long))

/***   Macros   ***/

#define ABS(X) ( ( (X) >= 0 ) ? (X) : -(X) )

