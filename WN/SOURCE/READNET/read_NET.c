#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <assert.h>
#include <limits.h>
#include <errno.h>
#include "../../INCLUDE/const.h"
#include "../../INCLUDE/struct.h"
#include "../../INCLUDE/var_ext.h"
#include "../../INCLUDE/macros.h"

extern void lexer_set_buffer(const char *b);
extern int parser();
extern struct MD_RATE *get_md_rate();
extern void fill_transition_data_structure();
extern void out_error();
extern void *ecalloc();
extern char *emalloc();
extern FILE *efopen();

extern int has_repetitions();
extern int get_proj_num();

extern int read_domain[];  /* In wn.yac */


static void read_transition() ;
static void read_nested_units();

static FILE *net_fp = NULL;
// static FILE *parse_fp = NULL;
#ifdef LIBSPOT
static FILE *propA = NULL ;
#endif
static char *name_p = NULL;
// static char prs[MAXSTRING];


static int char_read;
static int skip_layer;
static int int_val;
static float xcoord1, ycoord1;

static int ntrSave = 0 ;

inline static int islinebreak(int ch) {
    return (ch == '\n' || ch == '\r');
}

/**************************************************************/

// separate the entity name from the list of tags (used by algebra)
// Added May, 2018.
int separate_name_tags(char* name_buf, char** p_tags) {
    char *tags = strchr(name_buf, '|'); // the pipe is the tags separator
    if (tags == NULL) { // no algebra tags
        *p_tags = NULL;
        return 0;
    }
    // split the tags from the name, and return a separate pointer
    *p_tags = tags + 1;
    *tags = '\0';
    return 1;
}


