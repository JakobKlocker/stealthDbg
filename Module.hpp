#include <Windows.h>
#include <string>

struct Module{
    std::string name;
    DWORD64 memorySource;
    DWORD64 address;
};