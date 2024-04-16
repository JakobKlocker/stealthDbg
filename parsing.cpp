#include "parsing.hpp"

std::string parseCmd(std::string cmd)
{
    size_t pos = cmd.find(" ");
    if(pos == std::string::npos)
        return "";
    return cmd.substr(pos + 1);
}