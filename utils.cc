/*  $Id: utils.cc,v 1.1.4.2.2.1 2000/01/30 04:22:30 olaf Exp $ */

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

#include "utils.h"

using namespace std;


GdkColor RGB(gushort r, gushort g, gushort b)
{
    GdkColor col;
    col.red = r;
    col.green = g;
    col.blue = b;
    col.pixel = 0;
    return col;
}


void connect_accelerator
    (const char* label_text,
     GtkLabel* label_widget,
     GtkWidget* target,
     const char* signal,
     GtkAccelGroup* accel_group)
{
    guint accel_key;
    accel_key = gtk_label_parse_uline (label_widget,label_text);
    gtk_widget_add_accelerator
            (target, signal,
             accel_group,
             accel_key,
             GDK_MOD1_MASK,
             GTK_ACCEL_LOCKED);
}


void connect_button_accelerator
    (GtkButton* button,
     const char* label_text,
     GtkAccelGroup* accel_group)
{
    guint accel_key;
    GtkWidget* btn_label = gtk_label_new ("");
    accel_key = gtk_label_parse_uline (GTK_LABEL(btn_label),label_text);
    gtk_container_add (GTK_CONTAINER(button), btn_label);
    gtk_widget_show (btn_label);
    gtk_widget_add_accelerator
            (GTK_WIDGET(button), "clicked",
             accel_group,
             accel_key,
             GDK_MOD1_MASK,
             GTK_ACCEL_LOCKED);
}


string get_datafile_path(const char* filename, int desired_mode)
{
    string path;

    path = string(DATA_DIR) + "/gtris/" + filename;
    if (0 == access(path.c_str(), desired_mode))
        return path;

    gchar* buf = g_get_current_dir();
    path = buf;
    g_free (buf);
    path += "/";
    path += filename;
    if (0 == access(path.c_str(), desired_mode))
        return path;

    return "";
}
