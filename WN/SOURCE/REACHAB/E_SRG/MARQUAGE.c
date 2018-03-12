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
        printf("\nClasse:%s {", tabc[i].col_name);
        for (j = 0; j < tot[i]; j++)
            printf("<Z%d%d>", i, j);
        printf("}");
    }
    printf("\n\n");
//  printf("\n*********MARQUAGE*************\n");
    for (i = 0; i < npl; i++) {
        ptr = net_mark[i].marking;
        printf("\n Place (%s):", tabp[i].place_name);
        while (ptr) {
            printf("<");
            for (k = 0; k < tabp[i].comp_num - 1; k++) printf("Z%d%d,", tabp[i].dominio[k], ptr->id[k]);

            printf("Z%d%d", tabp[i].dominio[k], ptr->id[k]);
            ptr = ptr->next;
            if (ptr)printf(">+"); else printf(">");
        }
    }
    // printf("\n****** fin du marquage**********\n");
    // getchar();
}
