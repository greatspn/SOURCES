#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <float.h>
#include <assert.h>
#include "../../INCLUDE/const.h"
#include "../../INCLUDE/struct.h"
#include "../../INCLUDE/var_ext.h"
#include "../../INCLUDE/fun_ext.h"
#include "../../INCLUDE/macros.h"
#include "../../INCLUDE/ealloc.h"

#ifdef SIMULATION
extern double random_generate();
extern double read_t_st();
extern int events_made;
extern int  event_inst;
extern int old_made;
extern double old_time;
extern double cur_time;
extern time_t newt;
extern time_t oldt;
extern FILE *thptfp;
extern FILE *timefp;
extern FILE *statfp;

static FILE *resfp = NULL;
static FILE *stafp = NULL;

static int transitory = TRUE;
static int n_batch = 0 ;
// Number of events the next batch will have to simulate (event-driven batch)
static int batch_events = UNKNOWN;
// Duration of next batch (time-driven batch)
static double batch_duration = UNKNOWN;
static double start_of_batch_time;
extern int max_batches;

static double t_med_n = 0.0;
static double t_med_i = 0.0;

/***************** PER i POSTI *******************/
static double *tok_i = NULL;
static double *tmn = NULL;

/***************** PER LE TRANSIZIONI *******************/
static double *fire_i = NULL;
static double *fire_n = NULL;

/***************** PER ENTRAMBI *******************/
static double *tsq_f = NULL;
static double *ttsq_f = NULL;

extern FILE *statfp;
extern FILE *thptfp;
extern FILE *timefp;
extern int num_event;


#if STATISTICFEL
extern long numEvFEL;
extern long numEvOrdFEL;
#endif


/**************************************************************/
extern int invoked_from_gui();
extern int print_stat_for_gui() ;
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
static int is_tagged(int ii, int type)
{
    /* Init is_tagged */
    return ((type == TRANSITION) ? tabt[ii].tagged : tabp[ii].tagged);
}/* End is_tagged */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
static int get_batch_lenght(int min, int max)
{
    /* Init get_batch_lenght */
    return (min + floor((max - min) * random_generate()));
}/* Init get_batch_lenght */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
static int get_batch_duration(double min, double max)
{
    return min + ((max - min) * random_generate());
}
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
static int end_batch(const BatchLengthMode batch_length_mode) {
    int end = FALSE;
    // event-mode: do we have simulated enough events?
    if (batch_length_mode == BLM_BY_EVENT_COUNT) {
        if ((events_made - old_made + 1) == batch_events)
            end = TRUE;
    }
    else {
        assert(batch_length_mode == BLM_BY_DURATION);
        if ((cur_time - start_of_batch_time) >= batch_duration)
            end = TRUE;
    }

    if (end) {
        // Fine del batch 
        old_made = events_made;
        transitory = TRUE;
        if (!invoked_from_gui())
            fprintf(stdout, "End batch number %d\n", n_batch + 1, cur_time);
    }
    return (transitory);
}

/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
int end_transitory(int first_trans, int initial_events, int min_lenght, int max_lenght,
                   double min_batch_duration, double max_batch_duration,
                   const BatchLengthMode batch_length_mode)
{
    /* Init transitory */
    int ii;
    int comp;

    if (n_batch == 0)
        comp = first_trans;
    else
        comp = initial_events;

    if (transitory) {
        /* Flag per il transitorio inizialmente a TRUE */
        if ((events_made - old_made) == comp) {
            /* Fine del transitorio e determinazione lunghezza batch */
            old_made = events_made;
            transitory = FALSE;

            assert(batch_length_mode == BLM_BY_EVENT_COUNT || 
                   batch_length_mode == BLM_BY_DURATION);
            if (batch_length_mode == BLM_BY_EVENT_COUNT)
                batch_events = get_batch_lenght(min_lenght, max_lenght);
            else {
                batch_duration = get_batch_duration(min_batch_duration, max_batch_duration);
                start_of_batch_time = cur_time;
            }
            for (ii = 0; ii < npl; ii++)
                tok_i[ii] = 0.0;
            for (ii = 0; ii < ntr; ii++)
                fire_i[ii] = 0.0;
            t_med_i = 0.0;
            if (!invoked_from_gui()) {
                if (batch_length_mode == BLM_BY_EVENT_COUNT)
                    fprintf(stdout, "\nStart batch number %d with %d events.\n", n_batch + 1, batch_events);
                else
                    fprintf(stdout, "\nStart batch number %d of duration %lf.\n", n_batch + 1, batch_duration);
            }
        }/* Fine del transitorio e determinazione lunghezza batch */
    }/* Flag per il transitorio inizialmente a TRUE */
    return (!transitory);
}/* End transitory */

