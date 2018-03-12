#include <Xm/Xm.h>
#include <Xm/Label.h>
#include <Xm/PushB.h>
#include <Xm/SelectioB.h>
#include <Xm/Text.h>
#include <Xm/TextF.h>
#include <Xm/Separator.h>
#include <Xm/Form.h>

#include <Xm/MessageB.h>
#include <Xm/BulletinB.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
extern int errno;

#include <wordexp.h>


XtAppContext context;
XmStringCharSet char_set = XmSTRING_DEFAULT_CHARSET;

Widget toplevel, label, textfield, sep, sep1, sep2, form, netLabel, epsLabel, psLabel, lprLabel,
       ps_textfield, eps_textfield, lpr_textfield, ok_button, cancel_button ;

Widget error_dialog;

static char *WarningMessage =
    "It seems the first time you are running \n the GreatSPN tool. To work properly it needs \n some information to be set. The .greatspn file \n is being created in your home directory to store it.\n If you desire to change the default setting please \n fill in the fields below.";


static char *dir_names[3];

void
OK_CB(w, client_data, call_data)
Widget w;
XtPointer client_data;
XmPushButtonCallbackStruct *call_data;
{
    FILE *out;
    char buf[100];
    char *s, *path;
    wordexp_t pwordexp;
    int i;

    dir_names[0] = (char *)XmTextGetString(textfield);
    dir_names[1] = (char *)XmTextGetString(eps_textfield);
    dir_names[2] = (char *)XmTextGetString(ps_textfield);



    for (i = 0; i < 3; i++) {
        path = dir_names[i];

        if (wordexp(path, &pwordexp, 0) != 0) {
            printf("Cannot expand %s directory name.\nPlease try  giving an absolute pathname.\n", path);
            exit(1);
        }
        else {
            char *c = "mkdir -m 755 -p ";
            char *str = (char *)malloc(sizeof(char) * (strlen(pwordexp.we_wordv[0]) + strlen(c) + 1));
            strcpy(str, c);
            strcat(str, pwordexp.we_wordv[0]);
            printf("%s\n", str);
            if (system(str/*mkdir(pwordexp.we_wordv[0],0755*/) == -1/*!=0*/ && errno != EEXIST) {
                printf("Cannot create the directory %s, errno: %d\n", pwordexp.we_wordv[0], errno);
                wordfree(&pwordexp);
                exit(1);
            }
            wordfree(&pwordexp);
        }
    }



    if ((s = getenv("HOME")) == NULL)
        exit(3);

    sprintf(buf, "%s/.greatspn", s);
    out = fopen(buf, "w");
    if (out == NULL)
        exit(2);

    fprintf(out, "GSPN_DEFAULT_PRINTER=\"%s\"\n", (char *)XmTextGetString(lpr_textfield));
    fprintf(out, "GSPN_NET_DIRECTORY=\"%s\"\n", dir_names[0]);
    fprintf(out, "GSPN_EPS_DIRECTORY=\"%s\"\n", dir_names[1]);
    fprintf(out, "GSPN_PS_DIRECTORY=\"%s\"\n",  dir_names[2]);

    fclose(out);

    exit(0);
}

void
CANCEL_CB(w, client_data, call_data)
Widget w;
XtPointer client_data;
XmPushButtonCallbackStruct *call_data;
{
    /*
       XtVaSetValues(error_dialog,
          XmNmessageString, XmStringCreateLtoR("ERROR", char_set),
          NULL);

        XtManageChild(error_dialog);
    */
    printf("cancel button pushed\n");
    exit(1);
}









