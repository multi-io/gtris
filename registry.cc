#include "registry.h"


Registry::Registry ()
{
}


Registry::~Registry ()
{
}


bool Registry::QueryValue (const std::string& name, int* pVal) const
{
    *pVal = 5;
    return true;
}


bool Registry::QueryValue (const std::string& name, std::string& result) const
{
    result = "hallo";
    return true;
}


bool Registry::SetValue (const std::string& name, int val)
{
    return true;
}


bool Registry::SetValue (const std::string& name, const std::string& val)
{
    return true;
}


Registry registry;
