#include <stdlib.h>
#include <stdio.h>
#include <fstream>
#include <algorithm>
#include "HighscoresManager.h"
#include "registry.h"


THscEntry::THscEntry(const char* iname, unsigned iscore, unsigned ilines, int idate) :
    name(iname), score(iscore), lines(ilines), date(idate)
{
}

THscEntry::THscEntry(const THscEntry& e)
{
    name = e.name;
    score = e.score;
    lines = e.lines;
    date = e.date;
}


bool THscEntry::operator< (const THscEntry& e2) const
{
    if (score==e2.score)
        return lines > e2.lines;    //Operator verdreht, da std::sort sonst "falschrum" sortiert
    else
        return score > e2.score;
}

bool THscEntry::operator== (const THscEntry& e2) const
{
    return (score==e2.score) && (lines == e2.lines);
}

ostream& operator<< (ostream& os, const THscEntry& e)
{
    size_t s = e.name.length();
    os.write ((char*)&s,sizeof(s));
    os.write (e.name.c_str(), s);
    os.write ((char*)&(e.score), sizeof(e.score));
    os.write ((char*)&(e.lines), sizeof(e.lines));
    os.write ((char*)&(e.date), sizeof(e.date));

    return os;
}


istream& operator>> (istream& is, THscEntry& e)
{
    size_t s;
    is.read ((char*)&s,sizeof(s));
    if (s > 1000)
    {
        is.set(ios::failbit);
        return is;
    }

    char* buf = new char[s+1];
    is.read (buf,s);
    buf[s] = '\0';
    e.name = buf;
    delete[] buf;

    is.read ((char*)&(e.score),sizeof(e.score));
    is.read ((char*)&(e.lines),sizeof(e.lines));
    is.read ((char*)&(e.date),sizeof(e.date));

    return is;
}



static gint dialog_delete_event( GtkWidget *widget,
                                 GdkEvent  *event,
                                 gpointer   data )
{
    gtk_widget_hide (widget);
    return TRUE;
}


static void close_click (GtkWidget*, gpointer pmgr)
{
    ((HighscoresManager*)pmgr)->ShowDialog (false);
}


HighscoresManager::HighscoresManager()
{
    m_dialog = GTK_DIALOG( gtk_dialog_new() );
    gtk_window_set_title ( GTK_WINDOW(m_dialog), "Highscores");

    GtkButton* b = GTK_BUTTON( gtk_button_new_with_label("Close") );
    gtk_container_add (GTK_CONTAINER(m_dialog->action_area), GTK_WIDGET(b));
    gtk_signal_connect (GTK_OBJECT(b), "clicked",
                        GTK_SIGNAL_FUNC(close_click), (gpointer)this);
    gtk_widget_show (GTK_WIDGET(b));

    m_notebook = GTK_NOTEBOOK( gtk_notebook_new() );
    gtk_container_add (GTK_CONTAINER(m_dialog->vbox), GTK_WIDGET(m_notebook));
    gtk_widget_show (GTK_WIDGET(m_notebook));


    char* titles[] = { "Pos", "Name         ", "Score    ", "Lines", "Date   " };
    int nTitles = 5; //sizeof(titles)/sizeof(titles[0]);

    for (int i=0; i < nLevels; i++)
    {
        GtkCList* cl = GTK_CLIST( gtk_clist_new_with_titles (nTitles,titles) );
        gtk_clist_set_column_justification (cl,0,GTK_JUSTIFY_RIGHT);
        gtk_clist_set_column_justification (cl,2,GTK_JUSTIFY_RIGHT);
        gtk_clist_set_column_justification (cl,3,GTK_JUSTIFY_RIGHT);
        
        char tabLabel[10];
        sprintf (tabLabel,"Level %i",i);
        GtkLabel* lbl = GTK_LABEL( gtk_label_new (tabLabel) );
        gtk_notebook_append_page (m_notebook, GTK_WIDGET(cl), GTK_WIDGET(lbl));
        gtk_widget_show (GTK_WIDGET(lbl));
        gtk_widget_show (GTK_WIDGET(cl));

        m_CLists[i] = cl;

        UpdateList (i);
    }

    gtk_signal_connect (GTK_OBJECT (m_dialog), "delete_event",
                        GTK_SIGNAL_FUNC (dialog_delete_event), (gpointer)this);

    int iTab;
    if (registry.QueryValue ("HscTab", iTab))
        gtk_notebook_set_page (m_notebook,iTab);

    gtk_widget_realize (GTK_WIDGET(m_dialog));

    bool visible;
    if (registry.QueryValue ("HscVisible", visible))
        ShowDialog (visible);
}


