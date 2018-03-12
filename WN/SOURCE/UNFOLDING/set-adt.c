/* BIT VECTOR Implementation  */


#include "set-adt.h"


/**************************************************************/
/* NAME :         init_set                                    */
/* DESCRIPTION  : create a new set                            */
/* PARAMETERS   : p_S pointer to the set                      */
/*                size dimension of set                       */
/* RETURN VALUE : OK if correct execution,                    */
/*                FALSE otherwise                             */
/* NOTE         : return created set pointed by p_S           */
/**************************************************************/
status init_set(set *p_S, int size) {
    int numbytes, i;

    numbytes = (size + BYTESIZE - 1) / BYTESIZE;
    BYTES(p_S) = (char *) malloc(numbytes);
    if (BYTES(p_S) == NULL)
        return ERROR;

    SIZE(p_S) = numbytes * BYTESIZE;
    for (i = 0; i < numbytes; i++)
        BYTES(p_S)[i] = 0;
    return OK;
}


static byte bit[BYTESIZE] = {0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80};



/**************************************************************/
/* NAME :         set_insert                                  */
/* DESCRIPTION  : insert in set element                       */
/* PARAMETERS   : p_S pointer to the set                      */
/*                element index to insert                     */
/* RETURN VALUE : OK if element is a valid index,             */
/*                FALSE otherwise                             */
/**************************************************************/
status set_insert(set *p_S, int element) {
    if (element < 0 || element >= SIZE(p_S))
        return ERROR;
    BYTES(p_S)[element / BYTESIZE] |= bit[element % BYTESIZE];
    return OK;
}



/**************************************************************/
/* NAME :         set_member                                  */
/* DESCRIPTION  : test if element is in set                   */
/* PARAMETERS   : p_S pointer to the set                      */
/*                element index to test                       */
/* RETURN VALUE : TRUE if element is in set,                  */
/*                FALSE if element is not in set or           */
/*                        element is not a valid index        */
/**************************************************************/
bool set_member(set *p_S, int element) {
    if (element < 0 || element >= SIZE(p_S))
        return FALSE;
    return (BYTES(p_S)[element / BYTESIZE] & bit[element % BYTESIZE])
           ? TRUE
           : FALSE;
}

