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
    gtk_box_pack_start (GTK_BOX(m_dialog->vbox), msglabel, FALSE, TRUE, 10);
    gtk_widget_show (msglabel);

    vector<const char**> labels;
    int i;
    for (i=0; i<nBtnTypes; i++)
    {
        if (btns & twoPowX[i])
            labels.push_back (&(btnLabels[i]));
    }

    for (i=0; i<labels.size(); i++)
    {
        GtkWidget* btn = gtk_button_new_with_label(*(labels[i]));
        gtk_box_pack_start (GTK_BOX(m_dialog->action_area), btn, FALSE, TRUE, 0);
        gtk_signal_connect (GTK_OBJECT(btn), "clicked",
                            GTK_SIGNAL_FUNC(btn_click), (void*)(labels[i]));
        gtk_widget_show (btn);
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
