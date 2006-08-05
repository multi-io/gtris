/*  $Id: registry.h,v 1.3.2.1.2.1 2006/08/05 07:03:04 olaf Exp $ */

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

#ifndef REGISTRY_H
#define REGISTRY_H

#include <string>
#include <map>
#include <sstream>
#include <algorithm>


class Registry
{
    std::string m_strRegFileName;

    typedef std::map<std::string,std::string> map_type;
    map_type m_ValuesMap;

public:
    Registry ();
    ~Registry ();

    template <class T>
    bool QueryValue (const char* key, T& result) const
    {
        map_type::const_iterator it = m_ValuesMap.find(key);
        if (it == m_ValuesMap.end())
            return false;
        std::istringstream ss (it->second);
        ss >> result;
        return !ss.fail();
    }

    bool QueryValue (const char* key, std::string& result) const
    {
        map_type::const_iterator it = m_ValuesMap.find(key);
        if (it == m_ValuesMap.end())
            return false;
        result = it->second.c_str();
        return true;
    }

    template <class T>
    bool SetValue (const char* key, const T& value)
    {
        std::ostringstream ss;
        ss << value << '\0';
        if (ss.fail()) return false;
        m_ValuesMap[key] = ss.str();
        return true;
    }

};


extern Registry registry;

#endif