/**************************************************************/
/* NAME : Add a new arc to a transition */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
void parse_node(int type, int pos, int multiplicity, int place_ind, char *string_to_parse)
{
    /* Init parse_node */
    int comp, ii, jj, pl;
    char lex_buffer[MAXSTRING+100];

    Node_p node = NULL;
    Coeff_p func_ptr = NULL;

    if (place_ind <= 0 || place_ind > npl) { // place_ind is 1-based
        fprintf(stderr, "Place index %d of arc in .net file is out-of-bound.\n", place_ind);
        exit(EXIT_FAILURE);
    }

    switch (type) {
    /* Tipo di posti */
    case INPUT	   : node = tabt[ntr].inptr;
        break;
    case OUTPUT    : node = tabt[ntr].outptr;
        break;
    case INHIBITOR : node = tabt[ntr].inibptr;
        break;
    }/* Tipo di posti */

    for (ii = 1; ii < pos; node = NEXT_NODE(node), ii++);

    if (multiplicity >= 0)
        node->molt = multiplicity;  /* Init campo */
    else
        node->molt = -multiplicity;  /* Init campo */

    // 24/3/2015 Add support for marking parameters on transition arcs.
    if (node->molt >= 20000 && node->molt < 20000 + nmp) {
        // printf("Using marking parameter %s for arc multiplicity.\n", 
        //        tabmp[ node->molt - 20000 ].mark_name);
        node->molt = tabmp[ node->molt - 20000 ].mark_val;
    }

    node->analyzed = FALSE;     /* Init campo */

    node->place_no = pl = place_ind - 1; /* Init campo */

    switch (*(string_to_parse)) {
    /* Tipo di descrizione della funzione */
    case '@'  : {
        /************************************/
        /*  Da implementare adattando la	   */
        /*  grammatica affinche'riconosca   */
        /*  come color anche descrizioni    */
        /*  di funzioni riempiendo una ta   */
        /*  bella			   */
        /************************************/
        break;
    }
    case '\n' : {/* Nessuna funzione */
#ifdef SWN
        if (IS_NEUTRAL(pl)) {
            /* Posto con dominio neutro */
            node->arcfun = NULL;	   /* Init campo */
        }/* Posto con dominio neutro */
        else {
            /* Posto con dominio colorato */
            comp = GET_PLACE_COMPONENTS(pl);
            if (has_repetitions(pl)) {
                /* Transizione con dominio con ripetizioni */
                out_error(ERROR_REPETITIONS_AND_NO_FUNCTION, pl, ntr, 0, 0, NULL, NULL);
            }/* Transizione con dominio con ripetizioni */
            for (ii = 0; ii < comp; ii++)
                if (get_proj_num("?", tabp[pl].dominio[ii]) != 1) {
                    /* Transizione con dominio con ripetizioni */
                    out_error(ERROR_AMBIGUOUS_DEFINITION, pl, ntr, 0, 0, NULL, NULL);
                }/* Transizione con dominio con ripetizioni */
            func_ptr = (Coeff_p)emalloc(sizeof(struct ENN_COEFF));
            func_ptr->enn_coef = 1;
            func_ptr->card = 1;
            func_ptr->guard = NULL;
            func_ptr->next = NULL;
            func_ptr->coef = (struct COEFF *)ecalloc(comp, sizeof(struct COEFF));
            for (ii = 0; ii < comp; ii++) {
                /* Settaggio dei parametri dell'identita' */
                func_ptr->coef[ii].xsucc_coef = (int *)ecalloc(3, sizeof(int));
                func_ptr->coef[ii].xsucc_coef[0] = 1;
                func_ptr->coef[ii].xsucc_coef[1] = 0;
                func_ptr->coef[ii].xsucc_coef[2] = 0;
                func_ptr->coef[ii].sbc_coef = (int *)ecalloc(tabc[tabp[pl].dominio[ii]].sbc_num, sizeof(int));
                for (jj = 0; jj < tabc[tabp[pl].dominio[ii]].sbc_num ; jj++)
                    func_ptr->coef[ii].sbc_coef[jj] = 0;
            }/* Settaggio dei parametri dell'identita' */
            node->arcfun = func_ptr; /* Init campo */
        }/* Posto con dominio colorato */
#endif
        break;
        }/* Nessuna funzione */
    default  : {/* Arco con funzione */

        parse_DEF = FUNCTION;
        name_p = string_to_parse;
        sscanf(string_to_parse, "%d %f %f %n", &int_val, &xcoord1, &ycoord1, &char_read);
        name_p = string_to_parse + char_read;
        fun_ptr = NULL; /**** ARco con funz. in GSPN ********/
        pl_ind = node->place_no;  /* Init campo */

        // Initialize Lex buffer
        assert(strlen(name_p) <= MAXSTRING);
        sprintf(lex_buffer, "~f %s", name_p);
        lexer_set_buffer(lex_buffer);

        parser();
        lexer_set_buffer(NULL);

        if (fun_ptr != NULL)
            node->arcfun = fun_ptr;	 /* Init campo */
        break;
        }/* Arco con funzione */
    }/* Tipo di descrizione della funzione */
}/* End parse_node */
#ifdef SWN
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
int has_repetitions(p)
int p;
{
    /* Init has repetitions */
    int i, j;

    for (i = 0; i < tabp[p].comp_num; i++)
        for (j = 0; j < tabp[p].comp_num; j++)
            if (j != i)
                if (tabp[p].dominio[i] == tabp[p].dominio[j])
                    return (TRUE);
    return (FALSE);
}/* End has repetitions */
#endif
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
void init_nodes(type, num)
int type;
int num;
{
    /* Init init_nodes */
    int ii;
    Node_p place_list = NULL;
    Node_p node_ptr = NULL;

    if (num > 0) {
        /* Almeno un nodo */
        for (ii = 0; ii < num ; ii++) {
            /* Inizializzazione */
            node_ptr = (Node_p)emalloc(sizeof(struct NODO));
            node_ptr->arcfun = NULL;
            node_ptr->molt = UNKNOWN;
            node_ptr->fun_card = UNKNOWN;
            node_ptr->place_no = UNKNOWN;
            node_ptr->skip = UNKNOWN;
            node_ptr->analyzed = UNKNOWN;
            node_ptr->type = UNKNOWN;
            node_ptr->involved_in_CC_SC = FALSE;
            node_ptr->test_arc = FALSE;
            node_ptr->next = place_list;
            place_list = node_ptr;
        }/* Inizializzazione */
    }/* Almeno un nodo */
    else
        place_list = NULL;
    switch (type) {
    /* Tipo di posti */
    case INPUT	: tabt[ntr].inptr = place_list;
        break;
    case OUTPUT : tabt[ntr].outptr = place_list;
        break;
    case INHIBITOR : tabt[ntr].inibptr = place_list;
        break;
    }/* Tipo di posti */
}/* End init_nodes */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
void init_marking_parameters(num)
int num;
{
    /* Init init_marking_parameters */
    int i;

    for (i = 0; i < num; i++) {
        /* Per ogni elemento della tabella */
        tabmp[i].mark_name = NULL;
        tabmp[i].mark_val = UNKNOWN;
    }/* Per ogni elemento della tabella */
}/* End init_marking_parameters */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
void init_places(num)
int num;
{
    /* Init init_places */
    int i;

    for (i = 0; i < num; i++) {
        /* Per ogni elemento della tabella */
        tabp[i].place_name = NULL;
        tabp[i].algebra_tags = NULL;
#ifdef SWN
        tabp[i].dominio = NULL;
#endif
        tabp[i].comp_num = UNKNOWN;
        tabp[i].card = UNKNOWN;
        tabp[i].position = UNKNOWN;
#ifdef GREATSPN
        tabp[i].tagged = TRUE;
#else
        tabp[i].tagged = FALSE;
#endif
        tabp[i].unit = NULL;
    }/* Per ogni elemento della tabella */
}/* End init_places */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
void init_rate_parameters(num)
int num;
{
    /* Init init_rate_parameters */
    int i;

    for (i = 0; i < num; i++) {
        /* Per ogni elemento della tabella */
        tabrp[i].rate_name = NULL;
        tabrp[i].rate_val = UNKNOWN;
    }/* Per ogni elemento della tabella */
}/* End init_rate_parameters */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
void init_groups(num)
int num;
{
    /* Init init_groups */
    int i;

    for (i = 0; i < num; i++) {
        /* Per ogni elemento della tabella */
        tabg[i].name = NULL;
        tabg[i].priority = UNKNOWN;
    }/* Per ogni elemento della tabella */
}/* End init_groups */
/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
/* RETURN VALUE : */
/**************************************************************/
void init_transitions(num)
int num;
{
    /* Init init_transitions */
    int i;

    for (i = 0; i < num; i++) {
        /* Per ogni elemento della tabella */
        tabt[i].trans_name = NULL;
        tabt[i].algebra_tags = NULL;
#ifdef SWN
        tabt[i].names = NULL;
#endif

        tabt[i].in_arc = UNKNOWN;
        tabt[i].out_arc = UNKNOWN ;
        tabt[i].inib_arc = UNKNOWN ;

#ifdef GREATSPN
        tabt[i].tagged = TRUE;
#else
        tabt[i].tagged = FALSE;
#endif
        tabt[i].reset = FALSE;
#ifdef SWN
#ifdef SIMULATION
        tabt[i].skippable = UNKNOWN;
        tabt[i].en_deg = UNKNOWN;
#endif
#endif
        tabt[i].no_serv = UNKNOWN;
        tabt[i].pri = UNKNOWN ;
        tabt[i].timing = UNKNOWN;

#ifdef SWN
        tabt[i].comp_num = UNKNOWN;
        tabt[i].dominio = NULL;
        tabt[i].comp_type = NULL;
        tabt[i].rip = NULL;
        tabt[i].off = NULL;
#ifdef SYMBOLIC
#ifdef SIMULATION
        tabt[i].split_type = NULL;
        tabt[i].pre_split = UNKNOWN;
#endif
#endif
#endif
        tabt[i].add_l = NULL;
        tabt[i].test_l = NULL;
        tabt[i].me_l = NULL;

        tabt[i].inptr = NULL;
        tabt[i].outptr = NULL ;
        tabt[i].inibptr = NULL ;

        tabt[i].guard = NULL ;
        tabt[i].mean_t = UNKNOWN;

        tabt[i].md_rate_val = NULL;

#ifdef SIMULATION

        tabt[i].instances_h = NULL;
        tabt[i].instances_t = NULL;

        tabt[i].enabl_next = NULL;

#ifndef SYMBOLIC
        tabt[i].d_instances_h = NULL;
        tabt[i].d_instances_t = NULL;
        tabt[i].dist = NULL;
        tabt[i].stages = 0;
        tabt[i].policy = UNKNOWN;
        tabt[i].deschedule_policy = UNKNOWN;
        tabt[i].reuse_policy = UNKNOWN;
#endif
#endif
        tabt[i].general_function = NULL;
        tabt[i].rate_par_id=-1;
    }/* Per ogni elemento della tabella */
}/* End init_transitions */
/**************************************************************/


