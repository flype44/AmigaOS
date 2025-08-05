/* C-Language Syntax Highlighting */

#include <stdio.h>
#include "main.h"

#define D(...) \
    KPrintF(__VA_ARGS__)

#ifndef PI
#define PI 3.141592653589793
#endif

static const double pi = PI;

/* struct */
typedef struct Point {
    int x, y; // coordinates
} Point;

/* entry point */
int main(int argc, char ** argv)
{
    while (--argc > 0)
    {
        printf("%s ", argv[argc]);
    }
    
    Point p = { 1, 2 };
    printf("x=%d y=%d\n", p.x, p.y);
    
    return 0;
}