/**************************************************************/
/* Approximation reached on places/transitions                */
/**************************************************************/
struct percentage_stat {
    double min_perc, max_perc, avg_perc;
    int num_elems;
};
/**************************************************************/
/* Print the convergence stat for the JavaGUI                 */
/**************************************************************/
static void print_stat_line(int type, const struct percentage_stat *perc, int approx, int ok) {
    const char *REACHED = "\033[1m\033[32mREACHED\033[0m";
    const char *NOT_YET = "\033[1m\033[31mNOT YET\033[0m";

    if (type == PLACE)
        printf("PLACES     : ");
    else if (type == TRANSITION)
        printf("TRANSITIONS: ");
    else
        printf("???????????: ");

    printf("AVG approx ");
    if (perc->avg_perc >= 0 && perc->avg_perc < 100)
        printf("%5.2lf%% ", perc->avg_perc);
    else
        printf("-----%% ");

    printf("(min ");
    if (perc->min_perc >= 0 && perc->min_perc < 100)
        printf("%5.2lf%% ", perc->min_perc);
    else
        printf("-----%% ");

    printf(", max ");
    if (perc->max_perc >= 0 && perc->max_perc < 100)
        printf("%5.2lf%% ", perc->max_perc);
    else
        printf("-----%% ");

    printf("),  target approx: %2d%% %s      \n", approx, ok ? REACHED : NOT_YET);
}
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
static int compute_confidence_level(int conf, int approx, int type, 
                                    FILE* outfp, FILE* lstafp, 
                                    struct percentage_stat *out_perc)
{
    /* Init compute_confidence_level */
    int ii, camp, elements, go = TRUE;
    double *nth;
    double t_sigma_quad = 0.0;
    double time_value, estimated_value, D, K, mid_value, inf_lim, sup_lim, delta, perc;
    double DEN;
    FILE *filefp = NULL;

    if (out_perc != NULL) {
        out_perc->min_perc = DBL_MAX;
        out_perc->max_perc = out_perc->avg_perc = 0.0;
        out_perc->num_elems = 0;
    }

