#ifndef REGISTRY_H
#define REGISTRY_H

#include <string>

class Registry
{
    //    HKEY hWintrisRegKey;

public:
    Registry ();
    ~Registry ();

    bool QueryValue (const std::string& name, int* pVal) const;
    bool QueryValue (const std::string& name, std::string& result) const;

    bool SetValue (const std::string& name, int val);
    bool SetValue (const std::string& name, const std::string& val);
};


extern Registry registry;

#endif