HighscoresManager::~HighscoresManager()
{
    registry.SetValue ("HscTab", gtk_notebook_get_current_page(m_notebook));
    registry.SetValue ("HscVisible", IsDialogVisible());
    if (IsDialogVisible())
    {
        int x,y;
        gdk_window_get_position (GTK_WIDGET(m_dialog)->window, &x,&y);
        registry.SetValue ("HscPosX", x);
        registry.SetValue ("HscPosY", y);
    }

    ShowDialog (false);
    gtk_widget_destroy (GTK_WIDGET(m_dialog));
}


void HighscoresManager::UpdateList (int iLevel)
{
    THscList& hsclist = m_HscLists[iLevel];
    
    GtkCList* clist = m_CLists[iLevel];
    gtk_clist_clear (clist);
    char* row[5];

    size_t i;
    for (i=0; i<hsclist.size(); i++)
    {
        THscEntry& entry = hsclist[i];

        char posstr[5];
        sprintf (posstr, "%i",i+1);
        row[0] = posstr;

        char namestr[50];
        row[1] = strcpy (namestr, entry.name.c_str());

        char scorestr[15];
        sprintf (scorestr,"%i",entry.score);
        row[2] = scorestr;

        char linesstr[15];
        sprintf (linesstr,"%i",entry.lines);
        row[3] = linesstr;

        char datestr[50];
        tm* ptm = localtime (&entry.date);
        strftime (datestr, 50, "%m/%d/%y", ptm);
        row[4] = datestr;

        gtk_clist_append (clist,row);
    }

    row[0] = row[1] = row[2] = row[3] = row[4] = NULL;
    for (; i<nEntries; i++)
    {
        gtk_clist_append (clist,row);
    }
}


void HighscoresManager::AddNewEntry (const THscEntry& entry, int iLevel)
{
    THscList& list = m_HscLists[iLevel];
    list.push_back (entry);
    std::sort (list.begin(), list.end());
    if (list.size() > nEntries)
        list.erase (list.end()-1);
    UpdateList (iLevel);
}


int HighscoresManager::GetLeastScore (int iLevel) const
{
    const THscList& list = m_HscLists[iLevel];
    if (list.size() < nEntries)
        return 0;
    else
        return list[nEntries-1].score;
}


bool HighscoresManager::LoadFromFile (const char* file)
{
    ifstream fs (file,ios::in|ios::binary|ios::nocreate);

    if (!fs.rdbuf()->is_open())
        return false;

    THscList hsclists[nLevels];
    int iLvl;
    for (iLvl=0; iLvl < nLevels; iLvl++)
    {
        int EntrCnt;
        fs.read ((char*)&EntrCnt,sizeof(EntrCnt));
        if ((EntrCnt < 0) || (EntrCnt > nEntries))
            return false;

        THscList& list = hsclists[iLvl];
        for (int i=0; i<EntrCnt; i++)
        {
            THscEntry NewEntry;
            fs >> NewEntry;
            if (fs.fail())
                return false;
            list.push_back (NewEntry);
        }
        std::sort (list.begin(), list.end());
    }

    for (iLvl=0; iLvl < nLevels; iLvl++)
    {
        m_HscLists[iLvl].erase(m_HscLists[iLvl].begin(), m_HscLists[iLvl].end());
        m_HscLists[iLvl].insert
            (m_HscLists[iLvl].begin(),
             hsclists[iLvl].begin(), hsclists[iLvl].end());
        UpdateList (iLvl);
    }

    return true;
}


