/*  $Id: registry.cc,v 1.3.2.1.2.1 2006/08/05 07:03:04 olaf Exp $ */

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

    ifstream fs (m_strRegFileName.c_str(),ios::in);
    if (!fs)
        return;

    const int maxLength=1000;
    char buf[maxLength+1];
    string strKey,strVal;
    fs.getline (buf,maxLength);
    while (!(fs.rdstate() & ios::eofbit))
    {
        if (fs.rdstate() & ios::failbit)
        {
            cerr << "Warning: line length in " << m_strRegFileName
                 << " exceeds " << maxLength << " bytes." << endl;
            break;
            //TODO: das folgende klappt nicht wie erwartet (deswegen der break-Befehl).
            //scheinbar spult getline() bis zum Dateiende vor,
            //wenn die linie zu lang war
            /*
            fs.getline (buf,maxLength);
            continue;
            */
        }

        int len = strlen (buf);
        /*const*/ char* pEnd = buf+len;
        char* pVal = find (buf,pEnd,'=');
        if (pVal == pEnd)
        {
            cerr << "Warning: " << m_strRegFileName
                 << ": no '=' found in line " << endl;
            fs.getline (buf,maxLength);
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

        fs.getline (buf,maxLength);
    }
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
        fs << it->first << '=';

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
                strCodedVal.append ("\\n");
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
