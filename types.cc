#include "types.h"


GdkColor RGB(gushort r, gushort g, gushort b)
{
    GdkColor col;
    col.red = r;
    col.green = g;
    col.blue = b;
    col.pixel = 0;
    return col;
}

