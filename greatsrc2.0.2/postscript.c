#define ___POSTSCRIPT___
#	include "global.h"
#	include <time.h>
#include "postscript.h"
#include "GREAT_C_REVISION"

static FILE *fp;
Boolean PSflag;

static char *String2PSString(char *s) {
    static char t[512];
    register char *p = t;

    while (*s) {
        if (*s == '(' || *s == ')') {
            *p++ = '\\';
        }
        *p++ = *s++;
    }
    *p = *s;
    return t;
}

Boolean StartPS(char *filename, float xclip, float yclip, float wclip, float hclip,
                int offx, int offy, int height, int width, Boolean rotate, Boolean tex) {
    char temp[300];
    time_t tloc;

    time(&tloc);
    strcpy(temp, ctime(&tloc));
    temp[24] = '\0';

    fp = fopen(filename, "w");
    if (fp == NULL) {
        puts("Impossibile creare file PS !!!!!");
        return TRUE;
    }

    /* postscript has images reverse from real world */

    fputs(tex ? "%!\n" : "%!PS-Adobe-2.0\n", fp);
    if (rotate)
        fprintf(fp, "%%%%BoundingBox: %d %d %d %d\n", (int)(595. - ((offy + height)) / .351), (int)(842. - ((offx + width) / 0.351)),
                (int)(595. - (((float)offy) / .351)), (int)(842. - (offx / .351)));
    else
        fprintf(fp, "%%%%BoundingBox: %d %d %d %d\n", (int)(((float)offx) / .351), (int)(842. - ((offy + height) / 0.351)),
                (int)((offx + width) / .351), (int)(842. - (offy / .351)));
    fprintf(fp, "%%%%CreationDate: %s\n", temp);
    fprintf(fp, "%%Creator: GreatSPN%s\n", PROGRAM_REVISION);

    fprintf(fp, "%%%%For: %s\n", getenv("USER"));

    fputs("%%Pages: 1\n", fp);

    fputs(rotate ? "%%Orientation: Landscape\n" : "%%Orientation: Portrait\n", fp);
    fputs("%%DocumentFonts: (atend)\n", fp);
    fputs("%%EndComments\n", fp);
    fputs("%%BeginProlog\n\n", fp);
    fputs("/arcdict 9 dict def\n", fp);
    fputs("/placedict 3 dict def\n", fp);
    fputs("/transdict 5 dict def\n", fp);
    fputs("/linedict 4 dict def\n", fp);
    fputs("/stringdict 3 dict def\n", fp);
    fputs("/pgreekdict 2 dict def\n\n", fp);

    fputs("/a { arcdict begin\n", fp);
    fputs("     gsave newpath\n", fp);
    fputs("     /y2 exch def\n", fp);
    fputs("     /x2 exch def\n", fp);
    fputs("     /y1 exch def\n", fp);
    fputs("     /x1 exch def\n", fp);
    fputs("     /yc exch def\n", fp);
    fputs("     /xc exch def\n", fp);
    fputs("     /r x1 xc sub dup mul y1 yc sub dup mul add sqrt\n", fp);
    fputs("     x2 xc sub dup mul y2 yc sub dup mul add sqrt add 2 div def\n", fp);
    fputs("     /ang1 y1 yc sub x1 xc sub atan def\n", fp);
    fputs("     /ang2 y2 yc sub x2 xc sub atan def\n", fp);
    fputs("     xc yc r ang1 ang2 arc stroke grestore end\n", fp);
    fputs("   } def\n\n", fp);

    fputs("/p { placedict begin \n", fp);
    fputs("     /radius exch def\n", fp);
    fputs("     /y exch def\n", fp);
    fputs("     /x exch def\n", fp);
    fputs("     newpath x y radius 0 360 arc stroke end\n", fp);
    fputs("   } def\n\n", fp);

    fputs("/t { placedict begin \n", fp);
    fputs("     /radius exch def\n", fp);
    fputs("     /y exch def\n", fp);
    fputs("     /x exch def\n", fp);
    fputs("     newpath x y radius 0 360 arc fill end\n", fp);
    fputs("   } def\n\n", fp);

    fputs("/k { transdict begin\n", fp);
    fputs("     /r exch def\n", fp);
    fputs("     /h exch def\n", fp);
    fputs("     /w exch def\n", fp);
    fputs("     /y exch def\n", fp);
    fputs("     /x exch def\n", fp);
    fputs("     gsave\n", fp);
    fputs("     x y translate\n", fp);
    fputs("     r rotate\n", fp);
    fputs("     newpath \n", fp);
    fputs("     w 2 div h 2 div moveto\n", fp);
    fputs("     w neg 0 rlineto\n", fp);
    fputs("     0 h neg rlineto\n", fp);
    fputs("     w 0 rlineto\n", fp);
    fputs("     0 h rlineto	\n", fp);
    fputs("     stroke \n", fp);
    fputs("     grestore end\n", fp);
    fputs("   } def\n\n", fp);

    fputs("/kf { transdict begin\n", fp);
    fputs("      /r exch def\n", fp);
    fputs("      /h exch def\n", fp);
    fputs("      /w exch def\n", fp);
    fputs("      /y exch def\n", fp);
    fputs("      /x exch def\n", fp);
    fputs("      gsave\n", fp);
    fputs("      x y translate\n", fp);
    fputs("      r rotate\n", fp);
    fputs("      newpath \n", fp);
    fputs("      w 2 div h 2 div moveto\n", fp);
    fputs("      w neg 0 rlineto\n", fp);
    fputs("      0 h neg rlineto\n", fp);
    fputs("      w 0 rlineto\n", fp);
    fputs("      0 h rlineto	\n", fp);
    fputs("      fill \n", fp);
    fputs("      grestore end\n", fp);
    fputs("    } def\n\n", fp);

    fprintf(fp, "/l { linedict begin\n");
    fprintf(fp, "     /ye exch def\n");
    fprintf(fp, "     /xe exch def\n");
    fprintf(fp, "     /ys exch def\n");
    fprintf(fp, "     /xs exch def\n");
    fprintf(fp, "     newpath xs ys moveto\n");
    fprintf(fp, "     xe ye lineto stroke end\n");
    fprintf(fp, "   } def\n\n");

    fprintf(fp, "/s { stringdict begin\n");
    fprintf(fp, "     /string exch def\n");
    fprintf(fp, "     /y exch def\n");
    fprintf(fp, "     /x exch def\n");
    fprintf(fp, "     gsave x y moveto 1 -1 scale string show grestore end\n");
    fprintf(fp, "   } def\n\n");

    fprintf(fp, "/pg { pgreekdict begin\n");
    fprintf(fp, "      /y exch def\n");
    fprintf(fp, "      /x exch def\n");
    fprintf(fp, "      gsave /Symbol findfont 8 scalefont setfont x y moveto 1 -1 scale (p) show grestore end\n");
    fprintf(fp, "    } def\n\n");

    fputs("/mpp { 25.4 72.27 div } def\n", fp);
    fputs("/ppm { 72.27 25.4 div } def\n\n", fp);

    fputs("%%EndProlog\n", fp);
    fputs("%%Page: 1 1\n\n", fp);
    if (rotate)
        fprintf(fp, "%%%%PageBoundingBox: %d %d %d %d\n", (int)(595. - ((offy + height)) / .351), (int)(842. - ((offx + width) / 0.351)),
                (int)(595. - (((float)offy) / .351)), (int)(842. - (offx / .351)));
    else
        fprintf(fp, "%%%%PageBoundingBox: %d %d %d %d\n", (int)(((float)offx) / .351), (int)(842. - ((offy + height) / 0.351)),
                (int)((offx + width) / .351), (int)(842. - (offy / .351)));
    fputs("gsave\n", fp);

    fprintf(fp, "/Times-Roman findfont %.2f scalefont setfont\n", 8.*zoom_level);
    fputs("1 -1 scale\n", fp);
    if (rotate) {
        fputs("595 -842 translate\n", fp);
        fputs("90 rotate\n", fp);
    }
    else {
        fputs("0 -842 translate\n", fp);
    }
    fputs(".6 setlinewidth\n", fp);
    fputs(" ppm ppm scale\n", fp);
    fprintf(fp, "%.2f %.2f translate \n", (float)offx, (float)offy);

    fprintf(fp, "%.2f %.2f div dup scale\n", (float)width, wclip);
    fprintf(fp, "%.2f %.2f translate \n", -xclip, -yclip);

    fprintf(fp, "newpath %.2f %.2f moveto %.2f 0.0 rlineto 0.0 %.2f rlineto %.2f 0.0 rlineto closepath clip\n",
            xclip, yclip, wclip, hclip, -wclip);

    fprintf(fp, " 1 %.2f div dup scale\n", zoom_level);

    return ferror(fp);
}

