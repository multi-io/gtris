/*  $Id: utils.h,v 1.1.4.1 1999/08/29 18:28:34 olaf Exp $ */

/*  GTris
 *  $Name:  $
 *  Copyright (C) 1999  Olaf Klischat
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

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