/**************************************************************/
/* Support for dynamically change the value of marking and
   rate parameters. */
/**************************************************************/
#define MAX_PARAMS_TO_CHANGE    32
struct {
    const char *mpar_name;
    int new_val;
} mpars_to_change[MAX_PARAMS_TO_CHANGE];
struct {
    const char *rpar_name;
    double new_val;
} rpars_to_change[MAX_PARAMS_TO_CHANGE];
static int num_mpars_to_change = 0;
static int num_rpars_to_change = 0;

void add_marking_param_to_change(const char *name, int new_val) {
    if (num_mpars_to_change > MAX_PARAMS_TO_CHANGE) {
        fprintf(stderr, "\nError: too many -mpar switches."
                "Raise the value of MAX_PARAMS_TO_CHANGE.\n");
        exit(1);
    }
    mpars_to_change[num_mpars_to_change].mpar_name = name;
    mpars_to_change[num_mpars_to_change].new_val = new_val;
    num_mpars_to_change++;
}
void add_rate_param_to_change(const char *name, double new_val) {
    if (num_rpars_to_change > MAX_PARAMS_TO_CHANGE) {
        fprintf(stderr, "\nError: too many -rpar switches."
                "Raise the value of MAX_PARAMS_TO_CHANGE.\n");
        exit(1);
    }
    rpars_to_change[num_rpars_to_change].rpar_name = name;
    rpars_to_change[num_rpars_to_change].new_val = new_val;
    num_rpars_to_change++;
}
void change_marking_param_value(const char *mpar_name, int *value) {
    int i;
    for (i = 0; i < num_mpars_to_change; i++) {
        if (0 == strcmp(mpar_name, mpars_to_change[i].mpar_name)) {
            *value = mpars_to_change[i].new_val;
            return;
        }
    }
}
void change_rate_param_value(const char *rpar_name, double *value) {
    int i;
    for (i = 0; i < num_rpars_to_change; i++) {
        if (0 == strcmp(rpar_name, rpars_to_change[i].rpar_name)) {
            *value = rpars_to_change[i].new_val;
            return;
        }
    }
}
void check_param_to_change_existance() {
    int i, j;
    // verify that all marking/rate parameters whose value has been
    // changed on-the-fly are actually existing parameters
    for (i = 0; i < num_mpars_to_change; i++) {
        for (j = 0; j < nmp; j++) {
            if (0 == strcmp(tabmp[j].mark_name, mpars_to_change[i].mpar_name))
                break;
        }
        if (j == nmp) { // not found
            fprintf(stderr, "Error: marking parameter \"%s\" does not exists.\n",
                    mpars_to_change[i].mpar_name);
            exit(1);
        }
    }
    for (i = 0; i < num_rpars_to_change; i++) {
        for (j = 0; j < nrp; j++) {
            if (0 == strcmp(tabrp[j].rate_name, rpars_to_change[i].rpar_name))
                break;
        }
        if (j == nrp) { // not found
            fprintf(stderr, "Error: rate parameter \"%s\" does not exists.\n",
                    rpars_to_change[i].rpar_name);
            exit(1);
        }
    }
    // verify that all marking/rate parameters that should have been
    // set with the -mpar / -rpar parameter switches have been so
    int quit = 0;
    for (j = 0; j < nmp; j++) {
        if (tabmp[j].mark_val == -7134) {
            fprintf(stderr, "Error: marking parameter \"%s\" needs a value. Specify it with -mpar %s <value>\n",
                    tabmp[j].mark_name, tabmp[j].mark_name);
            quit = 1;
        }
    }
    for (j = 0; j < nrp; j++) {
        if (tabrp[j].rate_val == -7134) {
            fprintf(stderr, "Error: rate parameter \"%s\" needs a value. Specify it with -rpar %s <value>\n",
                    tabrp[j].rate_name, tabrp[j].rate_name);
            quit = 1;
        }
    }
    if (quit)
        exit(1);
}

