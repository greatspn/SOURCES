#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <assert.h>
#include "../../INCLUDE/const.h"
#include "../../INCLUDE/struct.h"
#include "../../INCLUDE/var_ext.h"
#include "../../INCLUDE/ealloc.h"

static FILE *def_fp = NULL;
// static FILE *parse_fp = NULL;

extern void lexer_set_buffer(const char *b);
extern int parser() ;
extern FILE *efopen() ;
extern FILE *nfp2 ;

inline static int islinebreak(int ch) {
    return (ch == '\n' || ch == '\r');
}

#ifdef SIMULATION
#ifndef SYMBOLIC
int parse_mdep_function_simulator(const char* fn, int trnum);
#endif // SYMBOLIC
#endif // SIMULATION

void read_DEF_file() {
    /* Init read_DEF_file */
    char tmp[MAXSTRING];
    // char prs[MAXSTRING];
    char lex_buffer[MAXSTRING+100];
    char trash[4];
    char *name_p = NULL;
    char type;
    int start_parse = FALSE;
    int tr;
    float xcoord, ycoord;


    sprintf(tmp, "%sdef", net_name);
    // sprintf(prs, "%sparse", net_name);
    def_fp = efopen(tmp, "r");
    while (TRUE) {
        /* Scanning .def file */
        if (fgets(tmp, MAXSTRING - 1, def_fp) == NULL) {
            if (feof(def_fp))
                break;
        }
        if (tmp[0] == '|') {
            /* Rate MD o delimitatore */
            if (tmp[1] == '2' && tmp[2] == '5' && tmp[3] == '6' && islinebreak(tmp[4])) {
                /* Delimitatore |256 */
                do {
                    if (NULL == fgets(tmp, MAXSTRING - 1, def_fp))
                        break;
                }
                while (!(tmp[0] == '|' && islinebreak(tmp[1])));
                start_parse = TRUE;
            }/* Delimitatore |256 */
            else {
                /* Definizioni di rate MD */
                if (parse_DEF == MDRATE) {
                    int should_parse = TRUE;
                    const size_t SZBUF = 2048;
                    char large_buf[SZBUF];
                    name_p = tmp;
                    sscanf(tmp + 1, "%d", &tr);
                    nmd = tr - 1;
                    if (NULL == fgets(large_buf, SZBUF - 1, def_fp)) {
                        fprintf(stderr, "Missing expected definition in .def file.\n");
                        exit(1);
                    }
                    if (tabt[nmd].pri == 127 && tabt[nmd].mean_t == 0.0) {
                        // 12/07/2016 Elvio: do not parse general functions, but store them.
                        // This change allows to encode the general function in the .def
                        // file (like any other marking-dependent functions), for the
                        // DSPN tool. The WN* tools ignore general functions, so this
                        // change does not affect the WN* tools.
                        tabt[nmd].general_function = strdup(large_buf);
                        printf("Transition %s has function:  %s\n",
                                tabt[nmd].trans_name, tabt[nmd].general_function);
                        should_parse = FALSE;
                    }
#ifdef SIMULATION
#ifndef SYMBOLIC
                    else if (tabt[nmd].pri == 0 && tabt[nmd].mean_t == 0.0) {
                        // Try interpreting this MDEP function definition as a general
                        // function policy for the simulator
                        should_parse = parse_mdep_function_simulator(large_buf, nmd);
                    }
#endif // SYMBOLIC
#endif // SIMULATION

                    // Parse the MDEP function
                    if (should_parse) {
                        /*tabmd[nmd].trans_ind = tr;*/
                        // Initialize Lex buffer
                        assert(strlen(large_buf) <= MAXSTRING);
                        sprintf(lex_buffer, "~z %s", large_buf);
                        lexer_set_buffer(lex_buffer);

                        parser();
                        lexer_set_buffer(NULL);
                    }
                }
                else if (parse_DEF == FALSE)
                    nmd++;
            }/* Definizioni di rate MD */
        }/* Rate MD o delimitatore */
        else if (start_parse) {
            /* Superato l'inizio file */
            if (parse_DEF != MDRATE) {
                /* Non e' la sola lettura degli MD terza passata */
                /* Lettura del nome della definizione (classe di colore....) */
                if (tmp[0] == '(') {
                    /* Definizioni di colori, marcature o funzioni */
                    name_p = tmp + 1;
                    sscanf(name_p, "%s %c %f %f %c", read_name, &type, &xcoord, &ycoord, trash);
                    if (parse_DEF == INITIAL_MARKING)
                        cnt++;
                    if (parse_DEF != FALSE)
                        strcpy(bufname, read_name);
                    strcpy(error_name, read_name);

                    /* Lettura della definizione */
                    if (NULL == fgets(tmp, MAXSTRING - 1, def_fp))  {
                        fprintf(stderr, "Missing expected definition in .def file.\n");
                        exit(1);
                    }

                    /* Tipo di definizione e in tmp c'e' la definizione */
                    switch (type) {
#ifdef SWN
                    case 'c' :
                        if (parse_DEF == FALSE || parse_DEF == COLOR_CLASS) {
                            // Initialize Lex buffer
                            assert(strlen(tmp) <= MAXSTRING);
                            sprintf(lex_buffer, "~c %s", tmp);
                            lexer_set_buffer(lex_buffer);

                            parser();
                            lexer_set_buffer(NULL);
                        }
                        break;
#endif
                    case 'm' : if (tmp[0] == '(') {
                            /* Definizione di sottoclasse dinamica */
                            if (parse_DEF == DYNAMIC_SUBCLASS) {
                                // Initialize Lex buffer
                                assert(strlen(tmp) <= MAXSTRING);
                                sprintf(lex_buffer, "~j %s", tmp);
                                lexer_set_buffer(lex_buffer);

                                parser();
                                lexer_set_buffer(NULL);
                            }
                        }/* Definizione di sottoclasse dinamica */
                        else {
                            /* Definizione di marcatura */
                            if (parse_DEF == INITIAL_MARKING) {
                                // printf("INIT MARKING \'%s\'' nmr=%d cnt=%d\n", tmp, nmr, cnt);
                                // tabm[nmr].file_pos = ftell(mfp);
                                // Store the place number and the marking expression string
                                tabm[nmr].position = cnt;
                                tabm[nmr].mark_expr = strdup(tmp);
                                nmr++;
                                // fprintf(mfp, "~m %s", tmp);
                            }
                            else if (parse_DEF == FALSE)
                                nmr++;
                        }/* Definizione di marcatura */
                        break;
                    case 'f' : if (parse_DEF == FUNCTION)
                            if (nfp2 != NULL)
                                fprintf(nfp2, "# define %s %s", bufname, tmp);
                            else {
                                fprintf(stderr, "OLD pre-processing of the net/def file is disabled.");
                            }
                        break;
                    }
                    if (NULL == fgets(tmp, MAXSTRING - 1, def_fp)) { }
                }/* Definizioni di colori, marcature o funzioni */
            }/* Non e' la sola lettura degli MD terza passata */
            else
                goto finish;
        }/* Superato l'inizio file */
    }/* Scanning .def file */
finish:
    fclose(def_fp);
    // sprintf(prs, "%sparse", net_name);
    // if ((parse_fp = fopen(prs, "r")) != NULL) {
    //     fclose(parse_fp);
    //     sprintf(tmp, "/bin/rm \"%s\"", prs);
    //     system(tmp);
    // }
}/* End read_DEF_file */




