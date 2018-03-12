

char *name_p = NULL;
extern void lexer_set_buffer(const char *b);
extern int parser();

void read_gard(FILE *gards_file) {

    char tmp [MAXSTRING] ;
    char read_name[MAXSTRING];
    char gard_read[MAXSTRING];
    char lex_buffer[MAXSTRING+100];
    int i;

    while (fgets(tmp, MAXSTRING - 1, gards_file)) {
        sscanf(tmp, "%s %s", read_name, gard_read);

        for (i = 0; i < ntr && strcmp(read_name, tabt[i].trans_name) ; i++);

        parse_DEF = TRANSITION;

        // Initialize Lex buffer
        assert(strlen(gard_read) <= MAXSTRING);
        sprintf(lex_buffer, "~k %s", gard_read);
        lexer_set_buffer(lex_buffer);

        parser();
        lexer_set_buffer(NULL);

//         parse_fp = efopen(prs, "w");
//         fprintf(parse_fp, "~k %s", gard_read);
//         fclose(parse_fp);
//         jjin = efopen(prs, "r");
// #ifdef BASTA_CON_LINUX
// #ifdef LINUX
//         jjout = stderr;
// #else
//         jjout = efopen("/dev/null", "w");
// #endif
// #endif
//         parser();
//         fclose(jjin);
// #ifdef BASTA_CON_LINUX
//         fclose(jjout);
// #endif
        tabt[i].dyn_gard = tabt[i].guard ;
        tabt[i].guard = NULL;
    }
}




