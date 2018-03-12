#define ___PARSE___
#	include <ctype.h>
#	include "global.h"
#include "parse.h"

char *LTrim(char *string) {
    while (isspace(*string))
        string++;
    return string;
}

Boolean  ParseRate(char *toParse, float *retVal) {
    float temp;
    int retc;

    while (isspace(*toParse))
        toParse++;

    if (sscanf(toParse, "%f", &temp) != 1)
        return TRUE;
    if (temp < 0)
        return TRUE;
    *retVal = temp;
    puts("ParseRate OK ...");
    return FALSE;
}

Boolean ParseColor(char *toParse, char **retVal, Widget focusTo, Widget parent) {
    while (isspace(*toParse))
        toParse++;

    if (*toParse == '@') {
        ShowErrorDialog("A color label cannot start with '@' !", parent);
        XmProcessTraversal(focusTo, XmTRAVERSE_CURRENT);
        return TRUE;
    }
    if (*toParse == '\0')
        *retVal = NULL;
    else
        *retVal = toParse;
    return FALSE;
}

int  ParseMarkPar(char *toParse, struct mpar_object **Parsed, struct lisp_object **ParsedLisp) {
    struct mpar_object *mpar;
    struct lisp_object *lisp;

    while (isspace(*toParse))
        toParse++;

    if (!isalpha(*toParse))
        return BAD_RATE;


    for (mpar = netobj->mpars; mpar != NULL; mpar = mpar->next)
        if (strcmp(mpar->tag, toParse) == 0) {
            *Parsed = mpar;
            return GOOD_RATE;
        }

    for (lisp = netobj->lisps; lisp != NULL; lisp = lisp->next)
        if (strcmp(lisp->tag, toParse) == 0 && lisp->type == 'm') {
            *ParsedLisp = lisp;
            return GOOD_RATE;
        }
    return NO_SUCH_RATE;
}

int  ParseRatePar(char *toParse, struct rpar_object **Parsed) {
    struct rpar_object *rpar;

    while (isspace(*toParse))
        toParse++;

    if (!isalpha(*toParse))
        return BAD_RATE;

    for (rpar = netobj->rpars; rpar != NULL; rpar = rpar->next)
        if (strcmp(rpar->tag, toParse) == 0) {
            *Parsed = rpar;
            return GOOD_RATE;
        }
    return NO_SUCH_RATE;
}

Boolean ParseTag(char *toParse, char **Parsed, Widget focusTo, Widget parent) {
    char *begin;
    Boolean err = FALSE;
    char            Message[TAG_SIZE + 50];

    while (isspace(*toParse) && *toParse) 	/* Remove Trailing Blanks	*/
        toParse++;

    if (*toParse) {
        if (isalpha(*toParse)) {	/* the first char must be a letter */
            begin = toParse;		/* the string begins here	*/

            while (*toParse)		/* no blanks allowed in the tag	*/
                if (isspace(*toParse++)) {
                    err = TRUE;
                    sprintf(Message, "%s : No Blanks Allowed in Tag  ", begin);
                    break;
                }
        }
        else {
            err = TRUE;
            sprintf(Message, "Tag  %s  must begin with a letter", toParse);
        }

    }
    else {
        err = TRUE;
        strcpy(Message, "Empty string isn't a valid Tag");
    }
    if (err) {
        ShowErrorDialog(Message, parent);
        XmProcessTraversal(focusTo, XmTRAVERSE_CURRENT);
        return TRUE;
    }
    *Parsed = begin;
    return FALSE;
}

Boolean ParseRes(char *toParse, char **retVal, Widget focusTo, Widget parent) {
    while (isspace(*toParse))
        toParse++;

    if (*toParse == '\0')
        *retVal = NULL;
    else
        *retVal = toParse;
    return FALSE;
}

Boolean ParseTransTag(char *toParse, char **Parsed, Widget focusTo, Widget parent, struct trans_object *trans) {
    struct trans_object *last_trans;
    struct group_object *group;

    if (ParseTag(toParse, Parsed, focusTo, parent))
        return TRUE;

    for (group = netobj->groups; group != NULL; group = group->next)
        for (last_trans = group->trans; last_trans != NULL;
                last_trans = last_trans->next)
            if (strcmp(last_trans->tag, *Parsed) == 0 && last_trans != trans) {
                char            Message[TAG_SIZE + 50];

                sprintf(Message, "New Name : %s : must be unique.", *Parsed);
                ShowErrorDialog(Message, parent);
                XmProcessTraversal(focusTo, XmTRAVERSE_CURRENT);
                return TRUE;
            }
    return FALSE;
}


