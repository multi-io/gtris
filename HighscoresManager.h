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
    THscEntry(const char* name, int score, int lines, int date);
    THscEntry(const THscEntry& e);
    std::string name;
    int score,lines;
    time_t date;
    bool operator< (const THscEntry& e2) const;
    bool operator== (const THscEntry& e2) const;
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
