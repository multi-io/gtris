/*  $Id: HighscoresManager.h,v 1.4.2.1.2.1 2006/08/05 07:03:04 olaf Exp $ */

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

#ifndef HIGHSCORES_MANAGER_H
#define HIGHSCORES_MANAGER_H

#include <vector>
#include <string>
#include <time.h>
#include <gtk/gtk.h>

#define nLevels 5
#define nEntries 20

struct THscEntry
{
    THscEntry() {}
    THscEntry(const char* name, unsigned score, unsigned lines, int date);
    THscEntry(const THscEntry& e);
    std::string name;
    unsigned score,lines;
    time_t date;
    bool operator< (const THscEntry& e2) const;
    bool operator== (const THscEntry& e2) const;
    friend std::ostream& operator<< (std::ostream&, const THscEntry&);
    friend std::istream& operator>> (std::istream&, THscEntry&);
};

typedef std::vector <THscEntry> THscList;


class HighscoresManager
{
    THscList m_HscLists[nLevels];

    GtkDialog* m_dialog;
    GtkCList* m_CLists[nLevels];
    GtkNotebook* m_notebook;

    void UpdateList (int iLevel);

public:
    HighscoresManager();
    ~HighscoresManager();

    void AddNewEntry (const THscEntry& entry, int iLevel);
    int GetLeastScore (int iLevel) const;

    bool LoadFromFile (const char* file);
    bool SaveToFile (const char* file) const;

    void ShowDialog (bool bShow = true);
    bool IsDialogVisible () const;

    static bool HighscoresUserQuery (THscEntry* entry, int level);
};


#endif