/**************************************************************/
// Pre-read a .net file to discover the marking parameters names.
// We need to know the mpar names in advance, because the .def
// file is parsed before the .net, and color class definitions
// could be defined using marking parameters.
/**************************************************************/

static int num_preread_mpars;
static struct MARK_PAR* preread_mpars;

void pre_read_NET_file() {
    int m;
    char line[MAXSTRING], name[MAXSTRING];

    sprintf(line, "%snet", net_name);
    FILE *net = efopen(line, "r");

    while (TRUE) {
        if (fgets(line, MAXSTRING - 1, net) == NULL)
            if (feof(net))
                break;
        if (line[0] == '|' && islinebreak(line[1])) { // next line is the header
            if (NULL == fgets(line, MAXSTRING - 1, net)) { // read marking parameter line
                fprintf(stderr, "Missing header in .net file.\n");
                exit(1);
            }
            sscanf(line, "f %d", &num_preread_mpars);
            break;
        }
    }
    // Read the marking parameters
    if (num_preread_mpars > 0) {
        preread_mpars = (struct MARK_PAR*)ecalloc(num_preread_mpars, sizeof(struct MARK_PAR));
        for (m=0; m<num_preread_mpars; m++) {
            if (NULL == fgets(line, MAXSTRING - 1, net)) { // read marking parameter line
                fprintf(stderr, "Missing expected marking parameter in .net file.\n");
                exit(1);
            }
            sscanf(line, "%s %d", name, &preread_mpars[m].mark_val); // extract name
            preread_mpars[m].mark_name = ecalloc(strlen(name)+1, sizeof(char));
            strcpy(preread_mpars[m].mark_name, name);
            change_marking_param_value(preread_mpars[m].mark_name, 
                                       &preread_mpars[m].mark_val);
        }
    }
    fclose(net);
}

/**************************************************************/
// Check if a given string is a marking or a rate parameter.
/**************************************************************/

int get_mpar_value(const char* name, int* value) {
    int m;
    for (m=0; m<nmp; m++) {
        if (0 == strcmp(tabmp[m].mark_name, name)) {
            *value = tabmp[m].mark_val;
            return 1;
        }
    }
    if (nmp == 0 && tabmp == NULL/* && !parse_DEF*/) {
        // .net file has not yet been read, but we have pre-read the marking 
        // parameter names, so it is possible to distinguish the token values.
        for (m=0; m<num_preread_mpars; m++) {
            if (0 == strcmp(preread_mpars[m].mark_name, name)) {
                *value = preread_mpars[m].mark_val;
                return 1;
            }
        }
    }
    return 0;
}

int get_rpar_value(const char* name, double* value) {
    int r;
    for (r=0; r<nrp; r++) {
        if (0 == strcmp(tabrp[r].rate_name, name)) {
            *value = tabrp[r].rate_val;
            return 1;
        }
    }
    return 0;
}

/**************************************************************/

// Convert a string into an int ensuring that no overflows occurs
int atoi_checked(const char* buffer) {
    char *endptr;
    errno = 0; // to distinguish errors
    long int value = strtol(buffer, &endptr, 10);

    if (errno != 0) {
        fprintf(stderr, "Value '%s' is not a valid integer.\n", buffer);
        exit(EXIT_FAILURE);
    }
    if (value > INT_MAX || value < INT_MIN) {
        fprintf(stderr, "Value '%s' cannot be stored as an int.\n", buffer);
        exit(EXIT_FAILURE);
    }

    return value;
}

