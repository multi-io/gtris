#ifndef TYPES_H
#define TYPES_H

#include <gdk/gdk.h>


struct CPoint : public GdkPoint
{
    CPoint ();
    CPoint (gint16 x, gint16 y);
};

inline CPoint::CPoint ()
{
    x = y = 0;
}

inline CPoint::CPoint (gint16 i_x, gint16 i_y)
{
    x = i_x; y = i_y;
}



//fscking egcs definiert diese beiden nicht automatisch :(

inline bool operator== (const GdkColor& cl1, const GdkColor& cl2)
{
    return cl1.red==cl2.red && cl1.green==cl2.green && cl1.blue==cl2.blue; 
}  

inline bool operator!= (const GdkColor& cl1, const GdkColor& cl2)
{
    return !(cl1==cl2); 
}



GdkColor RGB(gushort r, gushort g, gushort b);


#endif
