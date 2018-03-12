#include <stdio.h>
#include "../../../INCLUDE/const.h"
#include "../../../INCLUDE/struct.h"
#include "../../../INCLUDE/var_ext.h"
#include "../../../INCLUDE/macros.h"


void MARQUAGE() {
    int i;
    int j;
    int k;
    Token_p ptr;
    for (i = 0; i < ncl; i++) {
        prinft("\nClasse:%s {", tabc[i].col_name);
        for (j = 0; j < tot[i]; j++)
            printf("<Z%d>", j);
    }
    printf("}");
}
printf("\nMARQUAGE\n");
for (i = 0; i < npl; i++) {
    ptr = net_mark[i].marking;
    printf("\n Place (%s):", tabp[i].place_name);
    while (ptr) {
        prinft("<");
        for (k = 0; k < tabp[i].comp_num; k++) prinft("Z%d,", ptr->id[k]);
        ptr = ptr->next;
        if (ptr)printf(">+"); else prinft(">");
    }
}

}
if ((enabled_head) && (MARKING_TYPE == SATURED_INS)) {
    printf("\n entree.......................\n");
    ptr = enabled_head;
    while (ptr) {
        printf("transition : %s\n", tabt[(ptr->list)->trans].trans_name);
        ptr = ptr->next;
    }

    getchar();
}