/**************************************************************/
/* NAME : */
/* DESCRIPTION : */
/* PARAMETERS : */
//   read_postproc : should we read the .net directly (FALSE),
//     or should we read the post-processed .par file (TRUE)?
/* RETURN VALUE : */
/**************************************************************/
void read_NET_file(int read_postproc) {
    char tmp[MAXSTRING], *algebra_tags;
    char trash[MAXSTRING];
    char lex_buffer[MAXSTRING+100];
    char int_val_buffer[128];
    char type;
    int item, item_skip, has_tags;
    float xcoord2, ycoord2;
    double float_val;

    if (preread_mpars) { // remove pre-read marking parameters
        free(preread_mpars);
        preread_mpars = NULL;
        num_preread_mpars = 0;
    }

    sprintf(tmp, "%s%s", net_name, (read_postproc ? "par" : "net"));

    net_fp = efopen(tmp, "r");
    while (TRUE) {
        /* Scanning .def file */
        if (fgets(tmp, MAXSTRING - 1, net_fp) == NULL)
            if (feof(net_fp))
                break;
        if (tmp[0] == '|' && islinebreak(tmp[1])) {
            /* Parsing della prima riga */
            if (0 == fscanf(net_fp, "%c %d %d %d %d %d %d", &type, &el[0], &el[1], &el[2], &el[4], &el[3], &el[5])) {
                fprintf(stderr, "Missing header in .net file.\n");
                exit(1);
            }
            ntrSave = el[4];
#ifdef LIBSPOT
#ifndef LIBDMC
#ifndef ESYMBOLIC
            char tmp2 [256];
            sprintf(tmp2, "%stobs", net_name);
            propA = efopen(tmp2, "r");
            if (0 == fscanf(propA, "%d\n", &nSpottr)) {
                fprintf(stderr, "Missing expected SPOT_TR parameter.\n");
                exit(1);
            }
            el[4] = el[4] + nSpottr;
#endif
#endif
#endif
            if (el[4] > MAX_TRANS)
                out_error(ERROR_LOW_MAX_TRANS, 0, 0, 0, 0, NULL, NULL);
            while (getc(net_fp) != '\n');
            break;
        }/* Parsing della prima riga */
    }/* Scanning .def file */
    // Basic sanity checks
    if (el[1] == 0) { // no places
        fprintf(stderr, "Net file has 0 places. Stop.\n");
        exit(1);
    }
    if (el[4] == 0) { // no transitions
        fprintf(stderr, "Net file has 0 transitions. Stop.\n");
        exit(1);
    }
    /*********** MARKING PARAMETERS ***********/
    if (el[0] > 0) {
        /* Ci sono marking parameters */
        tabmp = (struct MARK_PAR *)ecalloc(el[0], sizeof(struct MARK_PAR));
        init_marking_parameters(el[0]);
        for (item = 1; item <= el[0]; item++) {
            /* Lettura dei Marking Parameters */
            if (NULL == fgets(tmp, MAXSTRING - 1, net_fp)) {
                fprintf(stderr, "Missing expected marking parameter definition in .net file.\n");
                exit(1);
            }

            sscanf(tmp, "%s %s %f %f %s", read_name, int_val_buffer, &xcoord1, &ycoord1, trash);
            tabmp[nmp].mark_name = (char *)ecalloc(strlen(read_name) + 1, sizeof(char));
            strcpy(tabmp[nmp].mark_name, read_name);
            int_val = atoi_checked(int_val_buffer);
            tabmp[nmp].mark_val = int_val;
            change_marking_param_value(tabmp[nmp].mark_name, &tabmp[nmp].mark_val);
            nmp++;
        }/* Lettura dei Marking Parameters */
    }/* Ci sono marking parameters */
    /*********** PLACES ***********/
    if (el[1] > 0) {
        /* Ci sono posti */
        tabp = (struct PLACES *)ecalloc(el[1], sizeof(struct PLACES));
        init_places(el[1]);
        for (item = 1; item <= el[1]; item++) {
            /* Lettura dei Posti */
            if (NULL == fgets(tmp, MAXSTRING - 1, net_fp)) {
                fprintf(stderr, "Missing expected place definition in .net file.\n");
                exit(1);
            }

            sscanf(tmp, "%s %s %f %f %f %f %n", read_name, int_val_buffer, &xcoord1, &ycoord1, &xcoord2, &ycoord2, &char_read);
            // separate place name from algebra tags
            has_tags = separate_name_tags(read_name, &algebra_tags);
            tabp[npl].place_name = (char *)ecalloc(strlen(read_name) + 1, sizeof(char));
            strcpy(tabp[npl].place_name, read_name);
            if (has_tags) {
                tabp[npl].algebra_tags = (char *)ecalloc(strlen(algebra_tags) + 1, sizeof(char));
                strcpy(tabp[npl].algebra_tags, algebra_tags);
            }
            // printf("place '%s' tags='%s'\n", tabp[npl].place_name, tabp[npl].algebra_tags);
            int_val = atoi_checked(int_val_buffer);

            name_p = tmp + char_read ;
            do {
                sscanf(name_p, "%d%n", &skip_layer, &char_read);
                name_p += char_read;
            }
            while (skip_layer);


            switch (*(name_p)) {
            /* Tipo di dominio del posto */
            case '@'  : {
                /************************************/
                /*  Da implementare adattando la	  */
                /*  grammatica affinche'riconosca    */
                /*  come color anche descrizioni     */
                /*  di domini riempiendo una tabella */
                /************************************/
                break;
            }
            case '\n' : {/* Dominio neutro */
                tabp[npl].comp_num = 0;
                break;
                }/* Dominio neutro*/
            default   : {/*Dominio colorato da parsificare*/
                parse_DEF = PLACE;
                while (!isalpha(*name_p))
                    name_p++;
                
                // Initialize Lex buffer
                assert(strlen(name_p) <= MAXSTRING);
                sprintf(lex_buffer, "~w %s", name_p);
                lexer_set_buffer(lex_buffer);

                parser();
                lexer_set_buffer(NULL);
#ifdef SWN
                tabp[npl].dominio = (int *)ecalloc(tabp[npl].comp_num, sizeof(int));
                for (item_skip = 0; item_skip < tabp[npl].comp_num ; item_skip++)
                    tabp[npl].dominio[item_skip] = read_domain[item_skip];
#endif
                }/*Dominio colorato da parsificare*/
            }/* Tipo di dominio del posto */
#ifdef SWN
            for (item_skip = 0, tabp[npl].card = 1; item_skip < tabp[npl].comp_num; item_skip++)
                tabp[npl].card *= tabc[tabp[npl].dominio[item_skip]].card;
#endif
            tabp[npl].position = int_val;
            npl++;
        }/* Lettura dei Posti */
    }/* Ci sono posti */
    /*********** RATE PARAMETERS ***********/
    if (el[2] > 0) {
        /* Ci sono rate parameters */
        tabrp = (struct RATE_PAR *)ecalloc(el[2], sizeof(struct RATE_PAR));
        init_rate_parameters(el[2]);
        for (item = 1; item <= el[2]; item++) {
            /* Lettura dei Rate Parameters */
            if (NULL == fgets(tmp, MAXSTRING - 1, net_fp)) {
                fprintf(stderr, "Missing expected rate parameter definition in .net file.\n");
                exit(1);
            }

            sscanf(tmp, "%s %lg %f %f %s", read_name, &float_val, &xcoord1, &ycoord1, trash);
            tabrp[nrp].rate_name = (char *)ecalloc(strlen(read_name) + 1, sizeof(char));
            strcpy(tabrp[nrp].rate_name, read_name);
            tabrp[nrp].rate_val = float_val;
            change_rate_param_value(tabrp[nrp].rate_name, &tabrp[nrp].rate_val);
            nrp++;
        }/* Lettura dei Rate Parameters */
    }/* Ci sono rate parameters */
    check_param_to_change_existance();
    /*********** PRIORITY GROUPS ***********/
    if (el[3] > 0) {
        /* Ci sono groups */
        tabg = (struct GROUP *)ecalloc(el[3], sizeof(struct GROUP));
        init_groups(el[3]);
        for (item = 1; item <= el[3]; item++) {
            /* Lettura dei Groups */
            if (NULL == fgets(tmp, MAXSTRING - 1, net_fp)) {
                fprintf(stderr, "Missing expected group definition in .net file.\n");
                exit(1);
            }

            sscanf(tmp, "%s %f %f %d", read_name, &xcoord1, &ycoord1, &int_val);
            tabg[ngr].name = (char *)ecalloc(strlen(read_name) + 1, sizeof(char));
            strcpy(tabg[ngr].name, read_name);
            tabg[ngr++].priority = int_val;
        }/* Lettura dei Groups */
    }/* Ci sono groups */
    /*********** TRANSITIONS ***********/
    if (el[4] > 0) {
        /* Ci sono transizioni */
        tabt = (Trans_p)ecalloc(el[4], sizeof(struct TRANS));

#ifdef SWN
        occ = (struct FUNBUF **)ecalloc(el[4], sizeof(struct FUNBUF *));
        for (item = 0; item < el[4]; item++)
            occ[item] = (struct FUNBUF *)ecalloc(MAX_DOMAIN, sizeof(struct FUNBUF));
#endif

        init_transitions(el[4]);

        for (item = 1; item <= ntrSave; item++) {
            read_transition();
        }


#ifdef LIBSPOT
#ifndef LIBDMC
#ifndef ESYMBOLIC
        fclose(net_fp);
        net_fp = propA;
        for (item = 1; item <= nSpottr ; item++) {
            read_transition();
        }
        ntr = ntrSave;
#endif
#endif
#endif
    }/* Ci sono transizioni */
    fclose(net_fp);
    /*system("rm nets/parse_temp");*/

    // Read Nested Units (NuPN support)
    read_nested_units();
}/* End read_NET_file */


