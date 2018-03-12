/*
CREATED  BY MARIETA
*/
# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <assert.h>
#include "../../INCLUDE/const.h"
# include "../../INCLUDE/ealloc.h"
# include "../../INCLUDE/struct.h"
# include "../../INCLUDE/var_ext.h"

static FILE *pin_prs = NULL;
static char f[MAXSTRING];


extern int parser() ;
extern FILE *efopen();

// Forward declarations
static int find_min();
static int find_max(int keep, int *checked_pinv);
static int common_places_are_not_touched(int *not_touched, int position);

// Array of arrays with P-invariants (dense vectors)
static int **loaded_pinvs = NULL;
// Number of p-invariants in loaded_pinvs.
static int num_pinv = 0;

/****************************************************************************************************/

int read_PIN_file() {
    char PIN_filename[1024];
    sprintf(PIN_filename, "%spin", net_name);
    int i, k;

    FILE *file = efopen(PIN_filename, "r");

    if (file == NULL) {
        fprintf(stderr, "Can't open file %s\n", PIN_filename);
        return -1;
    }

    // Read the number of P-semiflows
    if (1 != fscanf(file, "%d", &num_pinv)) {
        fprintf(stderr, "Bad P-semiflow file %s\n", PIN_filename);
        return -1;
    }
    if (num_pinv == 0)
        return -1; // No P-invariants in this <net_name>.pin file.

    loaded_pinvs = (int **)ecalloc(num_pinv, sizeof(int *));

    for (i = 0; i < num_pinv; i++) {
        // Read semiflow length
        int len;
        if (1 != fscanf(file, "%d", &len)) {
            fprintf(stderr, "Bad P-semiflow file %s\n", PIN_filename);
            num_pinv = 0;
            return -1;
        }
        if (len <= 0) {
            fprintf(stderr, "Bad P-semiflow with length of %d in %s.\n", len, PIN_filename);
            num_pinv = 0;
            return -1;
        }
        loaded_pinvs[i] = (int *)ecalloc(npl, sizeof(int));
        // Read the flow data
        for (k = 0; k < len; k++) {
            int card, place;
            if (2 != fscanf(file, "%d %d", &card, &place)) {
                fprintf(stderr, "Bad P-semiflow file %s\n", PIN_filename);
                num_pinv = 0;
                return -1;
            }
            place--;
            assert(place >= 0 && place < npl);
            loaded_pinvs[i][place] = card;
        }
    }
    fclose(file);
    return 0;
}

/****************************************************************************************************/

void free_PIN_file() {
    int i;
    if (loaded_pinvs) {
        for (i=0; i<num_pinv; i++) {
            free(loaded_pinvs[i]);
        }
        free(loaded_pinvs);
    }
    loaded_pinvs = NULL;
    num_pinv = 0;
}

/****************************************************************************************************/

int *sort_according_to_pinv() {

    int min, max, order, back, ok;
    int cnt, keep, keep_sec, map_position; //  keep = position on min, keep_sec = position of max
    int i, j, k, tmp, run;
    int visited_pinv, how_many, move ;
    int  *new_map_sort; /*inter_vec_min, *num_inter_max,*/
    int  *not_touched, *checked_pinv, *selected_place;
    int *pntr_ret;


    min = 100;
    max = order = 0;
    cnt = back = 0;
    i = j = k = 0;
    keep = 0;
    keep_sec = 0;
    tmp = 0;
    map_position = 0;
    checked_pinv = NULL;
    not_touched = NULL;
    new_map_sort = NULL;
    // itr = 0;
    run = 0;
    visited_pinv = 0;
    move = 1;
    pntr_ret = NULL;

    new_map_sort = (int *)calloc(npl, sizeof(int));
    not_touched = (int *)calloc(npl, sizeof(int));
    // stores if a p-invariant has already been selected
    checked_pinv = (int *)calloc(num_pinv, sizeof(int));
    // selected_place[i] != 0 means that place i has already been inserted into new_map_sort[]
    selected_place = (int*)calloc(npl, sizeof(int));


    for (i = 0; i < num_pinv; i++) {
        checked_pinv[i] = -1;
    }

    for (i = 0; i < npl; i++) {
        not_touched[i] = -1;
        new_map_sort[i] = -1;
        selected_place[i] = FALSE;
    }

    pntr_ret = new_map_sort;
    
    // Return immediately if there is no P-semiflow
    if (num_pinv == 0){
        fprintf(stderr, "No semiflows found!!\n");
        fflush(stderr);
        exit(-1);
        return pntr_ret;
    }



    keep = find_min();

    checked_pinv[keep] = keep;//  keep the first p-inv i keep OLNY the pinvs that I have dealt with and are not needed any more
    cnt = 0;

    while (move) {


        keep_sec = find_max(keep, checked_pinv);


        if (keep_sec == -1) { //if intersection is empty, put in the vector only the places of the first p-invariant
            for (i = 0; i < npl; i++) {

                if (loaded_pinvs[keep][i] == 1) { //&& !(loaded_pinvs[keep][i] & loaded_pinvs[keep_sec][i]) ){
                    if (!selected_place[i]) { // else i store it in the vector
                        new_map_sort[cnt++] = i;
                        selected_place[i] = TRUE;
                    }
                }
            }



        }

        else { // if intersection is non empty
            for (i = 0; i < npl; i++) { // store non common places
                if ((loaded_pinvs[keep][i] == 1) && !(loaded_pinvs[keep][i] && loaded_pinvs[keep_sec][i])) {
                    if (!selected_place[i]) {
                        new_map_sort[cnt++] = i;
                        selected_place[i] = TRUE;
                    }
                }
            }


            for (i = 0; i < npl; i++) { // store only common places between p-invariant

                if ((loaded_pinvs[keep][i] && loaded_pinvs[keep_sec][i])) {

                    back = common_places_are_not_touched(not_touched, i); //

                    if (!back) { // if it isn't already in common places
                        if (!selected_place[i]) { // if it isn't on the new_map_sort
                            new_map_sort[cnt++] = i;
                            selected_place[i] = TRUE;
                            not_touched[i] = i; // places which I am not allowed to touch
                        }
                        else { // if common places exists and i can touch it (isn't already in common )

                            for (j = 0; j < npl; j++) {
                                if (new_map_sort[j] == i) { // keep the position of new common place
                                    break;
                                }
                            }

                            for (tmp = j; tmp < cnt - 1; tmp++) {
                                new_map_sort[tmp] = new_map_sort[tmp + 1]; // reorder the vector
                            }


                            new_map_sort[cnt - 1] = i;
                            //cnt++; i don't increase the counter because i haven't use any new position


                            for (k = 0; k < cnt; k++) {
                                for (j = k + 1; j < cnt; j++) {
                                    if (new_map_sort[k] == new_map_sort[j]) {
                                        fprintf(stdout, "problem\n");
                                        exit(0);
                                    }
                                }
                            }
                        }
                    }
                }

            }



            for (i = 0; i < npl; i++) { // store places of the second p-invariant

                if (loaded_pinvs[keep_sec][i] == 1 && !(loaded_pinvs[keep_sec][i] && loaded_pinvs[keep][i])) {
                    if (!selected_place[i]) {
                        new_map_sort[cnt++] = i;
                        selected_place[i] = TRUE;
                    }
                }


            }
        }

        move = 0;
        for (i = 0; i < num_pinv; i++) {
            if (checked_pinv[i] != -1 || cnt == npl) {
                continue;
            }
            else {
                checked_pinv[i] = i;
                keep = i; // next p-invariant to deal with
                move = 1;
                break;
            }
        }

    }


    // for (k = 0; k < npl; k++) {
    //     //fprintf(stdout,"%d\t",new_map_sort[k]);
    // }


    free(not_touched);
    free(checked_pinv);
    free(selected_place);

    return pntr_ret;

}

