/*  $Id: msgbox.cc,v 1.3.4.2 1999/08/29 18:30:43 olaf Exp $ */

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

//#include <string>
#include <vector>
#include <gtk/gtk.h>
#include "msgbox.h"

using namespace std;

 
const char* btnLabels[nBtnTypes] =
{
    "OK",
    "Cancel",
    "Yes",
    "No"
};

const int twoPowX [] = {1,2,4,8,16,32,64,128};


static GtkDialog* m_dialog;

static void btn_click (GtkWidget*, gpointer p);
const char** pClicked_label;

static gint on_dlg_delete ( GtkWidget*, GdkEvent, gpointer );

int MsgBox (const char* title, const char* msg, int btns)
{
    m_dialog = GTK_DIALOG( gtk_dialog_new() );
    gtk_window_set_title ( GTK_WINDOW(m_dialog), title);

    GtkWidget* msglabel = gtk_label_new (msg);
    gtk_label_set_line_wrap (GTK_LABEL(msglabel), true);
    gtk_label_set_justify (GTK_LABEL(msglabel), GTK_JUSTIFY_FILL);
    gtk_misc_set_alignment (GTK_MISC(msglabel), 0,0);
    gtk_box_pack_start (GTK_BOX(m_dialog->vbox), msglabel, FALSE, TRUE, 10);
    gtk_widget_show (msglabel);

    for (int i=0; i<nBtnTypes; i++)
    {
        if (btns & twoPowX[i])
        {
            GtkWidget* btn = gtk_button_new_with_label(btnLabels[i]);
            gtk_box_pack_start (GTK_BOX(m_dialog->action_area), btn, FALSE, TRUE, 0);
            gtk_signal_connect (GTK_OBJECT(btn), "clicked",
                                GTK_SIGNAL_FUNC(btn_click), (void*) &(btnLabels[i]));
            gtk_widget_show (btn);
        }
    }

    gtk_signal_connect (GTK_OBJECT (m_dialog), "delete_event",
                        GTK_SIGNAL_FUNC (on_dlg_delete), NULL);

    gtk_widget_show (GTK_WIDGET(m_dialog));

    gtk_grab_add (GTK_WIDGET(m_dialog));
    gtk_main ();
    gtk_grab_remove (GTK_WIDGET(m_dialog));

    gtk_widget_destroy (GTK_WIDGET(m_dialog));

    return pClicked_label==NULL? 0 : twoPowX [ pClicked_label - btnLabels ];
}


static void btn_click (GtkWidget*, gpointer p)
{
    pClicked_label  = (const char**)p;
    gtk_widget_hide (GTK_WIDGET(m_dialog));
    gtk_main_quit();
}


static gint on_dlg_delete ( GtkWidget*, GdkEvent, gpointer )
{
    pClicked_label  = NULL;
    gtk_main_quit();
}
