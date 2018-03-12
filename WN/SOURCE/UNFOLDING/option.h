enum {
    OPT_HELP = 0,
    OPT_USAGE,
    OPT_VERSION,
    OPT_UNF_NET_NAME,
    OPT_TR_IDX_NAME,
    OPT_PL_IDX_NAME,
    OPT_ORIZ_TRANS_DISPLACEMENT,
    OPT_VERT_TRANS_DISPLACEMENT,
    OPT_ORIZ_PLACE_DISPLACEMENT,
    OPT_VERT_PLACE_DISPLACEMENT,
};

typedef struct option_tag {
    const char *option;
    const char *name;
    const char *desc;
    enum {NONE = 0, MANDATORY, OPTIONAL} arg;
    enum {INTEGER, STRING} arg_type;
    const char *arg_val;
    unsigned found;
} OptionType;
typedef OptionType *OptionTypePTR;




