#include <stdio.h>

main(argc, argv)
int argc;
char **argv;
{
    float x, y;
    char *c;

    if (argc < 3) {
        fprintf(stderr, "\nError: not enough parameters\n");
        exit(1);
    }
    c = *++argv;
    switch (*c) {
    case '+' :
        c = *++argv;
        sscanf(c, "%f", &x);
        c = *++argv;
        sscanf(c, "%f", &y);
        printf("%f\n", x + y);
        break;
    case '-' :
        c = *++argv;
        sscanf(c, "%f", &x);
        c = *++argv;
        sscanf(c, "%f", &y);
        printf("%f\n", x - y);
        break;
    case '*' :
        c = *++argv;
        sscanf(c, "%f", &x);
        c = *++argv;
        sscanf(c, "%f", &y);
        printf("%f\n", x * y);
        break;
    case '/' :
        c = *++argv;
        sscanf(c, "%f", &x);
        c = *++argv;
        sscanf(c, "%f", &y);
        printf("%f\n", x / y);
        break;
    case 'm' :
        c = *++argv;
        sscanf(c, "%f", &x);
        if (x >= 0)
            printf("%f\n", x);
        else
            printf("%f\n", -x);
        break;
    case 't' :
        c = *++argv;
        sscanf(c, "%f", &x);
        printf("%d\n", (int)x);
        break;
    default :
        fprintf(stderr, "\nError: unknown operation '%c'\n", *c);
        exit(1);
        break;
    }
}