    if (n_batch > 1) {
        /* Almeno un batch */
#ifdef GREATSPN
        if (type == TRANSITION || type == GREATSPN)
#else
        if (type == TRANSITION)
#endif
        {
            elements = ntr;
            nth = fire_n;
            filefp = thptfp;
        }
        else {
            elements = npl;
            nth = tmn;
            filefp = statfp;
        }
        for (camp = 1; camp <= n_batch ; camp++) {
            /* Per i campioni precedenti */
            fscanf(timefp, "%lf\n", &time_value);
            t_sigma_quad += (time_value - t_med_n) * (time_value - t_med_n);
            for (ii = 0; ii < elements; ii++) {
                /* Per ogni elemento */
                if (is_tagged(ii, type)) {
                    /* Elemento per il quale raccogliere le statistiche */
                    fscanf(filefp, "%lf\n", &estimated_value);
                    tsq_f[ii] += (estimated_value - nth[ii]) * (estimated_value - nth[ii]);
                    ttsq_f[ii] += (estimated_value - nth[ii]) * (time_value - t_med_n);
                }/* Elemento per il quale raccogliere le statistiche */
            }/* Per ogni elemento */
        }/* Per i campioni precedenti */
        t_sigma_quad /= (n_batch - 1) ;
        K = read_t_st(conf, n_batch - 1);
        K *= K;
        K /= n_batch;
        for (ii = 0 , go = TRUE ; ii < elements ; ii++) {
            /* Per ogni elemento */
            if (is_tagged(ii, type)) {
                /* Elemento per il quale raccogliere le statistiche */
                tsq_f[ii] /= (n_batch - 1);
                ttsq_f[ii] /= (n_batch - 1);
                D = (t_med_n * nth[ii] - K * ttsq_f[ii]) * (t_med_n * nth[ii] - K * ttsq_f[ii])
                    - (t_med_n * t_med_n - K * t_sigma_quad) * (nth[ii] * nth[ii] - K * tsq_f[ii]);
                if (D < 0.0)
                    D = 0.0;
                DEN = (t_med_n * t_med_n - K * t_sigma_quad);
                if (DEN != 0) {
                    /* Denominatore != 0 */
                    inf_lim = (t_med_n * nth[ii] - K * ttsq_f[ii] - sqrt(D)) / DEN;
                    sup_lim = (t_med_n * nth[ii] - K * ttsq_f[ii] + sqrt(D)) / DEN;
                }/* Denominatore != 0 */
                else {
                    /* Denominatore = 0 */
                    inf_lim = 0.0;
                    sup_lim = 0.0;
                }/* Denominatore = 0 */
                delta = (sup_lim - inf_lim) / 2;
                mid_value = delta + inf_lim;
                if (mid_value != 0.0)
                    perc = delta * 100.0 / mid_value ;
                else
                    perc = 0.0;
		        // fprintf(outfp,"mid_value=%f delta=%f perc=%f sup_lim=%f inf_lim=%f \n",
          //               mid_value,delta,perc,sup_lim,inf_lim);
                if (type == TRANSITION) {
// #warning "Not clear if we are taking the right scalar result (inf_lim, mid_value, sup_lim)."
                    if (invoked_from_gui()) {
                        printf("#{GUI}# RESULT ALL THROUGHPUT %s ~ %12.16lf %12.16lf %12.16lf\n", 
                                tabt[ii].trans_name, inf_lim, mid_value, sup_lim);
                    }
                    else {
                        fprintf(outfp, "Throughput of %s (%lf ): %1.12g <= X <= %1.12g \nValue %1.12g Mean Value %1.12g Accuracy %1.12g\n"
                                , tabt[ii].trans_name, fire_i[ii], inf_lim, sup_lim, nth[ii] / t_med_n, mid_value, perc);
                    }
                    if (lstafp != NULL)
                        fprintf(lstafp, "Thru_%s = %lg\n", tabt[ii].trans_name, nth[ii] / t_med_n);    
                }
                else if (type == PLACE) {
                    if (invoked_from_gui()) {
                        printf("#{GUI}# RESULT ALL MEAN %s ~ %12.16lf %12.16lf %12.16lf\n",
                                tabp[ii].place_name, inf_lim, mid_value, sup_lim);
                    }
                    else {
                        fprintf(outfp, "Mean n.of tokens in %s : %1.12g <= mu <= %1.12g\nValue %1.12g Mean Value %1.12g Accuracy %1.12g\n"
                                , tabp[ii].place_name, inf_lim, sup_lim, nth[ii] / t_med_n, mid_value, perc);
                    }
                    if (lstafp != NULL)
                        fprintf(lstafp, "%s = %lg\n", tabp[ii].place_name, nth[ii] / t_med_n);
                }
                else {
                    if (invoked_from_gui()) {
                        printf("#{GUI}# RESULT ALL THROUGHPUT %s ~ %12.16lf %12.16lf %12.16lf\n", 
                                tabt[ii].trans_name, inf_lim, mid_value, sup_lim);
                    }
                    else {
                        fprintf(outfp, "Thru_%s = %lf\n", tabt[ii].trans_name, nth[ii] / t_med_n);
                    }
                }
                go = go && (perc <= approx);
                // Save percentage for statistics
                if (out_perc != NULL) {
                    out_perc->min_perc = MIN(out_perc->min_perc, perc);
                    out_perc->max_perc = MAX(out_perc->max_perc, perc);
                    out_perc->avg_perc += perc;
                    out_perc->num_elems++;
                }
            }/* Elemento per il quale raccogliere le statistiche */
#ifdef GREATSPN
            else if (type == GREATSPN) {
                if (invoked_from_gui()) {
                    printf("#{GUI}# RESULT ALL THROUGHPUT %s ~ %12.16lf %12.16lf %12.16lf\n", 
                            tabt[ii].trans_name, inf_lim, mid_value, sup_lim);
                }
                else {
                    fprintf(outfp, "Thru_%s = %g\n", tabt[ii].trans_name, nth[ii] / t_med_n);
                }
            }
#endif
        }/* Per ogni elemento */
    }/* Almeno un batch */
    if (out_perc != NULL)
        out_perc->avg_perc /= out_perc->num_elems;