void PSCircle(float x, float y, float radius) {
    fprintf(fp, "%.2f %.2f %.2f p\n", x, y, radius);
}

void PSText(float x, float y, char *string) {
    fprintf(fp, "%.2f %.2f (%s) s\n", x, y, String2PSString(string));
}

void PSPGreek(int x, int y) {
    fprintf(fp, "%d %d pg\n", x, y);
}

void PSTransition(float x, float y, float w, float h, float a, int fi) {
    if (fi)
        fprintf(fp, "%.2f %.2f %.2f %.2f %.2f kf\n", x, y, w, h, a);
    else
        fprintf(fp, "%.2f %.2f %.2f %.2f %.2f k\n", x, y, w, h, a);
}


void PSToken(float x, float y, float radius) {
    fprintf(fp, "%.2f %.2f %.2f t\n", x, y, radius / 2.);
}

void PSSpline(float xc, float yc, float xs, float ys, float ixend, float iyend) {
    fprintf(fp, "%.2f %.2f %.2f %.2f %.2f %.2f a\n", xc, yc, xs, ys, ixend, iyend);
}

void PSline(float x, float y, float xx, float yy) {
    fprintf(fp, "%.2f %.2f %.2f %.2f l\n", x, y, xx, yy);
}


Boolean EndPS(Boolean tex) {
    Boolean err;

    if (!tex)
        fprintf(fp, "\nshowpage\n");
    fputs("grestore\n", fp);
    fputs("%%Trailer\n", fp);
    fputs("%%DocumentFonts: Times-Roman Symbol\n", fp);
    fputs("%%EOF\n", fp);
    err = ferror(fp);
    return fclose(fp) || err;
}
