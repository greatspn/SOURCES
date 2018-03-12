#include "global.h"
#include "showdialog.h"

void type_md_def(struct trans_object *trans) {
    char            string[10000];

    sprintf(string, "transition %s m-d rate : \n%s", trans->tag, trans->mark_dep);
    ShowShowDialog(string);
}