/****************************************** FIND MIN INTERSECTION ****************************************************/
/*
 Finds the P-invariant with the minimum intersection with the others
 and returns its position on the vector where P-invariants are stored

 */
static int find_min() {

    int i, j, cnt, k;
    int *inter_vec_min;
    int min, keep;

    i = j = cnt = k = 0;
    keep = 0;
    min = 100;

    inter_vec_min = NULL;


    inter_vec_min = (int *)calloc(num_pinv, sizeof(int));

    if (inter_vec_min == NULL) {
        fprintf(stderr, "Error in malloc in inter_vec_min (read_PIN_file) \n");
    }


    for (i = 0; i < num_pinv; i++) {
        for (j = i + 1; j < num_pinv; j++) {
            k = 0;
            for (cnt = 0; cnt < npl; cnt++) {
                if (loaded_pinvs[i][cnt] && loaded_pinvs[j][cnt]) {
                    k++; // i = Pprev
                }
            }
            if (k > 0) {
                inter_vec_min[i]++;
            }

            if (inter_vec_min[i] < min && k > 0) {
                min = inter_vec_min[i]; // i shows the pinv with the minimum number of intersection with othe pinv
                keep = i; //BE AWARE it doesn't show the set of intersection only that there is intersection
            } // with another invariant
            if (k == 0) { // if there is no intersection with the other P-invariants
                min = 0;
                keep = i;
            }
        }

    }



    free(inter_vec_min);
    return keep;

}


/******************************************** FIND MAX INTERSECTION  **************************************************/
/*

  Finds the P-invariant with the maximum intersection with the others
 and returns its position on the vector where P-invariants are stored

 */


static int find_max(int keep, int *checked_pinv) {

    int i, j, cnt, max, keep_sec, k;
    int *num_inter_max;

    i = j = keep_sec = cnt = k = 0;
    max = -1;
    num_inter_max = NULL;


    num_inter_max = (int *)calloc(num_pinv, sizeof(int));

    if (num_inter_max == NULL) {
        fprintf(stderr, "Error in malloc in num_inter_max (read_PIN_file) \n");
    }


    for (i = 0; i < num_pinv; i++) {
        if (i == checked_pinv[i]) { // if p-invariant is already checked_pinv
            continue;
        }
        else {
            k = 0;
            for (cnt = 0; cnt < npl; cnt++) {
                if (loaded_pinvs[keep][cnt] && loaded_pinvs[i][cnt]) {
                    k++; // i = Pprev
                    num_inter_max[i]++; // max inter #places
                }
            }

            if (num_inter_max[i] > max && k > 0) {
                max = num_inter_max[i];// i shows the pinv with the maximum number of intersection with othe pinv
                keep_sec = i; //BE AWARE it doesn't show the set of intersection only that there is intersection
                //k = 0;//printf("min intersection exei to %d \n",keep);
            }// with another invariant

            if (k == 0) {
                if (max == -1) {
                    keep_sec = -1; // if there is no intersection deal only with the first p-invariant (position = keep)
                }
            }
        }

    }

    free(num_inter_max);
    return keep_sec;

}

/****************************************** COMMON PLACES ARE NOT TOUCHED ***********************************************/

/*
 Checks if the place already exists in the vector where common places are stored
 I know that this function with the above is the same. I wrote it twice to make it clear for me.
 Purpose of << reading >> the code. The final code will have only one function not both.

 */

static int common_places_are_not_touched(int *not_touched, int position) {


    int i;

    for (i = 0; i < npl; i++) {
        if (not_touched[i] == position) { // to pithanotero na vriskontai stin idia thesi
            return 1;
        }
    }

    return 0;

}

