//#include "../SUPPORT/lex.c"
void yyerror(char const *s) {
    fprintf(stdout, "Syntax error while parsing\n");
    switch (parsing_flag) {
    /* Tipo di errore */
    case TRANSITION :
        fprintf(stdout, "predicate of transition %s\n", TRANS_NAME(ntr));
        break;
    case PLACE :
        fprintf(stdout, "color domain of place %s\n", PLACE_NAME(npl));
        break;
    case MDRATE :
        fprintf(stdout, "MD rate definition of transition %s\n", TRANS_NAME(nmd));
        break;
    case FUNCTION :
        fprintf(stdout, "color function labeling arc connecting\n");
        fprintf(stdout, "place %s and transition %s\n", PLACE_NAME(pl_ind), TRANS_NAME(ntr));
        break;
    case COLOR_CLASS :
        fprintf(stdout, "color class definition %s\n", error_name);
        break;
    case STATIC_SUBCLASS :
        fprintf(stdout, "static subclass definition %s\n", error_name);
        break;
    case DYNAMIC_SUBCLASS :
        fprintf(stdout, "dynamic subclass definition %s\n", error_name);
        break;
    case INITIAL_MARKING :
        fprintf(stdout, "initial marking definition %s\n", error_name);
        break;
    default:
        break;
    }/* Tipo di errore */
    exit(1);
}
