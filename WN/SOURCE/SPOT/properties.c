#include <stdlib.h>
#include <string.h>
#include "../../INCLUDE/const.h"
#include "../../INCLUDE/struct.h"
#include "../../INCLUDE/gspnlib.h"
#include "../../INCLUDE/var_ext.h"
#include "../../INCLUDE/macros.h"


/* ----------------- Property related Functions ----------------*/

/* Returns the index of the property given as "name"
   Non zero return = error codes
   return = 1 => property not found
*/

int prop_index(const char *name, pAtomicProp  propindex) {

    int i;

    /* look for an observation transition bearing this name == state atomic property */
    for (i = ntr ; i < nSpottr + ntr ; i++) {
        if (! strcmp(tabt[i].trans_name, name))
            break;
    }

    if (i == nSpottr + ntr) {
        /* else looking for a net transition == event based atomic property */
        for (i = 0 ; i < ntr ; i++) {
            if (! strcmp(tabt[i].trans_name, name))
                break;
        }
        if (i == ntr) {
            *propindex = EVENT_TRUE ;
            return 1;
        }
        else {
            /* add this transition to the list of those we wish to observe */
            tab_event_prop = realloc(tab_event_prop, ++tab_event_prop_size);
            tab_event_prop[tab_event_prop_size - 1] = i;
        }
    }

    *propindex = i ;

    return 0;
}


/* Returns the type of "prop" in "kind"
   non zero return = error codes
*/
int prop_kind(const AtomicProp prop, pAtomicPropKind  kind) {

    if (prop >= 0 && prop < ntr) {
        *kind = EVENT_PROP ;
    }
    else {
        *kind = STATE_PROP ;
    }
    return 0;
}