/**************************************************************/
// Read transition entries from .net file
/**************************************************************/

void read_transition() {

    char tmp [MAXSTRING], *algebra_tags;
    char lex_buffer[MAXSTRING+100];
    char int_val_buffer[128];
    int int_val1, int_val2, int_val3, int_val4;
    float xcoord2, ycoord2;
    float xcoord3, ycoord3;
    int item_arc, item_skip, has_tags;
    int rte;
    float float_val;

    /* Lettura delle transizioni */
    if (NULL == fgets(tmp, MAXSTRING - 1, net_fp)) {
        fprintf(stderr, "Missing expected transition definition in .net file.\n");
        exit(1);
    }

    sscanf(tmp, "%s %f %d %d %d %d %f %f %f %f %f %f %n",
           read_name,
           &float_val,
           &int_val1, &int_val2, &int_val3, &int_val4,
           &xcoord1, &ycoord1, &xcoord2, &ycoord2, &xcoord3, &ycoord3,
           &char_read);
    // separate transition name from algebra tags
    has_tags = separate_name_tags(read_name, &algebra_tags);
    tabt[ntr].trans_name = (char *)ecalloc(strlen(read_name) + 1, sizeof(char));
    strcpy(tabt[ntr].trans_name, read_name);
    if (has_tags) {
        tabt[ntr].algebra_tags = (char *)ecalloc(strlen(algebra_tags) + 1, sizeof(char));
        strcpy(tabt[ntr].algebra_tags, algebra_tags);        
    }
    // printf("transition '%s' tags='%s'\n", tabt[ntr].trans_name, tabt[ntr].algebra_tags);


#ifdef ESYMBOLIC
    /***************** For_ESRG ************/
    tabt[ntr].trans_type = SYM_T;
    /***************************************/
#endif

    // Decode rate/delay
    if (float_val == -510) // marking-dependent rate
    { /*tabt[ntr].md_rate_val=get_md_rate(ntr);*/
        if (int_val2 == 127) { // it is a general transition (black rectangle in the gui)
            // has_general_fn = TRUE;
            tabt[ntr].mean_t = 0.0;
        }
    }
    else if (float_val < 0) { // it is a rate parameter
        rte = -float_val - 1;
        tabt[ntr].mean_t = tabrp[rte].rate_val;
        tabt[ntr].rate_par_id=rte;
    }
    else // float_val is the transition rate (EXP)  or delay (DET)
        tabt[ntr].mean_t = float_val;
    tabt[ntr].pri = int_val2;

    // Decode transition timing
    if (tabt[ntr].pri == 0)
        tabt[ntr].timing = TIMING_EXPONENTIAL;
    else if (int_val2 >= 127) { // general distribution
        tabt[ntr].timing = TIMING_DETERMINISTIC;
#ifdef SIMULATION
#ifndef SYMBOLIC
        // printf("Making %s a general transition.\n", tabt[ntr].trans_name);
        tabt[ntr].policy = ENABLING_M;
        tabt[ntr].reuse_policy = POLICY_DIS_RANDOM;
        tabt[ntr].deschedule_policy = POLICY_DIS_RANDOM;
        tabt[ntr].stages = 0;
        tabt[ntr].pri = 0; // make this a timed transition
#endif
#endif 
    }
    else
        tabt[ntr].timing = TIMING_IMMEDIATE;

    tabt[ntr].no_serv = int_val1;
    if (int_val1 == INFINITE_SERVER)
        tabt[ntr].no_serv = 0;
    tabt[ntr].in_arc = int_val3;
    comp_check = 0;

    name_p = tmp + char_read;
    do {
        sscanf(name_p, "%d%n", &skip_layer, &char_read);
        name_p += char_read;
    }
    while (skip_layer);

    // Skip trailing spaces (if any)
    while (isspace(*name_p))
        name_p++;

    switch (*(name_p)) {
        /* Tipo di dominio della transizione */
        case '@'  : {
            /************************************/
            /*  Da implementare adattando la	*/
            /*  grammatica affinche'riconosca   */
            /*  come color anche descrizioni    */
            /*  di domini riempiendo una tabella*/
            /************************************/
            break;
        }

        case '\0' :
        case '\n' : {/* Dominio neutro*/
            break;
        }/* Dominio neutro*/

        default   : {/*Dominio colorato da parsificare*/
            parse_DEF = TRANSITION;
            while ((*name_p) != '[' && (*name_p) != '#')
                name_p++;
            
            // Initialize Lex buffer
            assert(strlen(name_p) <= MAXSTRING);
            sprintf(lex_buffer, "~k %s", name_p);
            lexer_set_buffer(lex_buffer);

            parser();
            lexer_set_buffer(NULL);
        }/*Dominio colorato da parsificare*/
    }/* Tipo di dominio della transizione */


    /*********** INPUT ARCS *************/
    input_flag = FALSE;
    init_nodes(INPUT, int_val3);
    if (tabt[ntr].in_arc != 0) {
        /* Ci sono archi di ingresso */
        input_flag = TRUE;
        for (item_arc = 1; item_arc <= tabt[ntr].in_arc; item_arc++) {
            /* Lettura di ogni arco di ingresso */
            if (NULL == fgets(tmp, MAXSTRING - 1, net_fp)) {
                fprintf(stderr, "Missing expected input arc definition in .net file.\n");
                exit(1);
            }

            sscanf(tmp, "%s %d %d %n", // multiplicity  place  num_points  [layers]
                   int_val_buffer, &int_val2, &int_val3,
                   &char_read);
            name_p = tmp + char_read;
            int_val1 = atoi_checked(int_val_buffer);
            do {
                sscanf(name_p, "%d%n", &skip_layer, &char_read);
                name_p += char_read;
            }
            while (skip_layer);
            parse_node(INPUT, item_arc, int_val1, int_val2, name_p);
            for (item_skip = 1; item_skip <= int_val3; item_skip++) {
                if (NULL == fgets(tmp, MAXSTRING - 1, net_fp)) { }
            }
        }/* Lettura di ogni arco di ingresso */
    }/* Ci sono archi di ingresso */
    /*********** OUTPUT ARCS *************/
    if (NULL == fgets(tmp, MAXSTRING - 1, net_fp)) {
        fprintf(stderr, "Missing expected output arc definitions header in .net file.\n");
        exit(1);
    }
    sscanf(tmp, "%d", &int_val3);

    input_flag = FALSE;
    tabt[ntr].out_arc = int_val3;
    init_nodes(OUTPUT, int_val3);
    if (tabt[ntr].out_arc != 0) {
        /* Ci sono archi di uscita */
        for (item_arc = 1; item_arc <= tabt[ntr].out_arc; item_arc++) {
            /* Lettura di ogni arco di ingresso */
            if (NULL == fgets(tmp, MAXSTRING - 1, net_fp)) {
                fprintf(stderr, "Missing expected output arc definition in .net file.\n");
                exit(1);
            }

            sscanf(tmp, "%s %d %d %n",
                   int_val_buffer, &int_val2, &int_val3,
                   &char_read);
            name_p = tmp + char_read;
            int_val1 = atoi_checked(int_val_buffer);
            do {
                sscanf(name_p, "%d%n", &skip_layer, &char_read);
                name_p += char_read;
            }
            while (skip_layer);
            parse_node(OUTPUT, item_arc, int_val1, int_val2, name_p);
            for (item_skip = 1; item_skip <= int_val3; item_skip++) {
                if (NULL == fgets(tmp, MAXSTRING - 1, net_fp)) { }
            }
        }/* Lettura di ogni arco di ingresso */
    }/* Ci sono archi di uscita */
    /*********** INHIBITOR ARCS *************/
    if (NULL == fgets(tmp, MAXSTRING - 1, net_fp)) {
        fprintf(stderr, "Missing expected inhibitor arc definitions header in .net file.\n");
        exit(1);
    }
    sscanf(tmp, "%d", &int_val3);

    input_flag = FALSE;
    tabt[ntr].inib_arc = int_val3;
    init_nodes(INHIBITOR, int_val3);
    if (tabt[ntr].inib_arc != 0) {
        /* Ci sono archi inibitori */
        for (item_arc = 1; item_arc <= tabt[ntr].inib_arc; item_arc++) {
            /* Lettura di ogni arco di ingresso */
            if (NULL == fgets(tmp, MAXSTRING - 1, net_fp)) {
                fprintf(stderr, "Missing expected inhibitor arc definition in .net file.\n");
                exit(1);
            }

            sscanf(tmp, "%s %d %d %n",
                   int_val_buffer, &int_val2, &int_val3,
                   &char_read);
            name_p = tmp + char_read ;
            int_val1 = atoi_checked(int_val_buffer);
            do {
                sscanf(name_p, "%d%n", &skip_layer, &char_read);
                name_p += char_read;
            }
            while (skip_layer);
            parse_node(INHIBITOR, item_arc, int_val1, int_val2, name_p);
            for (item_skip = 1; item_skip <= int_val3; item_skip++) {
                if (NULL == fgets(tmp, MAXSTRING - 1, net_fp)) { }
            }
        }/* Lettura di ogni arco di ingresso */
    }/* Ci sono archi inibitori */
    fill_transition_data_structure(ntr);
#ifdef LIBSPOT
#ifdef ESYMBOLIC
    tabt[ntr].dyn_guard = NULL;
#endif
#endif

    ntr++;
}/* Lettura delle transizioni */



