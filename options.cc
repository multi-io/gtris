#include "options.h"
#include <gtk/gtk.h>
#include <stdio.h>


static GtkDialog* m_dialog;

static GtkRadioButton
    *m_btnClBnW,
    *m_btnClBasic,
    *m_btnClWide;

static GtkEntry* m_entryFileName;

static GtkSpinButton* m_spbtnLevel;

static void ok_click (GtkWidget*, gpointer);
static void cancel_click (GtkWidget*, gpointer);
static gint on_dlg_delete ( GtkWidget*, GdkEvent, gpointer );

static bool m_bOK;


bool GetOptions (int* level, std::string* hscfile, CTetrisGameProcess::StoneColorRange* colorRange)
{
    printf ("GetOptions Anfang: %i\n",*level);

    m_dialog = GTK_DIALOG( gtk_dialog_new() );
    gtk_window_set_title ( GTK_WINDOW(m_dialog), "Options");

    GtkButton* b = GTK_BUTTON( gtk_button_new_with_label("OK") );
    gtk_box_pack_start (GTK_BOX(m_dialog->action_area), GTK_WIDGET(b), FALSE, TRUE, 0);
    gtk_signal_connect (GTK_OBJECT(b), "clicked",
                        GTK_SIGNAL_FUNC(ok_click), NULL);
    gtk_widget_show (GTK_WIDGET(b));

    b = GTK_BUTTON( gtk_button_new_with_label("Cancel") );
    gtk_box_pack_start (GTK_BOX(m_dialog->action_area), GTK_WIDGET(b), FALSE, TRUE, 0);
    gtk_signal_connect (GTK_OBJECT(b), "clicked",
                        GTK_SIGNAL_FUNC(cancel_click), NULL);
    gtk_widget_show (GTK_WIDGET(b));

    GtkTable* table = GTK_TABLE( gtk_table_new (5,2,FALSE) );
    gtk_container_add (GTK_CONTAINER(m_dialog->vbox), GTK_WIDGET(table));
    gtk_widget_show (GTK_WIDGET(table));


    m_btnClBnW = GTK_RADIO_BUTTON( gtk_radio_button_new_with_label (NULL,"black & white") );

    GSList* group = gtk_radio_button_group (m_btnClBnW);
    m_btnClBasic = GTK_RADIO_BUTTON( gtk_radio_button_new_with_label(group, "basic") );
    group = gtk_radio_button_group (m_btnClBasic);
    m_btnClWide  = GTK_RADIO_BUTTON( gtk_radio_button_new_with_label(group, "wide") );

    gtk_table_attach_defaults (table, GTK_WIDGET(m_btnClBnW),0,1,0,1);
    gtk_table_attach_defaults (table, GTK_WIDGET(m_btnClBasic),0,1,1,2);
    gtk_table_attach_defaults (table, GTK_WIDGET(m_btnClWide),0,1,2,3);

    gtk_widget_show (GTK_WIDGET(m_btnClBnW));
    gtk_widget_show (GTK_WIDGET(m_btnClBasic));
    gtk_widget_show (GTK_WIDGET(m_btnClWide));


    GtkWidget* lbl = gtk_label_new ("Highscores File:");
    gtk_table_attach_defaults (table, lbl,0,1,3,4);
    gtk_widget_show (lbl);

    m_entryFileName = GTK_ENTRY( gtk_entry_new_with_max_length (255) );
    gtk_table_attach_defaults (table, GTK_WIDGET(m_entryFileName),0,2,4,5);
    gtk_widget_show (GTK_WIDGET(m_entryFileName));


    lbl = gtk_label_new ("Level:");
    gtk_table_attach_defaults (table, lbl,1,2,0,1);
    gtk_widget_show (lbl);

    GtkAdjustment* adj = GTK_ADJUSTMENT( gtk_adjustment_new (0,0,4,1,4,1) );
    m_spbtnLevel = GTK_SPIN_BUTTON( gtk_spin_button_new (adj, 0.5, 1) );
    gtk_table_attach_defaults (table, GTK_WIDGET(m_spbtnLevel),1,2,1,2);
    gtk_widget_show (GTK_WIDGET(m_spbtnLevel));


    GtkRadioButton* rb;
    switch (*colorRange)
    {
    case CTetrisGameProcess::scrBlackWhite:
        rb = m_btnClBnW;
        break;
    case CTetrisGameProcess::scrBasic:
        rb = m_btnClBasic;
        break;
    case CTetrisGameProcess::scrWide:
        rb = m_btnClWide;
        break;
    default:
        rb = m_btnClBasic;
        break;
    }
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(rb), TRUE);

    gtk_entry_set_text (m_entryFileName, hscfile->c_str());

    gtk_spin_button_set_value (m_spbtnLevel,*level);

    gtk_signal_connect (GTK_OBJECT (m_dialog), "delete_event",
                        GTK_SIGNAL_FUNC (on_dlg_delete), NULL);

    gtk_widget_show (GTK_WIDGET(m_dialog));

    gtk_grab_add (GTK_WIDGET(m_dialog));
    gtk_main ();
    gtk_grab_remove (GTK_WIDGET(m_dialog));

    if (!m_bOK)
        return false;

    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(m_btnClBnW)))
        *colorRange = CTetrisGameProcess::scrBlackWhite;
    else if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(m_btnClBasic)))
        *colorRange = CTetrisGameProcess::scrBasic;
    else if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(m_btnClWide)))
        *colorRange = CTetrisGameProcess::scrWide;

    *hscfile = gtk_entry_get_text (m_entryFileName);

    *level = gtk_spin_button_get_value_as_int (m_spbtnLevel);

    gtk_widget_destroy (GTK_WIDGET(m_dialog));

    printf ("GetOptions Ende: %i\n",*level);
}


static void ok_click (GtkWidget*, gpointer)
{
    m_bOK = true;
    gtk_widget_hide (GTK_WIDGET(m_dialog));
    gtk_main_quit();
}


static void cancel_click (GtkWidget*, gpointer)
{
    m_bOK = false;
    gtk_widget_hide (GTK_WIDGET(m_dialog));
    gtk_main_quit();
}


static gint on_dlg_delete ( GtkWidget*, GdkEvent, gpointer )
{
    cancel_click (NULL,NULL);
}