bool HighscoresManager::SaveToFile (const char* file) const
{
    ofstream fs (file,ios::out|ios::binary);
    if (!fs.rdbuf()->is_open())
        return false;

    for (int iGrd=0; iGrd < nLevels; iGrd++)
    {
        const THscList& CurrList = m_HscLists[iGrd];
        size_t s = CurrList.size();
        fs.write ((char*)&s,sizeof(s));

        for (size_t i=0; i<s; i++)
        {
            fs << CurrList[i];
        }
    }

    return true;
}


void HighscoresManager::ShowDialog (bool bShow = true)
{
    if (bShow && !IsDialogVisible())
    {
        gtk_widget_show (GTK_WIDGET(m_dialog));

        //TODO:wenn man die Position _vor_ dem gtk_widget_show() - Aufruf setzt,
        //ignoriert der WM (oder wer auch immer) deren Wert und denkt sich selber einen aus :(
        int x,y;
        if (registry.QueryValue ("HscPosX", x) &&
            registry.QueryValue ("HscPosY", y))
        {
            gdk_window_move (GTK_WIDGET(m_dialog)->window, x,y);
            //gtk_window_set_default_size (GTK_WINDOW(m_dialog), x,y);
        }
    }
    else if (!bShow && IsDialogVisible())
    {
        int x,y;
        //TODO: X liefert bei gleichbleibender Fensterposition anscheinend andauernd schwankende
        //Positionswerte zurueck
        gdk_window_get_position (GTK_WIDGET(m_dialog)->window, &x,&y);
        registry.SetValue ("HscPosX", x);
        registry.SetValue ("HscPosY", y);

        gtk_widget_hide (GTK_WIDGET(m_dialog));
    }
}


bool HighscoresManager::IsDialogVisible () const
{
    return GTK_WIDGET_VISIBLE ( GTK_WIDGET(m_dialog) );
}



static void on_btn_clicked (GtkButton *button, gpointer user_data);
static void on_dlg_hide (GtkWidget* dlg);
static gint on_dlg_delete ( GtkWidget*, GdkEvent, gpointer );

static GtkWidget* m_clicked_button;
static GtkWidget* m_dlgHighscores;