#ifdef SIMULATION
#ifndef SYMBOLIC
extern int get_mpar_value(const char* name, int* value);
extern int get_rpar_value(const char* name, double* value);

// Elvio 19/09/2016: Simulation mode (WNSIM/GSPNSIM):
// parse a marking-dependent general function definition as a delay distribution
int parse_mdep_function_simulator(const char* fn, int trnum) {
    int matches;
    char buf1[128], buf2[128];
    int i1, i2;
    double r1, r2;

    if (fn[0] == 'F' && fn[1] == 'N' && fn[2] == ':')
        fn += 3; // verbatim general function: skip the initial "FN:" prefix

    // Parse by pattern-matching:
    if (2 == sscanf(fn, "Erlang [ %[a-zA-Z_0-9.] , %[a-zA-Z_0-9] ]", buf1, buf2)) {
        // Erlang[ rate, stages ]
        if (0 == get_rpar_value(buf1, &r1)) // convert rate parameter (Erlang rate)
            r1 = atof(buf1);
        if (0 == get_mpar_value(buf2, &i2)) { // convert marking parameter (# of stages)
            i2 = atoi(buf2);
            if (fabs(atof(buf2) - i2) > 1.0e-5) {
                fprintf(stderr, "Error: expect an integral number of Erlang stages, found %s.\n", buf2);
                exit(-1);
            }
        }

        // printf("ERL rate=%lf stages=%d\n", r1, i2);
        tabt[trnum].timing = TIMING_ERLANG;
        tabt[trnum].policy = ENABLING_M;
        tabt[trnum].reuse_policy = POLICY_DIS_RANDOM;
        tabt[trnum].deschedule_policy = POLICY_DIS_RANDOM;
        tabt[trnum].mean_t = r1;
        tabt[trnum].stages = i2;

        return FALSE; // Do not parse over
    }
    if (2 == sscanf(fn, "Uniform [ %[a-zA-Z_0-9.] , %[a-zA-Z_0-9.] ]", buf1, buf2)) {
        if (0 == get_rpar_value(buf1, &r1))
            r1 = atof(buf1);
        if (0 == get_rpar_value(buf2, &r2))
            r2 = atof(buf2);

        // printf("UNIFORM %lf %lf\n", r1, r2);
        tabt[trnum].timing = TIMING_UNIFORM;
        tabt[trnum].policy = ENABLING_M;
        tabt[trnum].reuse_policy = POLICY_DIS_RANDOM;
        tabt[trnum].deschedule_policy = POLICY_DIS_RANDOM;
        tabt[trnum].stages = 0;
        tabt[trnum].mean_t = 0.0;
        tabt[trnum].dist = (double *)ecalloc(2, sizeof(double));
        tabt[trnum].dist[0] = r1;
        tabt[trnum].dist[1] = r2;

        return FALSE; // Do not parse over
    }

    printf("parse_mdep_function_simulator(): Unknown MDEP function %s of transition %s.\n",
           fn, tabt[trnum].trans_name);
    // Yet to parse
    return TRUE;
}
#endif // SYMBOLIC
#endif /* SIMULATION */



