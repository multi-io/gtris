#include <stdlib.h>
#include <stdio.h>
#include <fstream.h>
#include <algorithm>
#include "HighscoresManager.h"
#include "registry.h"


THscEntry::THscEntry(const char* iname, int iscore, int ilines, int idate) :
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
        return lines > e2.lines;
    else
        return score > e2.score;
}

bool THscEntry::operator== (const THscEntry& e2) const
{
    return (score==e2.score) && (lines == e2.lines);
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

    GtkNotebook* notebook = GTK_NOTEBOOK( gtk_notebook_new() );
    gtk_container_add (GTK_CONTAINER(m_dialog->vbox), GTK_WIDGET(notebook));
    gtk_widget_show (GTK_WIDGET(notebook));


    char* titles[] = { "Name         ", "Score ", "Lines", "Date   " };
    int nTitles = 4; //sizeof(titles)/sizeof(titles[0]);

    for (int i=0; i < nLevels; i++)
    {
        GtkCList* cl = GTK_CLIST( gtk_clist_new_with_titles (nTitles,titles) );
        
        char tabLabel[10];
        sprintf (tabLabel,"Level %i",i);
        GtkLabel* lbl = GTK_LABEL( gtk_label_new (tabLabel) );
        gtk_notebook_append_page (notebook, GTK_WIDGET(cl), GTK_WIDGET(lbl));
        gtk_widget_show (GTK_WIDGET(lbl));
        gtk_widget_show (GTK_WIDGET(cl));

        m_CLists[i] = cl;

        UpdateList (i);
    }

    gtk_signal_connect (GTK_OBJECT (m_dialog), "delete_event",
                        GTK_SIGNAL_FUNC (dialog_delete_event), (gpointer)this);


//     int iTab;
//     if (registry.QueryValue ("HscTab", (LPDWORD)&iTab) == ERROR_SUCCESS)
//         m_HscTabCtrl.SetCurSel (iTab);

//     int x,y;
//     if (registry.QueryValue ("HscPosX", (LPDWORD)&x) == ERROR_SUCCESS &&
//         registry.QueryValue ("HscPosY", (LPDWORD)&y) == ERROR_SUCCESS)
//     {
//         SetWindowPos (NULL,x,y,0,0,SWP_NOSIZE|SWP_NOZORDER);
//     }

}


HighscoresManager::~HighscoresManager()
{
//     CRect r;
//     GetWindowRect (&r);

//     registry.SetValue ("HscPosX", r.left);
//     registry.SetValue ("HscPosY", r.top);
//     registry.SetValue ("HscTab", m_HscTabCtrl.GetCurSel ());

    ShowDialog (false);
    gtk_widget_destroy (GTK_WIDGET(m_dialog));
}


void HighscoresManager::UpdateList (int iLevel)
{
    THscList& hsclist = m_HscLists[iLevel];
    
    GtkCList* clist = m_CLists[iLevel];
    gtk_clist_clear (clist);
    char* row[4];

    size_t i;
    for (i=0; i<hsclist.size(); i++)
    {
        THscEntry& entry = hsclist[i];

        char namestr[50];
        row[0] = strcpy (namestr, entry.name.c_str());

        char scorestr[15];
        sprintf (scorestr,"%i",entry.score);
        row[1] = scorestr;

        char linesstr[15];
        sprintf (linesstr,"%i",entry.lines);
        row[2] = linesstr;

        char datestr[50];
        tm* ptm = localtime (&entry.date);
        strftime (datestr, 50, "%m/%d/%y", ptm);
        row[3] = datestr;

        gtk_clist_append (clist,row);
    }

    row[0] = row[1] = row[2] = row[3] = NULL;
    for (; i<nEntries; i++)
    {
        gtk_clist_append (clist,row);
    }
}


void HighscoresManager::AddNewEntry (THscEntry& entry, int iLevel)
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


bool HighscoresManager::LoadFromFile (const std::string& file)
{
    struct
    {
        char name[20];
        int score,lines;
        time_t date;
    }
    FileEntry;


    ifstream fs (file.c_str(),ios::in|ios::binary|ios::nocreate);

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
            fs.read ((char*)&FileEntry,sizeof(FileEntry));
            if (strlen(FileEntry.name) >= 20 ||
                FileEntry.score < 0 ||
                FileEntry.lines < 0 ||
                FileEntry.date < 0)
                return false;

            THscEntry NewEntry
                (FileEntry.name, FileEntry.score,
                 FileEntry.lines, FileEntry.date);
            list.push_back (NewEntry);
        }
        std::sort (list.begin(), list.end());
    }

    for (iLvl=0; iLvl < nLevels; iLvl++)
    {
        //m_HscLists[iLvl].assign (hsclists[iLvl].begin(), hsclists[iLvl].end());
        m_HscLists[iLvl].erase(m_HscLists[iLvl].begin(), m_HscLists[iLvl].end());
        m_HscLists[iLvl].insert(m_HscLists[iLvl].begin(), hsclists[iLvl].begin(), hsclists[iLvl].end());
    }

    for (int i=0; i<nLevels; i++)
        UpdateList (i);

    return true;
}


bool HighscoresManager::SaveToFile (const std::string& file) const
{
    struct
    {
        char name[20];
        int score,lines;
        time_t date;
    }
    FileEntry;

    ofstream fs (file.c_str(),ios::out|ios::binary);
    if (!fs.rdbuf()->is_open())
        return false;

    for (int iGrd=0; iGrd < nLevels; iGrd++)
    {
        const THscList& CurrList = m_HscLists[iGrd];
        size_t s = CurrList.size();
        fs.write ((char*)&s,sizeof(s));

        for (size_t i=0; i<s; i++)
        {
            const THscEntry& entry = CurrList[i];
            strcpy (FileEntry.name, entry.name.c_str());
            FileEntry .score = entry.score;
            FileEntry .lines = entry.lines;
            FileEntry .date = entry.date;
            fs.write ((char*)&FileEntry, sizeof(FileEntry ));
        }
    }

    return true;
}


void HighscoresManager::ShowDialog (bool bShow = true)
{
    if (bShow)
        gtk_widget_show (GTK_WIDGET(m_dialog));
    else
        gtk_widget_hide (GTK_WIDGET(m_dialog));
}


bool HighscoresManager::IsDialogVisible ()
{
    return GTK_WIDGET_VISIBLE ( GTK_WIDGET(m_dialog) );
}



bool HighscoresManager::HighscoresUserQuery (THscEntry* entry, int level)
{
    entry->name = "Olaf";
    return true;
}
