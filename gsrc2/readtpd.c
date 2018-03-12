#define MAX_TOKEN 32767


#include <stdio.h>
#include <stdlib.h>

char  netname[1000];
char  filename[1000];

extern void store_double(/* valp, fp */);
/*
  double * valp;
  FILE * fp;
*/

extern void load_double(/* valp, fp */);
/*
  double * valp;
  FILE * fp;
*/

unsigned sub_num, group_num, trans_num, place_num;

int
main(argc, argv)
int argc;
char **argv;
{
    FILE  *grgfp, * tpdfp;
    double mp;
    double mint, maxt;
    long jj, n;


    if (argc < 2) {
        printf("ERROR: no net name !\n");
        exit(1);
    }
    sprintf(netname, "%s", argv[1]);

    sprintf(filename, "%s.grg", netname);
    if ((grgfp = fopen(filename, "r")) == NULL) {
        printf("can_t_open %s\n", filename);
        exit(1);
    }

    /* read number of objects in the net */
    fscanf(grgfp, "%d %d %d %d\n", &sub_num, &place_num, &group_num, &trans_num);
    printf("the net  %s has %d places\n", netname, place_num);
    printf("Observe: the places are ordered according to their position in the file .net\n");
    fclose(grgfp);

    sprintf(filename, "%s.tpd", netname);
    if ((tpdfp = fopen(filename, "r")) == NULL) {
        printf("can_t_open %s\n", filename);
        exit(1);
    }


    for (n = 1; n <= place_num; n++) {
        double sumprob = 0;
        double average = 0;
        load_double(&mint, tpdfp);
        load_double(&maxt, tpdfp);
        for (jj = 0; jj <= maxt - mint ; jj++) {
            load_double(&mp, tpdfp);
            sumprob += mp;
            average += mp * jj;
            printf("P(p%ld): %ld\n", n, jj/*,mp*/);
        }
        printf("\tE[p%ld]: %f\n", n, average);
    }
    (void) fclose(tpdfp);
}
