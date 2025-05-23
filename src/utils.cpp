#include "utils.h"

std::vector<std::string> split(const std::string &str, const std::string &delimiter)
{
    std::vector<std::string> result;
    size_t pos = 0;
    while (true)
    {
        size_t next_pos = str.find(delimiter, pos);
        if (next_pos == std::string::npos)
        {
            result.push_back(str.substr(pos));
            break;
        }
        result.push_back(str.substr(pos, next_pos - pos));
        pos = next_pos + delimiter.size();
    }
    return result;
}

std::string join(const std::vector<std::string> &strs, const std::string &delimiter)
{
    std::string result;
    for (const auto &str : strs)
    {
        if (!result.empty())
            result += delimiter;
        result += str;
    }
    return result;
}

void replace(std::string &str, const std::string &from, const std::string &to)
{
    size_t start_pos = 0;
    while ((start_pos = str.find(from, start_pos)) != std::string::npos)
    {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
    }
}

std::string replace_all(const std::string &str, const std::string &from, const std::string &to)
{
    std::string result = str;
    size_t start_pos = 0;
    while ((start_pos = str.find(from, start_pos)) != std::string::npos)
    {
        result.replace(start_pos, from.length(), to);
        start_pos += to.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
    }
    return result;
}