int main(int argc, char *argv[]) {
    Dimension w, h;


    /* create the toplevel shell */
    toplevel = XtAppInitialize(&context, "", NULL, 0, &argc, argv, NULL, NULL, 0);

    /* declare variables and set the font */
    {
        XFontStruct *font = NULL;
        XmFontList fontlist = NULL;
        char *namestring = NULL;

        namestring = "-adobe-helvetica-medium-r-normal--14-140-75-75-p-77-iso8859-1";
        font = XLoadQueryFont(XtDisplay(toplevel), namestring);
        if (font != NULL) {
            fontlist = XmFontListCreate(font, XmSTRING_DEFAULT_CHARSET);
            XtVaSetValues(toplevel, XmNlabelFontList, fontlist, NULL);
            XtVaSetValues(toplevel, XmNbuttonFontList, fontlist, NULL);
        }
    }

    /* create the form widget */
    form = XtVaCreateManagedWidget("form",
                                   xmFormWidgetClass, toplevel,
                                   XmNmarginWidth, 20,
                                   XmNmarginHeight, 20,
                                   XmNfractionBase, 100,
                                   XmNnoResize, TRUE,
                                   NULL);




    /*         error_dialog=XmCreateErrorDialog(toplevel,"",NULL,0); */



    /* create label widget */
    label = XtVaCreateManagedWidget("",
                                    xmLabelWidgetClass, form,
                                    XmNtopAttachment, XmATTACH_FORM,
                                    XmNleftAttachment, XmATTACH_FORM,
                                    XmNrightAttachment, XmATTACH_FORM,
                                    XmNbackground, (Pixel)(128),
                                    NULL);

    XtVaSetValues(label,
                  XmNlabelString, XmStringCreateLtoR(WarningMessage, char_set),
                  XmNmarginLeft, 20,
                  XmNmarginRight, 20,
                  XmNmarginBottom, 25,
                  XmNmarginTop, 25,
                  NULL);



    /* create separator widget */
    sep = XtVaCreateManagedWidget("",
                                  xmSeparatorWidgetClass, form,
                                  XmNtopAttachment, XmATTACH_WIDGET,
                                  XmNtopWidget, label,
                                  XmNleftAttachment, XmATTACH_FORM,
                                  XmNrightAttachment, XmATTACH_FORM,
                                  XmNmarginBottom, 10,
                                  NULL);



    /* create textfield widget */
    lprLabel = XtVaCreateManagedWidget(
                   (String)"Default Printer",
                   xmLabelWidgetClass, form,
                   XmNtopAttachment, XmATTACH_WIDGET,
                   XmNtopWidget, sep,
                   XmNleftAttachment, XmATTACH_FORM,
                   XmNmarginLeft, 10,
                   XmNmarginRight, 10,
                   XmNalignment, XmALIGNMENT_END,
                   XmNtopOffset, 10,
                   NULL);


    lpr_textfield = XtVaCreateManagedWidget(
                        (String)"",
                        xmTextFieldWidgetClass, form,
                        XmNtopAttachment, XmATTACH_WIDGET, /* top, right to form */
                        XmNtopWidget, sep,
                        XmNleftAttachment, XmATTACH_WIDGET,
                        XmNleftWidget, lprLabel,
                        XmNtopOffset, 10,
                        NULL);
    XmTextSetString(lpr_textfield, "lpr");






    /* create separator widget */
    sep2 = XtVaCreateManagedWidget("",
                                   xmSeparatorWidgetClass, form,
                                   XmNtopAttachment, XmATTACH_WIDGET,
                                   XmNtopWidget, lpr_textfield,
                                   XmNleftAttachment, XmATTACH_FORM,
                                   XmNrightAttachment, XmATTACH_FORM,
                                   XmNmarginBottom, 10,
                                   XmNtopOffset, 10,
                                   NULL);





    /* create textfield widget */
    netLabel = XtVaCreateManagedWidget(
                   (String)"Nets Directory",
                   xmLabelWidgetClass, form,
                   XmNtopAttachment, XmATTACH_WIDGET,
                   XmNtopWidget, sep2,
                   XmNleftAttachment, XmATTACH_FORM,
                   XmNmarginLeft, 10,
                   XmNmarginRight, 10,
                   XmNalignment, XmALIGNMENT_END,
                   XmNtopOffset, 10,
                   NULL);


    textfield = XtVaCreateManagedWidget(
                    (String)"",
                    xmTextFieldWidgetClass, form,
                    XmNtopAttachment, XmATTACH_WIDGET, /* top, right to form */
                    XmNtopWidget, sep2,
                    XmNleftAttachment, XmATTACH_WIDGET,
                    XmNleftWidget, netLabel,
                    XmNtopOffset, 10,
                    NULL);
    XmTextSetString(textfield, "$HOME/nets");

    XtVaGetValues(textfield, XmNheight, &h, NULL);
    XtVaSetValues(netLabel, XmNheight, h, NULL);
    XtVaSetValues(lprLabel, XmNheight, h, NULL);




    /* create textfield widget */
    psLabel = XtVaCreateManagedWidget(
                  (String)"Postscripts Directory",
                  xmLabelWidgetClass, form,
                  XmNtopAttachment, XmATTACH_WIDGET,
                  XmNtopWidget, textfield,
                  XmNleftAttachment, XmATTACH_FORM,
                  XmNmarginLeft, 10,
                  XmNmarginRight, 10,
                  XmNheight, h,
                  XmNalignment, XmALIGNMENT_END,
                  NULL);
    XtVaGetValues(psLabel, XmNwidth, &w, NULL);

    ps_textfield = XtVaCreateManagedWidget(
                       (String)"",
                       xmTextFieldWidgetClass, form,
                       XmNtopAttachment, XmATTACH_WIDGET,
                       XmNtopWidget, textfield,
                       XmNleftAttachment, XmATTACH_WIDGET,
                       XmNleftWidget, psLabel,
                       NULL);
    XmTextSetString(ps_textfield, "$HOME/ps");






    /* create textfield widget */
    epsLabel = XtVaCreateManagedWidget(
                   (String)"Eps Directory",
                   xmLabelWidgetClass, form,
                   XmNtopAttachment, XmATTACH_WIDGET,
                   XmNtopWidget, ps_textfield,
                   XmNleftAttachment, XmATTACH_FORM,
                   XmNmarginLeft, 10,
                   XmNmarginRight, 10,
                   XmNheight, h,
                   XmNalignment, XmALIGNMENT_END,
                   NULL);


    eps_textfield = XtVaCreateManagedWidget(
                        (String)"",
                        xmTextFieldWidgetClass, form,
                        XmNtopAttachment, XmATTACH_WIDGET,
                        XmNtopWidget, ps_textfield,
                        XmNleftAttachment, XmATTACH_WIDGET,
                        XmNleftWidget, epsLabel,
                        NULL);
    XmTextSetString(eps_textfield, "$HOME/eps");



    XtVaSetValues(netLabel, XmNwidth, w, NULL);
    XtVaSetValues(epsLabel, XmNwidth, w, NULL);
    XtVaSetValues(lprLabel, XmNwidth, w, NULL);



    /* create separator widget */
    sep1 = XtVaCreateManagedWidget(
               (String)"",
               xmSeparatorWidgetClass, form,
               XmNtopAttachment, XmATTACH_WIDGET,
               XmNtopWidget, eps_textfield,
               XmNleftAttachment, XmATTACH_FORM,
               XmNrightAttachment, XmATTACH_FORM,
               XmNtopOffset, 10,
               NULL);



    /* create OK button widget */
    ok_button = XtVaCreateManagedWidget(
                    (String)"OK",
                    xmPushButtonWidgetClass, form,
                    XmNtopAttachment, XmATTACH_WIDGET,
                    XmNtopWidget, sep1,
                    XmNleftAttachment, XmATTACH_FORM,
                    XmNbottomAttachment, XmATTACH_FORM,
                    XmNleftOffset, 20,
                    XmNtopOffset, 20,
                    XmNheight, 30,
                    XmNwidth, 40,
                    NULL);

    XtAddCallback(ok_button, XmNactivateCallback, OK_CB, NULL);


    /* create CANCEL button widget */
    cancel_button = XtVaCreateManagedWidget(
                        (String)" CANCEL ",
                        xmPushButtonWidgetClass, form,
                        XmNtopAttachment, XmATTACH_WIDGET,
                        XmNtopWidget, sep1,
                        XmNrightAttachment, XmATTACH_FORM,
                        XmNbottomAttachment, XmATTACH_FORM,
                        XmNrightOffset, 20,
                        XmNtopOffset, 20,
                        NULL);

    XtAddCallback(cancel_button, XmNactivateCallback, CANCEL_CB, NULL);





    XtRealizeWidget(toplevel);
    XtAppMainLoop(context);

    return 0;
}