Boolean ParsePlaceTag(char *toParse, char **Parsed, Widget focusTo, Widget parent, struct place_object *place) {
    struct place_object *last_place;

    if (ParseTag(toParse, Parsed, focusTo, parent))
        return TRUE;

    for (last_place = netobj->places; last_place != NULL;
            last_place = last_place->next) {
        if ((strcmp(last_place->tag, *Parsed) == 0) && (last_place != place)) {
            char            Message[TAG_SIZE + 50];

            sprintf(Message, "New Name : %s : must be unique.", *Parsed);
            ShowErrorDialog(Message, parent);
            XmProcessTraversal(focusTo, XmTRAVERSE_CURRENT);
            return TRUE;
        }
    }
    return FALSE;
}

Boolean ParseMarkParTag(char *toParse, char **Parsed, Widget focusTo, Widget parent, struct mpar_object *mpar) {
    struct mpar_object *last_mpar;

    if (ParseTag(toParse, Parsed, focusTo, parent))
        return TRUE;

    for (last_mpar = netobj->mpars; last_mpar != NULL;
            last_mpar = last_mpar->next) {
        if ((strcmp(last_mpar->tag, *Parsed) == 0) && (last_mpar != mpar)) {
            char            Message[TAG_SIZE + 50];

            sprintf(Message, "New Name : %s : must be unique.", *Parsed);
            ShowErrorDialog(Message, parent);
            XmProcessTraversal(focusTo, XmTRAVERSE_CURRENT);
            return TRUE;
        }
    }
    return FALSE;
}

Boolean ParseColorTag(char *toParse, char **Parsed, Widget focusTo, Widget parent, struct lisp_object *lisp) {
    struct lisp_object *last_lisp;

    if (ParseTag(toParse, Parsed, focusTo, parent))
        return TRUE;

    for (last_lisp = netobj->lisps; last_lisp != NULL;
            last_lisp = last_lisp->next) {
        if ((strcmp(last_lisp->tag, *Parsed) == 0) && (last_lisp != lisp)) {
            char  Message[TAG_SIZE + 50];

            sprintf(Message, "New Name : %s : must be unique.", *Parsed);
            ShowErrorDialog(Message, parent);
            XmProcessTraversal(focusTo, XmTRAVERSE_CURRENT);
            return TRUE;
        }
    }
    return FALSE;
}

Boolean ParseNatural(char *toParse, int *retVal) {
    int temp;

    while (isspace(*toParse))
        toParse++;

    if (sscanf(toParse, "%d", &temp) != 1)
        return TRUE;
    if (temp < 0)
        return TRUE;

    *retVal = temp;
    return FALSE;
}


Boolean ParseRateParTag(char *toParse, char **Parsed, Widget focusTo, Widget parent, struct rpar_object *rpar) {
    struct rpar_object *last_rpar;

    if (ParseTag(toParse, Parsed, focusTo, parent))
        return TRUE;

    for (last_rpar = netobj->rpars; last_rpar != NULL;
            last_rpar = last_rpar->next) {
        if ((strcmp(last_rpar->tag, *Parsed) == 0) && (last_rpar != rpar)) {
            char            Message[TAG_SIZE + 50];

            sprintf(Message, "New Name : %s : must be unique.", *Parsed);
            ShowErrorDialog(Message, parent);
            XmProcessTraversal(focusTo, XmTRAVERSE_CURRENT);
            return TRUE;
        }
    }
    return FALSE;
}


Boolean ParseResTag(char *toParse, char **Parsed, Widget focusTo, Widget parent, struct res_object *res) {
    struct res_object *last_res;

    if (ParseTag(toParse, Parsed, focusTo, parent))
        return TRUE;

    for (last_res = netobj->results; last_res != NULL;
            last_res = last_res->next) {
        if ((strcmp(last_res->tag, *Parsed) == 0) && (last_res != res)) {
            char  Message[TAG_SIZE + 50];

            sprintf(Message, "New Name : %s : must be unique.", *Parsed);
            ShowErrorDialog(Message, parent);
            XmProcessTraversal(focusTo, XmTRAVERSE_CURRENT);
            return TRUE;
        }
    }
    return FALSE;
}

