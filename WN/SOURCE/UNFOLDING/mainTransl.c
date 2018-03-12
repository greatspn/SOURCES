/****** mainTransl.c ******/

#include<stdio.h>

#include "global.h"
#include "load.h"

#include <stdlib.h>
#include <ctype.h>
#include "util-transl.h"
#include "option.h"


OptionType opt_list[] = {
    {"h", "help", "this usage"},
    {"u", "usage", "this usage"},
    {"v", "version", "show version and exit"},
    {"un", "unf_net", "unfolded net name", OPTIONAL, STRING},
    {"tidx", "tidx", "", OPTIONAL, STRING},
    {"pidx", "pidx", "", OPTIONAL, STRING},
    {"otd", "otd", "transition displacement", OPTIONAL, INTEGER, "0"},
    {"vtd", "vtd", "vertical transition displacement", OPTIONAL, INTEGER, "0"},
    {"opd", "opd", "place displacement", OPTIONAL, INTEGER, "0"},
    {"vpd", "vpd", "vertical place displacement", OPTIONAL, INTEGER, "0"},
    {}
};

char *great_net = NULL;

static void usage(const char *prog_name) {
    OptionTypePTR opt;

    printf("Usage: %s [option...] net\n"
           "Where 'net' is name of net\n"
           "  and 'option' may be:\n", prog_name);
    for (opt = opt_list ; opt->name ; opt++) {
        int n = 0;

        if (opt->option)
            n = printf("  -%s, ", opt->option);
        else
            n = printf("      ");
        n += printf("--%s%s ", opt->name, opt->arg ? "=VAL" : "");
        n += printf("%*s%s", 24 - n, "", opt->desc);
        if (opt->arg) {
            n += printf(" (default `%s')", opt->arg_val);
        }
        printf("\n");
    }
    exit(1);
}


static void opt_parse(int argc, char *argv[]) {
    const char *prog_name;
    OptionTypePTR opt;
    char *p, *arg;
    int found, i;

    prog_name = argv[0];

    for (i = 1 ; i < argc ; i++) {
        p = argv[i];
        if (*p != '-') {
            if (great_net != NULL)
                usage(prog_name);
            great_net = p;
            continue;
        }
        found = 0;
        arg = NULL;
        do { p++; }
        while (*p == '-');
        for (opt = opt_list ; opt->name ; opt++) {
            if (!strncmp(p, opt->option, strlen(opt->name))) {
                char *q;
                if (!strncmp(p, opt->name, strlen(opt->name)))
                    q = p + strlen(opt->name);
                else
                    q = p + 1;
                if (*q == '\0')
                    found = 1;
                else if (*q == '=' && opt->arg) {
                    arg = q + 1;
                    found = 1;
                }
                else if (isdigit(*q) && opt->arg) {
                    arg = q;
                    found = 1;
                }
            }
            if (!found)
                continue;
            if (arg && !opt->arg)
                usage(prog_name);

            if (!arg && opt->arg && (i < argc - 1 && *argv[i + 1] != '-') &&
                    (!opt->arg_type == INTEGER || isdigit(*argv[i + 1])))
                arg = argv[++i];

            if (!arg && opt->arg == MANDATORY)
                usage(prog_name);
            opt->found++;
            if (opt->arg && arg)
                opt->arg_val = arg;
            break;
        }
        if (!found)
            usage(prog_name);
    }

}



void main_cmdline(int argc, char *argv[]) {
    const char *prog_name = argv[0];
    int val;

    opt_parse(argc, argv);

    if (opt_list[OPT_VERSION].found) {
        printf("Version: %d\n", VERSION);
        exit(0);
    }

    if (great_net == NULL || opt_list[OPT_HELP].found || opt_list[OPT_USAGE].found)
        usage(prog_name);

}




int main(int argc, char *argv[]) {
    char *unf_net, *transl_index;
    struct net_object *unf_netobj;

    main_cmdline(argc, argv);

    if (opt_list[OPT_UNF_NET_NAME].arg_val == NULL)
        unf_net = (char *) NewStringCat(great_net, "_unf");
    else
        unf_net = (char *) opt_list[OPT_UNF_NET_NAME].arg_val;

    if (opt_list[OPT_TR_IDX_NAME].arg_val == NULL)
        opt_list[OPT_TR_IDX_NAME].arg_val = (char *) NewStringCat(great_net, "_unf.idx");

    netobj = (struct net_object *)Emalloc(sizeof(struct net_object));
    unf_netobj = (struct net_object *)Emalloc(sizeof(struct net_object));
    read_file(great_net);

    printf("\nCompute unfolded net ...\n");

    unfold(netobj, unf_netobj);


    write_file(unf_net, unf_netobj);
    fflush(NULL);
    printf("\nNet %s written.\n", unf_net);
    return (0);
}





