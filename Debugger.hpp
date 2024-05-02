#include "ProcessMemory.hpp"
#include <Windows.h>
#include <string>
#include <vector>
#include <iostream>
#include <limits>

const int TRAP_FLAG = 1 << 8;

class Debugger
{
private:
    STARTUPINFOW si;
    PROCESS_INFORMATION pi;
    ProcessMemory pm;
    std::wstring targetPath;
    DEBUG_EVENT dEventInfo;
    CONTEXT ct;
    HANDLE currentHandle;

public:
    Debugger(const std::wstring target);
    void SetpInto(HANDLE handle);
    void determinAction(const std::string &cmd);
    void printBytes();
    void run();
};