    return (go);
}/* End compute_confidence_level */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
static int check_results(int approx, int conf, char file[MAXSTRING])
{
    /* Init check_results */
    int ok_transition, ok_place, ii;
    int max;
    char buf[MAXSTRING], str[MAXSTRING], buf2[MAXSTRING];

    /********************** INIZIALIZZAZIONE ***********************/
    max = MAX(ntr, npl);
    /********************** INIZIALIZZAZIONE ***********************/
    n_batch++;
    /************* media campionaria del tempo al passo i *****************/
    /************* e scrittura sul file dei campioni **********************/
    t_med_n = ((n_batch - 1) * t_med_n + t_med_i) / n_batch;
    fprintf(timefp, "%lf\n", t_med_i);
    /************* media campionaria del tempo al passo i *****************/
    /************* e scrittura sul file dei campioni **********************/
    /********** AGGIORNAMENTO VALOR MEDIO PER IL NUMERO DI TOKEN **********/
    for (ii = 0; ii < npl; ii++)
        if (tabp[ii].tagged) {
            /* Posto per il quale raccogliere le statistiche */
            tmn[ii] = ((n_batch - 1) * tmn[ii] + tok_i[ii]) / n_batch;
            fprintf(statfp, "%lf\n", tok_i[ii]);
        }/* Posto per il quale raccogliere le statistiche */
    /********** AGGIORNAMENTO VALOR MEDIO PER IL NUMERO DI TOKEN **********/

    /**********    AGGIORNAMENTO THROUGHPUT DELLE TRANSIZIONI    **********/
    /************* e scrittura sul file dei campioni **********************/
    for (ii = 0; ii < ntr; ii++)
        if (tabt[ii].tagged) {
            /* Transizione per la quale raccogliere le statistiche */
            fire_n[ii] = ((n_batch - 1) * fire_n[ii] + fire_i[ii]) / n_batch;
            fprintf(thptfp, "%lf\n", fire_i[ii]);
        }/* Transizione per la quale raccogliere le statistiche */
    /**********    AGGIORNAMENTO THROUGHPUT DELLE TRANSIZIONI    **********/
    if (!invoked_from_gui()) {
        // Print on the terminal
        fprintf(stdout, "\n---------------------------------------------\n");
        fprintf(stdout, "|                BATCH NUMBER %d             |\n", n_batch);
        fprintf(stdout, "---------------------------------------------\n\n");
        fprintf(stdout, "Current time %lf\n\n", cur_time);
    }
    int end_criteria = 0;
    if (n_batch > 1) {
        struct percentage_stat place_stat, trns_stat;
        rewind(thptfp);
        rewind(timefp);
        for (ii = 0; ii < max; ii++)
            tsq_f[ii] = ttsq_f[ii] = 0.0;
        ok_transition = compute_confidence_level(conf, approx, TRANSITION, stdout, NULL, &trns_stat);
        rewind(statfp);
        rewind(timefp);
        if (!invoked_from_gui())
            fprintf(stdout, "---------------------------------------------\n\n");
        for (ii = 0; ii < max; ii++)
            tsq_f[ii] = ttsq_f[ii] = 0.0;
        ok_place = compute_confidence_level(conf, approx, PLACE, stdout, NULL, &place_stat);

        end_criteria = (ok_transition && ok_place && n_batch > 10);

        if (invoked_from_gui()) {
            static clock_t timeOfLastPrint = (clock_t)-1;
            const float PRINT_RATE = 0.50f; // update every half second
            const char* FOUR_LINES_UP = "\033[A\033[A\033[A\033[A";
            const char *REACHED = "\033[1m\033[32mREACHED\033[0m";
            const char *NOT_YET = "\033[1m\033[31mNOT YET\033[0m";
            clock_t currCloc = clock();

            if (timeOfLastPrint == (clock_t)-1 || // first time
                (currCloc - timeOfLastPrint)/(float)CLOCKS_PER_SEC > PRINT_RATE || // intermediate step
                end_criteria) // Last frame, convergence reached
            {
                if (timeOfLastPrint != (clock_t)-1) 
                    printf(FOUR_LINES_UP);
                else 
                    printf("\n");

                timeOfLastPrint = currCloc;
                printf("Batch      : %-5d   Time: %lf             \n", n_batch, cur_time);
                print_stat_line(PLACE, &place_stat, approx, ok_place);
                print_stat_line(TRANSITION, &trns_stat, approx, ok_transition);
                printf("\n");
                fflush(stdout);
            }
        }
    }
    // Cut the simulation after a fixed amount of batches
    if (max_batches > 0) {
        if (n_batch >= max_batches)
            end_criteria = TRUE;
    }

    if (end_criteria) {
        /* Fine simulazione */
        time(&newt);
        if (!invoked_from_gui())
            fprintf(stdout, "\n");
        if (newt != oldt) {
            fprintf(stdout, "\nEfficiency ---> %zu transition firings per second\n",
                    events_made / (newt - oldt));
            if (print_stat_for_gui())
                printf("#{GUI}# RESULT STAT firings_per_second %zu\n", events_made / (newt - oldt));
        }
        else {
            fprintf(stdout, "\nEfficiency ---> %d transition firings per second\n",
                    events_made);
            if (print_stat_for_gui())
                printf("#{GUI}# RESULT STAT firings_per_second %zu\n", events_made);
        }
        fprintf(stdout, "Time required for %d events -------> %zu\n", events_made, newt - oldt);
        if (print_stat_for_gui()) {
            printf("#{GUI}# RESULT STAT total_events %d\n", events_made);
            printf("#{GUI}# RESULT STAT simulation_time %zu\n", newt - oldt);
            printf("#{GUI}# RESULT STAT total_simulated_time %lf\n", cur_time);
            printf("#{GUI}# RESULT STAT num_batches %d\n", n_batch);
        }

#ifdef GREATSPN
        sprintf(buf, "%s.simres", file);
        sprintf(buf2, "%s.sta", file);
#else
        sprintf(buf, "nets/%s.simres", file);
        sprintf(buf2, "nets/%s.sta", file);
#endif
#ifdef SWN
        for (ii = 0; ii < ncl; ii++) {
            sprintf(str, "%s%d", tabc[ii].col_name, tabc[ii].card);
            strcat(buf, str);
        }
        resfp = efopen(buf, "w");
        fprintf(resfp, "\n");
#ifdef COLOURED
        fprintf(resfp, " ******* Coloured Simulation *******\n");
#endif
#ifdef SYMBOLIC
        fprintf(resfp, " ******* Symbolic Simulation *******\n");
#endif
#endif // SWN
#ifdef GSPN
        resfp = efopen(buf, "w");
        fprintf(resfp, " ******* GSPN Simulation *******\n");
#endif
        stafp = efopen(buf2, "w");
        fprintf(resfp, "\n");
#ifdef DEBUG_malloc
#ifdef SIMULATION
        fprintf(resfp, "MEAN NUMBER OF EVENTS : %lf\n",
                (float)tot_event / events_made);
        fprintf(resfp, "MAX NUMBER OF EVENTS : %d\n", max_ev);
        fprintf(resfp, "MIN NUMBER OF EVENTS : %d\n", min_ev);
        fprintf(resfp, "\n");
#if STATISTICFEL
        fprintf(resfp, "NUMBER OF SYMBOLIC EVENTS: %lf VS NUMBER OF ORDINARY EVENTS: %lf\n", (float)numEvFEL / events_made, (float)numEvOrdFEL / events_made);
        fprintf(resfp, "NUMBER OF ORDINARY EVENTS/NUMBER OF SYMBOLIC EVENTS: %lf\n", (float)numEvOrdFEL / numEvFEL);
#endif

        fprintf(resfp, " ***********************************\n");
        fprintf(resfp, "\n");
#endif
#endif
        fprintf(resfp, "Results :         \n");

        rewind(thptfp);
        rewind(timefp);
        for (ii = 0; ii < max; ii++)
            tsq_f[ii] = ttsq_f[ii] = 0.0;
        ok_transition = compute_confidence_level(conf, approx, TRANSITION, resfp, stafp, NULL);
        rewind(statfp);
        rewind(timefp);
        for (ii = 0; ii < max; ii++)
            tsq_f[ii] = ttsq_f[ii] = 0.0;
        ok_place = compute_confidence_level(conf, approx, PLACE, resfp, stafp, NULL);
        if (newt - oldt <= 0)
            newt++;
        fprintf(resfp, "\n");
        fprintf(resfp, "Efficiency ---> %zu transition firings per second\n",
                events_made / (newt - oldt));
        fprintf(resfp, "Time required for %d events -------> %zu\n"
                , events_made, newt - oldt);
        fprintf(resfp, "Simulated time --------> %lf\n", cur_time);
        fprintf(resfp, "Numero di campioni usati  --------> %d\n", n_batch);
        fprintf(resfp, "Grado di approssimazione --------> %d\n", approx);
        fprintf(resfp, "Livello di confidenza  --------> %d\n", conf);
        fprintf(resfp, "\n");
        fprintf(resfp, " ***********************************\n");
        fprintf(resfp, "\n");
        fclose(resfp);
        fclose(stafp);
#ifdef GREATSPN
        sprintf(buf, "%s.sta", file);
        resfp = efopen(buf, "w");
        for (ii = 0; ii < max; ii++)
            tsq_f[ii] = ttsq_f[ii] = 0.0;
        ok_transition = compute_confidence_level(conf, approx, GREATSPN, resfp, NULL, NULL);
        fclose(resfp);
        /*  sprintf(buf,"%s.mpd",file);
          resfp = efopen(buf,"aw");
          fclose(resfp);
          sprintf(buf,"%s.epd",file);
          resfp = efopen(buf,"aw");
          fclose(resfp);
          sprintf(buf,"%s.tpd",file);
          resfp = efopen(buf,"aw");
          fclose(resfp); */
#endif
        return (FALSE);
    }/* Fine simulazione */
    return (TRUE);
}/* End check_results */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
void init_stat() {
    /* Init init_stat */
    int ii;

    tok_i = (double *)ecalloc(npl, sizeof(double));
    tmn = (double *)ecalloc(npl, sizeof(double));
    for (ii = 0; ii < npl; ii++)
        tmn[ii] = 0;

    fire_i = (double *)ecalloc(ntr, sizeof(double));
    fire_n = (double *)ecalloc(ntr, sizeof(double));
    for (ii = 0; ii < ntr; ii++)
        fire_n[ii] = 0;

    tsq_f = (double *)ecalloc(MAX(ntr, npl), sizeof(double));
    ttsq_f = (double *)ecalloc(MAX(ntr, npl), sizeof(double));
}/* End init_stat */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
int compute_batch_statistics(int ev_type, int approx, int conf, char file[MAXSTRING],
                             const BatchLengthMode batch_length_mode)
{
    /* Init compute_batch_statistics */
    int again_flag = TRUE;
    int ii;

    if ((cur_time - old_time) > EPSILON) {
        /* Transizione temporizzata */
        t_med_i += cur_time - old_time;
        for (ii = 0; ii < npl; ii++) {
            /* Raccolta statistiche */
            if (tabp[ii].tagged) {
                /* Statistiche specializzate */
                tok_i[ii] += net_mark[ii].total * (cur_time - old_time);
            }/* Statistiche specializzate */
        }/* Raccolta statistiche */
    }/* Transizione temporizzata */
    if (tabt[ev_type].tagged)
        fire_i[ev_type] += 1.0;
    if (end_batch(batch_length_mode))
        again_flag = check_results(approx, conf, file);
    return (again_flag);
}/* End compute_batch_statistics */
#endif
