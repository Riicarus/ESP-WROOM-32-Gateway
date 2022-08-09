#ifndef STRING_TOOL_H
#define STRING_TOOL_H

#include <string>
#include <vector>

std::vector<std::string> string_split(std::string s, const std::string d)
{
    if (s.empty())
        return {};
    std::vector<std::string> res;
    std::string strs = s + d;
    size_t pos = strs.find(d);
    size_t len = strs.size();
    while (pos != std::string::npos)
    {
        std::string x = strs.substr(0, pos);
        if (x != "")
            res.push_back(x);
        strs = strs.substr(pos + d.size(), len);
        pos = strs.find(d);
    }
    return res;
}

#endif