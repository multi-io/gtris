#include "options.h"
#include <gtk/gtk.h>
#include <stdio.h>


static GtkWidget* m_dialog;

static void on_btnOK_clicked (GtkWidget*, gpointer);
static void on_btnCancel_clicked (GtkWidget*, gpointer);
static gint on_dlg_delete ( GtkWidget*, GdkEvent, gpointer );

static bool m_bOK;


bool GetOptions (unsigned* level,
                 std::string* hscfile,
                 CTetrisGameProcess::StoneColorRange* colorRange,
                 unsigned* brickSize)
{
    GtkWidget *dialog_vbox1;
    GtkWidget *table2;
    GtkWidget *frame1;
    GtkWidget *vbox3;
    GSList *btngrp_group = NULL;
    GtkWidget *m_rbClBnW;
    GtkWidget *m_rbClBasic;
    GtkWidget *m_rbClWide;
    GtkWidget *m_entryHscFile;
    GtkWidget *accellabel2;
    GtkWidget *vbox4;
    GtkWidget *label6;
    GtkObject *m_spbLevel_adj;
    GtkWidget *m_spbLevel;
    GtkWidget *label5;
    GtkObject *m_spbBrickSize_adj;
    GtkWidget *m_spbBrickSize;
    GtkWidget *dialog_action_area1;
    GtkWidget *m_btnOK;
    GtkWidget *m_btnCancel;

    m_dialog = gtk_dialog_new ();
    gtk_signal_connect (GTK_OBJECT (m_dialog), "delete_event",
                        GTK_SIGNAL_FUNC (on_dlg_delete),
                        NULL);
    gtk_window_set_title (GTK_WINDOW (m_dialog), "Options");
    gtk_window_set_policy (GTK_WINDOW (m_dialog), TRUE, TRUE, FALSE);

    dialog_vbox1 = GTK_DIALOG (m_dialog)->vbox;
    gtk_widget_show (dialog_vbox1);

    table2 = gtk_table_new (3, 2, FALSE);
    gtk_widget_show (table2);
    gtk_box_pack_start (GTK_BOX (dialog_vbox1), table2, TRUE, TRUE, 0);
    gtk_container_border_width (GTK_CONTAINER (table2), 5);

    frame1 = gtk_frame_new ("Color Range");
    gtk_widget_show (frame1);
    gtk_table_attach (GTK_TABLE (table2), frame1, 0, 1, 0, 1,
                      (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                      (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 0);
    gtk_widget_set_usize (frame1, 135, -2);

    vbox3 = gtk_vbox_new (FALSE, 0);
    gtk_widget_show (vbox3);
    gtk_container_add (GTK_CONTAINER (frame1), vbox3);

    m_rbClBnW = gtk_radio_button_new_with_label (btngrp_group, "black & white");
    btngrp_group = gtk_radio_button_group (GTK_RADIO_BUTTON (m_rbClBnW));
    gtk_widget_show (m_rbClBnW);
    gtk_box_pack_start (GTK_BOX (vbox3), m_rbClBnW, TRUE, TRUE, 0);

    m_rbClBasic = gtk_radio_button_new_with_label (btngrp_group, "basic");
    btngrp_group = gtk_radio_button_group (GTK_RADIO_BUTTON (m_rbClBasic));
    gtk_widget_show (m_rbClBasic);
    gtk_box_pack_start (GTK_BOX (vbox3), m_rbClBasic, TRUE, TRUE, 0);

    m_rbClWide = gtk_radio_button_new_with_label (btngrp_group, "wide");
    btngrp_group = gtk_radio_button_group (GTK_RADIO_BUTTON (m_rbClWide));
    gtk_widget_show (m_rbClWide);
    gtk_box_pack_start (GTK_BOX (vbox3), m_rbClWide, TRUE, TRUE, 0);

    m_entryHscFile = gtk_entry_new ();
    gtk_widget_show (m_entryHscFile);
    gtk_table_attach (GTK_TABLE (table2), m_entryHscFile, 0, 2, 2, 3,
                      (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                      (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 0);

    accellabel2 = gtk_accel_label_new ("Highscores File:");
    gtk_widget_show (accellabel2);
    gtk_table_attach (GTK_TABLE (table2), accellabel2, 0, 1, 1, 2,
                      (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                      (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 0);
    gtk_misc_set_alignment (GTK_MISC (accellabel2), 1.93715e-07, 1);

    vbox4 = gtk_vbox_new (FALSE, 0);
    gtk_widget_show (vbox4);
    gtk_table_attach (GTK_TABLE (table2), vbox4, 1, 2, 0, 1,
                      (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                      (GtkAttachOptions) GTK_EXPAND, 10, 0);

    label6 = gtk_label_new ("Level:");
    gtk_widget_show (label6);
    gtk_box_pack_start (GTK_BOX (vbox4), label6, TRUE, TRUE, 0);
    gtk_misc_set_alignment (GTK_MISC (label6), 1.93715e-07, 1);

    m_spbLevel_adj = gtk_adjustment_new (1, 0, 4, 1, 10, 10);
    m_spbLevel = gtk_spin_button_new (GTK_ADJUSTMENT (m_spbLevel_adj), 1, 0);
    gtk_widget_show (m_spbLevel);
    gtk_box_pack_start (GTK_BOX (vbox4), m_spbLevel, TRUE, TRUE, 0);
    gtk_widget_set_usize (m_spbLevel, 71, -2);

    label5 = gtk_label_new ("Brick Size:");
    gtk_widget_show (label5);
    gtk_box_pack_start (GTK_BOX (vbox4), label5, TRUE, TRUE, 0);
    gtk_misc_set_alignment (GTK_MISC (label5), 1.93715e-07, 1);

    m_spbBrickSize_adj = gtk_adjustment_new (1, 1, 100, 1, 10, 10);
    m_spbBrickSize = gtk_spin_button_new (GTK_ADJUSTMENT (m_spbBrickSize_adj), 1, 0);
    gtk_widget_show (m_spbBrickSize);
    gtk_box_pack_start (GTK_BOX (vbox4), m_spbBrickSize, TRUE, TRUE, 0);

    dialog_action_area1 = GTK_DIALOG (m_dialog)->action_area;
    gtk_widget_show (dialog_action_area1);
    gtk_container_border_width (GTK_CONTAINER (dialog_action_area1), 10);

    m_btnOK = gtk_button_new_with_label ("OK");
    gtk_widget_show (m_btnOK);
    gtk_box_pack_start (GTK_BOX (dialog_action_area1), m_btnOK, TRUE, TRUE, 0);
    gtk_signal_connect (GTK_OBJECT (m_btnOK), "clicked",
                        GTK_SIGNAL_FUNC (on_btnOK_clicked),
                        NULL);

    m_btnCancel = gtk_button_new_with_label ("Cancel");
    gtk_widget_show (m_btnCancel);
    gtk_box_pack_start (GTK_BOX (dialog_action_area1), m_btnCancel, TRUE, TRUE, 0);
    gtk_signal_connect (GTK_OBJECT (m_btnCancel), "clicked",
                        GTK_SIGNAL_FUNC (on_btnCancel_clicked),
                        NULL);


    GtkWidget* rb;
    switch (*colorRange)
    {
    case CTetrisGameProcess::scrBlackWhite:
        rb = m_rbClBnW;
        break;
    case CTetrisGameProcess::scrBasic:
        rb = m_rbClBasic;
        break;
    case CTetrisGameProcess::scrWide:
        rb = m_rbClWide;
        break;
    default:
        rb = m_rbClBasic;
        break;
    }
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(rb), TRUE);

    gtk_entry_set_text (GTK_ENTRY(m_entryHscFile), hscfile->c_str());

    gtk_spin_button_set_value (GTK_SPIN_BUTTON(m_spbLevel),*level);

    gtk_spin_button_set_value (GTK_SPIN_BUTTON(m_spbBrickSize),*brickSize);

    gtk_widget_show (GTK_WIDGET(m_dialog));

    gtk_grab_add (GTK_WIDGET(m_dialog));
    gtk_main ();
    gtk_grab_remove (GTK_WIDGET(m_dialog));

    if (!m_bOK)
        return false;

    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(m_rbClBnW)))
        *colorRange = CTetrisGameProcess::scrBlackWhite;
    else if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(m_rbClBasic)))
        *colorRange = CTetrisGameProcess::scrBasic;
    else if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(m_rbClWide)))
        *colorRange = CTetrisGameProcess::scrWide;

    *hscfile = gtk_entry_get_text (GTK_ENTRY(m_entryHscFile));

    *level = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON(m_spbLevel));

    *brickSize = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON(m_spbBrickSize));

    gtk_widget_destroy (GTK_WIDGET(m_dialog));

    return true;
}


static void on_btnOK_clicked (GtkWidget*, gpointer)
{
    m_bOK = true;
    gtk_widget_hide (GTK_WIDGET(m_dialog));
    gtk_main_quit();
}


static void on_btnCancel_clicked (GtkWidget*, gpointer)
{
    m_bOK = false;
    gtk_widget_hide (GTK_WIDGET(m_dialog));
    gtk_main_quit();
}


static gint on_dlg_delete ( GtkWidget*, GdkEvent, gpointer )
{
    on_btnCancel_clicked (NULL,NULL);
    return TRUE;
}
