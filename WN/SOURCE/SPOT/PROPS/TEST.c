
#include "../../../INCLUDE/const.h"
#include "../../../INCLUDE/SCONSSPOT.h"
#include "../../../INCLUDE/struct.h"
#include "../../../INCLUDE/gspnlib.h"
#include  "../../../INCLUDE/SSTRUCTSPOT.h"
#include "../../../INCLUDE/var_ext.h"
#include  "../../../INCLUDE/SVAR_EXTSPOT.h"
#include "../../../INCLUDE/macros.h"

extern int succ();
extern int initialize();
extern int initial_state();
extern PART_MAT_P GetPartitions();
extern int finalize();
int main(int argc, char **argv) {
    State M0;
    State *succs;
    size_t  succ_size;
    signed char *prop = calloc(NbProp, sizeof(signed char));
    prop[0] = 1;
    prop[1] = 1;
    prop[2] = 1;
    prop[3] = 1;
    prop[4] = 1;
    prop[5] = 1;

    if (initialize(argc, argv) != 0) exit(1);

    initial_state(&M0);

    succ(NULL, prop, &succs, &succ_size);
    finalize();
    succ_free(succs) ;
}



