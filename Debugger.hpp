#include <Windows.h>
#include <string>
#include <vector>
#include <iostream>

class Debugger
{
private:
    std::wstring targetPath;
    DEBUG_EVENT dEventInfo;

public:
    Debugger(const std::wstring target);
    void run();
};