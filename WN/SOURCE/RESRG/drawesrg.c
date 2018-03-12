
#include "class.h"
#include <time.h>

using namespace std;

extern "C" {
    extern void OutPutParser(char *net);
    extern Q_list *Create_Q(char *net_name);
    extern int print_state_ctmc(Marking *s, char **st);
}



int print_state_ctmc(Marking *s, char **st) {
    FILE *fd;
    int pos;
    char  *mark = "states.mark";
    fd = fopen(mark, "w+");
    if (!fd) {
        cerr << "Error opening output stream" << endl;
        throw ExceptionIO();
    }
    load_sym_rep(s->get_srpos(), s->get_srlength(), s->get_srptr());
    if (s->get_type() == INSTANCE) {
        load_event(s->get_evlength(), s->get_evpos(), 1);
        NEW_ASYM_MARKING(&tabc, &net_mark, &card, &num, &tot,
                         MERG, ASYM_STATIC_STORE, RESULT);
    }
    get_canonical_marking();
    //write_on_srg(fd, (s == initial_marking ? 0 : 1) );
    write_on_srg(fd, 1);
    pos = ftell(fd);
    *st = (char *)malloc((pos + 1) * sizeof(char));
    fseek(fd, 0, SEEK_SET);
    fread(*st, sizeof(char), pos * sizeof(char), fd);
    (*st)[pos] = '\0';
    fclose(fd);
    return 0;
}

int main(int  argc,  char  *argv[]) {




}



