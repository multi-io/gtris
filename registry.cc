#include "registry.h"
#include <fstream>
#include <iostream>
#include <algorithm>
#include <glib.h>

using namespace std;


Registry::Registry ()
{
    m_strRegFileName = g_get_home_dir();
    m_strRegFileName += "/.gtrisrc";

    ifstream fs (m_strRegFileName.c_str(),ios::in|ios::nocreate);
    if (!fs.rdbuf()->is_open())
        return;

    const int maxLength=1000;
    char buf[maxLength+1];
    string strKey,strVal;
    do
    {
        fs.getline (buf,maxLength);
        if (fs.rdstate() & ios::failbit)
        {
            cerr << "Warning: line length in " << m_strRegFileName
                 << " exceeds " << maxLength << " bytes." << endl;
            continue;
        }

        int len = strlen (buf);
        /*const*/ char* pEnd = buf+len;
        char* pVal = find (buf,pEnd,'=');
        if (pVal == pEnd)
        {
            cerr << "Warning: " << m_strRegFileName
                 << ": no '=' found in line " << endl;
            continue;
        }

        *pVal = '\0';
        strKey = buf;
        pVal++;
        strVal = "";

        char* pNextSlash = find (pVal,pEnd,'\\');
        strVal.append (pVal, pNextSlash-pVal);
        while (pNextSlash != pEnd)
        {
            switch (*(pNextSlash+1))
            {
            case 'n':
                strVal.append ("\n");
                pVal = pNextSlash + 2;
                break;
            case '\\':
                strVal.append ("\\");
                pVal = pNextSlash + 2;
                break;
            default:
                pVal = pNextSlash + 1;
                break;
            }

            pNextSlash = find (pVal,pEnd,'\\');
            strVal.append (pVal, pNextSlash-pVal);
        }

        m_ValuesMap[strKey] = strVal;
    }
    while (!(fs.rdstate() & ios::eofbit));
}



static bool NeedsCoding (char c)
{
    return c=='\n' || c=='\\';
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
        fs << it->first << "=";

        const char* pVal = it->second.c_str();
        const char* pEnd = pVal + it->second.size();
        string strCodedVal;
        const char* pNextUncodedChar = find_if (pVal,pEnd,NeedsCoding);
        strCodedVal.append (pVal, pNextUncodedChar-pVal);
        while (pNextUncodedChar != pEnd)
        {
            switch (*pNextUncodedChar)
            {
            case '\n':
                strCodedVal.append ("\n");
                break;
            case '\\':
                strCodedVal.append ("\\\\");
                break;
            default:
                cerr << "Uncodable character found while writing "
                     << m_strRegFileName << ". Ignored." << endl;
                break;
            }
            pVal = pNextUncodedChar+1;
            pNextUncodedChar = find_if (pVal,pEnd,NeedsCoding);
            strCodedVal.append (pVal, pNextUncodedChar-pVal);
        }

        fs << strCodedVal << endl;
    }
}

//TODO: das Nullbyte mit '\0' codieren
//dann koennen die values aber nicht mehr in std::strings gespeichert werden
//(weil 0 Endekennzeichen)
//TODO: evtl. alle nichtdruckbaren Zeichen mit '\0x<code>' codieren


Registry registry;
