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

#include "HighscoresManager.h"
#include "ui_HighscoresWindow.h"

#include "ui_HscEntryDialog.h"

#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <fstream>
#include <algorithm>
#include "registry.h"


using namespace std;


//const char * const headers[] = {"Name", "Score", "Lines", "Date"};
const QStringList headers = {"Name", "Score", "Lines", "Date"};


HscEntry::HscEntry(const char* iname, unsigned iscore, unsigned ilines, time_t idate) :
    name(iname), score(iscore), lines(ilines), date(idate)
{
}

HscEntry::HscEntry(const HscEntry& e)
{
    name = e.name;
    score = e.score;
    lines = e.lines;
    date = e.date;
}


bool HscEntry::operator< (const HscEntry& e2) const
{
    if (score==e2.score)
        return lines > e2.lines;    //Operator verdreht, da std::sort sonst "falschrum" sortiert
    else
        return score > e2.score;
}

bool HscEntry::operator== (const HscEntry& e2) const
{
    return (score==e2.score) && (lines == e2.lines);
}

ostream& operator<< (ostream& os, const HscEntry& e)
{
    size_t s = e.name.length();
    os.write ((char*)&s,sizeof(s));
    os.write (e.name.c_str(), s);
    os.write ((char*)&(e.score), sizeof(e.score));
    os.write ((char*)&(e.lines), sizeof(e.lines));
    os.write ((char*)&(e.date), sizeof(e.date));

    return os;
}


istream& operator>> (istream& is, HscEntry& e)
{
    size_t s;
    is.read ((char*)&s,sizeof(s));
    if (s > 1000)
    {
        is.setstate(ios::failbit);
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


HighscoresManager::HighscoresManager(QWidget *parent) :
        m_parent(parent) {
    m_hscWindow = new QDialog(parent);
    m_hscWindowUi = new Ui::HighscoresWindowUi();
    m_hscWindowUi->setupUi(m_hscWindow);
    //m_hscWindow->adjustSize();
    for (int iLevel = 0; iLevel < nLevels; iLevel++) {
        QTableWidget *table = m_tables[iLevel] = new QTableWidget(nEntries, 4);
        table->setHorizontalHeaderLabels(headers);
        char title[50];
        snprintf(title, sizeof(title), "Level %i", (iLevel + 1));
        m_hscWindowUi->tabWidget->addTab(table, title);
        rebuildTable(iLevel);
    }
}

HighscoresManager::~HighscoresManager() {

}

void HighscoresManager::rebuildTable(int iLevel) {
    HscList& hscList = m_hscLists[iLevel];
    QTableWidget *table = m_tables[iLevel];
    table->setRowCount(hscList.size());
    int row = 0;
    char buf[50];
    for (HscEntry entry : hscList) {
        table->setItem(row, 0, new QTableWidgetItem(entry.name.c_str()));
        snprintf(buf, sizeof(buf), "%i", entry.score);
        table->setItem(row, 1, new QTableWidgetItem(buf));
        snprintf(buf, sizeof(buf), "%i", entry.lines);
        table->setItem(row, 2, new QTableWidgetItem(buf));
        tm *t = localtime(&entry.date);
        strftime(buf, sizeof(buf), "%Y/%m/%d", t);
        table->setItem(row, 3, new QTableWidgetItem(buf));
        ++row;
    }
}

void HighscoresManager::addNewEntry (const HscEntry& entry, int iLevel) {
    HscList& hscList = m_hscLists[iLevel];
    hscList.insert(entry);
    if (hscList.size() > nEntries) {
        hscList.erase(--hscList.end());
    }
    rebuildTable(iLevel);
}

int HighscoresManager::getLeastScore (int iLevel) const {
    const HscList& hscList = m_hscLists[iLevel];
    if (hscList.size() < nEntries) {
        return 0;
    } else {
        return (--hscList.end())->score;
    }
}

bool HighscoresManager::loadFromFile (const char* file) {

}

bool HighscoresManager::saveToFile (const char* file) const {

}

void HighscoresManager::showDialog (bool bShow) {
    m_hscWindow->setVisible(bShow);
}


bool HighscoresManager::isDialogVisible () const {
    return m_hscWindow->isVisible();
}

bool HighscoresManager::highscoresUserQuery(HscEntry* entry, int level) {
    QDialog *dlg = new QDialog(m_parent);
    auto ui = new Ui::HscEntryDialog();
    ui->setupUi(dlg);
    char buf[50];
    snprintf(buf, sizeof(buf), "%i", level);
    ui->levelLabel->setText(buf);
    snprintf(buf, sizeof(buf), "%i", entry->score);
    ui->scoreLabel->setText(buf);
    snprintf(buf, sizeof(buf), "%i", entry->lines);
    ui->linesLabel->setText(buf);
    ui->nameEdit->setText(entry->name.c_str());
    if (dlg->exec() == QDialog::Accepted) {
        entry->name = ui->nameEdit->text().toUtf8().constData();
        return true;
    } else {
        return false;
    }
}