/**************************************************************/
// Read Nested Units, as exported by the PNML -> net/def converter
/**************************************************************/

static NESTED_UNIT*
read_nested_unit_recursively(FILE *nufp, NESTED_UNIT *parent) {
    char unit_name[MAXSTRING], ch;
    NESTED_UNIT *nu;
    int p, s;

    nu = (NESTED_UNIT*)emalloc(sizeof(NESTED_UNIT));
    memset(nu, 0, sizeof(*nu));
    nu->parent = parent;
    nu->id = num_nested_units++;
    nu_array[nu->id] = nu;

    // Read unit header - the space before "unit" in the format string is important!
    if (3 != fscanf(nufp, " unit %s subunits %d places %d", unit_name, 
                    &nu->num_subunits, &nu->num_places)) 
    {
        fprintf(stderr, "Error reading NUPN file.\n");
        exit(-1);
    }
    nu->name = strdup(unit_name);
    nu->places = (int*)emalloc(sizeof(int) * nu->num_places);
    // printf("Reading nested unit %s with %d places and %d subunits\n", 
    //         nu->name, nu->num_places, nu->num_subunits);

    // Read place indexes
    for (p=0; p<nu->num_places; p++) {
        if (1 != fscanf(nufp, "%d", &nu->places[p])) {
            fprintf(stderr, "Error reading place list in NUPN file.\n");
            exit(-1);
        }
        tabp[ nu->places[p] ].unit = nu;
    }

    // Read sub-units
    nu->subunits = (NESTED_UNIT**)emalloc(sizeof(NESTED_UNIT*) * nu->num_subunits);
    for (s=0; s<nu->num_subunits; s++)
        nu->subunits[s] = read_nested_unit_recursively(nufp, nu);
    return nu;
}

static void read_nested_units() {
    char tmp[MAXSTRING];
    FILE *nufp;
    int num_units;

    sprintf(tmp, "%snu", net_name);
    nufp = fopen(tmp, "r");
    if (nufp == NULL)
        return; // No Nested Units file

    if (0 == fscanf(nufp, "NUPN units %d", &num_units)) {
        fprintf(stderr, "Nested Unit file is broken.\n");
        exit(1);
    }
    num_nested_units = 0;
    nu_array = (NESTED_UNIT**)emalloc(sizeof(NESTED_UNIT*) * num_units);
    nu_root = read_nested_unit_recursively(nufp, NULL);
    fclose(nufp);
    if (num_units != num_nested_units) {
        fprintf(stderr, "NUPN has %d units, but we loaded %d units.\n", num_units, num_nested_units);
        exit(1);
    }
}








