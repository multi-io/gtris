/*  GTris
 *  Copyright (C) Olaf Klischat
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

#include <QDialog>
#include <QTableWidget>

#include <string>
#include <ctime>
#include <iostream>
#include <set>

#define nLevels 6
#define nEntries 20

struct HscEntry
{
    HscEntry() {}
    HscEntry(const char* name, unsigned score, unsigned lines, time_t date);
    HscEntry(const HscEntry& e);
    std::string name;
    unsigned score,lines;
    time_t date;
    bool operator< (const HscEntry& e2) const;
    bool operator== (const HscEntry& e2) const;
    friend std::ostream& operator<< (std::ostream&, const HscEntry&);
    friend std::istream& operator>> (std::istream&, HscEntry&);
};

typedef std::multiset<HscEntry> HscList;


namespace Ui {
    class HighscoresWindowUi;
}


class HighscoresManager
{
    QWidget *m_parent;
    QDialog *m_hscWindow;
    Ui::HighscoresWindowUi *m_hscWindowUi;
    QTableWidget *m_tables[nLevels];
    HscList m_hscLists[nLevels];

    void rebuildTable(int iLevel);

public:
    HighscoresManager(QWidget *parent = 0);
    virtual ~HighscoresManager();

    void addNewEntry (const HscEntry& entry, int iLevel);
    int getLeastScore (int iLevel) const;

    bool loadFromFile (const char* file);
    bool saveToFile (const char* file) const;

    void showDialog (bool bShow = true);
    bool isDialogVisible () const;

    bool highscoresUserQuery(HscEntry* entry, int level);
};


#endif
