#ifndef REGISTRY_H
#define REGISTRY_H

#include <string>
#include <map>
#include <strstream>
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
        istrstream ss (it->second.c_str(),it->second.size());
        ss >> result;
        return true;
    }


    template <class T>
        bool SetValue (const char* key, const T& value)
    {
        ostrstream ss;
        ss << value << '\0';
        m_ValuesMap[key] = ss.str();
        return true;
    }

};


extern Registry registry;

#endif
