#include <stdio.h>
#include <stdlib.h>
extern int initialize() ;
extern int finalize();
extern void dsrg_graph();
extern void mc_dsrg_graph();
extern void reach_dsrg_graph();
extern void mc_dsrg_partial_graph(int comp);

void dsrg_usage(void) {

    fprintf(stdout, "\n\n-------------------------------------------------------\n");
    fprintf(stdout, "USE : dsrg netname [-i,-o,-m,-k]                           \n");
    fprintf(stdout, "  -o : textual format for the reachbility graph            \n");
    fprintf(stdout, "  -i : use the inclusion                                   \n");
    fprintf(stdout, "  -m : generate a DTMC                                     \n");
    fprintf(stdout, "  -k : ensure that the resulting grpah is <= |RG|          \n");
    fprintf(stdout, "-----------------------------------------------------------\n");

}


int main(int argc, char **argv) {
    int ii;
    int opt = 0, in = 0;
    int comp = 0;

    if (argc < 2) {
        dsrg_usage();
        exit(1);
    }

    for (ii = 2; ii < argc; ii++) {

        if (argv[ii][1] == 'm') opt = 1;
        if (argv[ii][1] == 'i') in = 1;
        if (argv[ii][1] == 'k') comp = 1;

        if (argv[ii][1] != 'm' &&
                argv[ii][1] != 'i' &&
                argv[ii][1] != 'p' &&
                argv[ii][1] != 'o' &&
                argv[ii][1] != 'k') {

            dsrg_usage();
            exit(1);
        }

    }

    initialize(argc, argv);
    if (opt) {
        if (!in)  mc_dsrg_graph(comp);
        else mc_dsrg_partial_graph(comp);
    }
    else  reach_dsrg_graph(comp);


    finalize();
}
