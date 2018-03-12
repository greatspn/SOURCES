
/***   Adjustable constants   ***/

#define MAX_TRANS 1023 /*  2**8-1 <= MAX_TRANS <= 2**16-1  */
#define MAX_ROW 10240 /*  10*MAX_TRANS <= MAX_ROW  */
#define MAX_STACK 512 /*  7 > MAX_STACK <= 64  */
/* #define MAX_MARKS 262144 */  /*  2**15 <= MAX_MARKS < 2**22  */
#define MAX_MARKS 4194304

#define MAX_ITEMS 65767 /*  10*MAX_TRANS < MAX_ITEMS < 2**16  */
#define L2_MAX_ITEMS 64 /*  2**L2_MAX_ITEMS > MAX_ITEMS */

#define MRKSLOT 1024
/*#define MAX_ARR 256    MAX_ARR * MRKSLOT == MAX_MARKS  */
#define MAX_ARR 4096

#define LINEMAX 2048
#define TAG_SIZE 512

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

