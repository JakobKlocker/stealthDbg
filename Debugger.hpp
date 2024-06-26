#include "ProcessMemory.hpp"
#include "Module.hpp"
#include "stringHelper.hpp"
#include <Windows.h>
#include <string>
#include <vector>
#include <iostream>
#include <limits>
#include <utility>

struct BreakpointInfo {
    LPVOID address;
    BYTE originalByte;
};

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
    std::vector<Module>moduleList;

    std::vector<BreakpointInfo> breakpoints;

public:
    Debugger(const std::wstring target);
    void SetpInto(HANDLE handle);
    void determinAction(const std::string &cmd);
    void printBytes();
    void breakpointCC(LPVOID * addr);
    void run();
};