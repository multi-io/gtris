/*  $Id: utils.h,v 1.1.4.2.2.1 2000/01/30 04:22:30 olaf Exp $ */

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

#ifndef UTILS_H
#define UTILS_H

#include <gdk/gdk.h>

#include <gtk/gtkwidget.h>
#include <gtk/gtklabel.h>
#include <gtk/gtkbutton.h>
#include <gtk/gtkaccelgroup.h>

#include <string>
#include <unistd.h>

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


std::string get_datafile_path(const char* filename, int desired_mode = R_OK);

void connect_accelerator
    (const char* label_text,
     GtkLabel* label_widget,
     GtkWidget* target,
     const char* signal,
     GtkAccelGroup* accel_group);

void connect_button_accelerator
    (GtkButton* button,
     const char* label_text,
     GtkAccelGroup* accel_group);


#endif
