#include "registry.h"
#include <fstream>
#include <iostream>
#include <glib.h>

using namespace std;


Registry::Registry ()
{
    m_strRegFileName = g_get_home_dir();
    m_strRegFileName += "/.gtrisrc";

    ifstream fs (m_strRegFileName.c_str(),ios::in|ios::nocreate);
    if (!fs.rdbuf()->is_open())
        return;

    string key,val;
    while (fs)
    {
        fs>>key>>val;
        m_ValuesMap[key] = val;
    }
}


Registry::~Registry ()
{
    ofstream fs (m_strRegFileName.c_str(),ios::out|ios::trunc);
    if (!fs.rdbuf()->is_open())
    {
        cerr << "couldn't write config file " << m_strRegFileName << endl;
        return;
    }

    for (map_type::const_iterator it = m_ValuesMap.begin();
         it!=m_ValuesMap.end(); it++)
    {
        fs << it->first << "  " << it->second << endl;
    }
}


Registry registry;