bool HighscoresManager::HighscoresUserQuery (THscEntry* entry, int level)
{
    GtkWidget *dialog_vbox2;
    GtkWidget *table3;
    GtkWidget *label2;
    GtkWidget *label3;
    GtkWidget *label1;
    GtkWidget *lblLevel;
    GtkWidget *lblScore;
    GtkWidget *lblLines;
    GtkWidget *entryName;
    GtkWidget *label4;
    GtkWidget *dialog_action_area2;
    GtkWidget *hbox1;
    GtkWidget *btnOK;
    GtkWidget *btnCancel;

    m_dlgHighscores = gtk_dialog_new ();
    gtk_window_set_title (GTK_WINDOW (m_dlgHighscores), "Congratulations!");
    gtk_window_set_policy (GTK_WINDOW (m_dlgHighscores), TRUE, TRUE, FALSE);

    dialog_vbox2 = GTK_DIALOG (m_dlgHighscores)->vbox;
    gtk_widget_show (dialog_vbox2);

    table3 = gtk_table_new (5, 2, TRUE);
    gtk_widget_show (table3);
    gtk_box_pack_start (GTK_BOX (dialog_vbox2), table3, FALSE, FALSE, 0);

    label2 = gtk_label_new ("Score:");
    gtk_widget_show (label2);
    gtk_table_attach (GTK_TABLE (table3), label2, 0, 1, 1, 2,
                      (GtkAttachOptions)GTK_EXPAND,
                      (GtkAttachOptions)(GTK_EXPAND | GTK_FILL), 0, 0);

    label3 = gtk_label_new ("Lines:");
    gtk_widget_show (label3);
    gtk_table_attach (GTK_TABLE (table3), label3, 0, 1, 2, 3,
                      (GtkAttachOptions) GTK_EXPAND,
                      (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 0);

    label1 = gtk_label_new ("Level:");
    gtk_widget_show (label1);
    gtk_table_attach (GTK_TABLE (table3), label1, 0, 1, 0, 1,
                      (GtkAttachOptions) GTK_EXPAND,
                      (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 0);

    lblLevel = gtk_label_new ("label5");
    gtk_widget_show (lblLevel);
    gtk_table_attach (GTK_TABLE (table3), lblLevel, 1, 2, 0, 1,
                      (GtkAttachOptions) GTK_EXPAND,
                      (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 0);

    lblScore = gtk_label_new ("label6");
    gtk_widget_show (lblScore);
    gtk_table_attach (GTK_TABLE (table3), lblScore, 1, 2, 1, 2,
                      (GtkAttachOptions) GTK_EXPAND,
                      (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 0);

    lblLines = gtk_label_new ("label7");
    gtk_widget_show (lblLines);
    gtk_table_attach (GTK_TABLE (table3), lblLines, 1, 2, 2, 3,
                      (GtkAttachOptions) GTK_EXPAND,
                      (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 0);

    entryName = gtk_entry_new ();
    gtk_widget_show (entryName);
    gtk_table_attach (GTK_TABLE (table3), entryName, 0, 2, 4, 5,
                      (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                      (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 0);

    label4 = gtk_label_new ("Name:");
    gtk_widget_show (label4);
    gtk_table_attach (GTK_TABLE (table3), label4, 0, 1, 3, 4,
                      (GtkAttachOptions) GTK_EXPAND,
                      (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 0);

    dialog_action_area2 = GTK_DIALOG (m_dlgHighscores)->action_area;
    gtk_widget_show (dialog_action_area2);
    gtk_container_border_width (GTK_CONTAINER (dialog_action_area2), 10);

    hbox1 = gtk_hbox_new (FALSE, 15);
    gtk_widget_show (hbox1);
    gtk_box_pack_start (GTK_BOX (dialog_action_area2), hbox1, TRUE, TRUE, 0);

    btnOK = gtk_button_new_with_label ("OK");
    gtk_widget_show (btnOK);
    gtk_box_pack_start (GTK_BOX (hbox1), btnOK, TRUE, TRUE, 0);
    gtk_signal_connect (GTK_OBJECT (btnOK), "clicked",
                        GTK_SIGNAL_FUNC (on_btn_clicked),
                        btnOK);

    btnCancel = gtk_button_new_with_label ("Cancel");
    gtk_widget_show (btnCancel);
    gtk_box_pack_start (GTK_BOX (hbox1), btnCancel, TRUE, TRUE, 0);
    gtk_signal_connect (GTK_OBJECT (btnCancel), "clicked",
                        GTK_SIGNAL_FUNC (on_btn_clicked),
                        btnCancel);
    gtk_signal_connect (GTK_OBJECT (m_dlgHighscores), "delete_event",
                        GTK_SIGNAL_FUNC (on_dlg_delete), NULL);

    char strLevel[10], strScore[10], strLines[10];
    sprintf (strLevel,"%i",level);
    sprintf (strScore,"%i",entry->score);
    sprintf (strLines,"%i",entry->lines);

    gtk_label_set_text (GTK_LABEL(lblLevel),strLevel);
    gtk_label_set_text (GTK_LABEL(lblScore),strScore);
    gtk_label_set_text (GTK_LABEL(lblLines),strLines);

    gtk_entry_set_text (GTK_ENTRY(entryName),entry->name.c_str());

    gtk_widget_show (m_dlgHighscores);

    gtk_grab_add (GTK_WIDGET(m_dlgHighscores));
    gtk_main ();
    gtk_grab_remove (m_dlgHighscores);

    entry->name = gtk_entry_get_text (GTK_ENTRY(entryName));

    bool retval = m_clicked_button == btnOK;
    gtk_widget_destroy (m_dlgHighscores);
    return retval;
}


static void on_btn_clicked (GtkButton* button, gpointer user_data)
{
    m_clicked_button = (GtkWidget*)user_data;
    gtk_widget_hide (GTK_WIDGET(m_dlgHighscores));
    gtk_main_quit();
}


static gint on_dlg_delete ( GtkWidget*, GdkEvent, gpointer )
{
    m_clicked_button = NULL;
    gtk_main_quit();
}
