#include "Debugger.hpp"
#include <limits>

std::string getUserInput()
{
    std::string ret;
    std::cout << "Type command ('stepin', 'bytes', 'go', 'quit')" << std::endl;
    std::getline(std::cin, ret);
    return ret;
}

Debugger::Debugger(const std::wstring target) : targetPath(target)
{
    std::wcout << "Target: " << target << std::endl;
}

void Debugger::SetpInto(HANDLE handle)
{
    ct.EFlags |= 0x00000100;

    int ret = SetThreadContext(handle, &ct);
    if (!ret)
    {
        DWORD error = GetLastError();
        std::cout << "SetThread failed with error: " << error << std::endl;
    }
    else
    {
        std::cout << "Single-step flag set successfully" << std::endl;
    }
}

void Debugger::printBytes()
{
    std::vector<unsigned char> bytes = this->pm.readMemoryBytes<uint64_t>(pi.hProcess, reinterpret_cast<LPVOID>(ct.Eip), sizeof(uint64_t));
    std::cout << "Reading: " << reinterpret_cast<LPVOID>(ct.Eip) << std::endl;

    for (auto &b : bytes)
    {
        std::cout << std::hex << static_cast<int>(b) << " ";
    }
    std::cout << std::endl;
}

void Debugger::determinAction(const std::string &cmd)
{
    if (cmd == "q")
        std::exit(0);
    if (cmd == "s")
        SetpInto(currentHandle);
    if (cmd == "b")
        printBytes();
    if (cmd.substr(0, 2) == "bp")
    {
        std::cout << "bp called  cmd2 is " << cmd[2] << " size is " << cmd.size() << std::endl;
        if (cmd.size() > 4 && cmd[2] == ' ')
        {
            std::cout << "Size correct" << std::endl;
            LPVOID *addr = (LPVOID *)std::stoull(cmd.substr(3), nullptr, 16);
            std::cout << addr << std::endl;
            breakpointCC(addr);
        }
    }
}

void Debugger::breakpointCC(LPVOID *addr)
{
    BreakpointInfo bp;
    std::vector<unsigned char> originalBytes = pm.readMemoryBytes<unsigned char>(pi.hProcess, addr, 1);

    bp.address = addr;
    bp.originalByte = originalBytes[0];
    breakpoints.push_back(bp);
    pm.writeMemory(pi.hProcess, addr, 0xCC);
}

void Debugger::run()
{
    ZeroMemory(&si, sizeof(si));
    ZeroMemory(&pi, sizeof(pi));

    if (!CreateProcessW(this->targetPath.c_str(), NULL, NULL, NULL, FALSE, DEBUG_PROCESS, NULL, NULL, &si, &pi))
    {
        std::wcout << L"CreateProcess failed" << std::endl;
        return;
    }

    int i = 0;
    while (WaitForDebugEvent(&this->dEventInfo, INFINITE))
    {
        switch (this->dEventInfo.dwDebugEventCode)
        {
        case EXCEPTION_DEBUG_EVENT:
            switch (this->dEventInfo.u.Exception.ExceptionRecord.ExceptionCode)
            {

            case EXCEPTION_SINGLE_STEP:
                std::cout << "Single Step Called" << std::endl;
                std::cout << "RIP: " << std::hex << ct.Eip << std::endl;

                break;

            case EXCEPTION_ACCESS_VIOLATION:
                break;

            case EXCEPTION_BREAKPOINT:
                
                std::cout  << "Exception: " << dEventInfo.u.Exception.ExceptionRecord.ExceptionAddress << std::endl;

                for (int i =0; i < breakpoints.size(); i++)
                {
                    if ((PVOID)breakpoints[i].address == dEventInfo.u.Exception.ExceptionRecord.ExceptionAddress)
                    {
                        memset(&ct, 0, sizeof(ct));
                        ct.ContextFlags = CONTEXT_FULL;
                        if (GetThreadContext(pi.hThread, &ct))
                        {
                            ct.EFlags |= 0x00000100;
                            ct.Eip--;
                            int ret = SetThreadContext(pi.hThread, &ct);
                            std::cout << "ret" << ret << std::endl;

                            std::cout << "Original Byte found = " << breakpoints[i].originalByte << std::endl;
                            pm.writeMemory(pi.hProcess, (LPVOID)ct.Eip, breakpoints[i].originalByte);
                        }
                        breakpoints.erase(breakpoints.begin() + i);
                    }
                }
                break;

            case EXCEPTION_DATATYPE_MISALIGNMENT:
                break;

            case DBG_CONTROL_C:
                break;

            default:
                break;
            }

            break;

        case CREATE_THREAD_DEBUG_EVENT:
            std::wcout << L"Create Thread" << std::endl;

            break;

        case CREATE_PROCESS_DEBUG_EVENT:
            std::wcout << L"Create Process" << std::endl;
            break;

        case EXIT_THREAD_DEBUG_EVENT:
            // Display the thread's exit code.
            std::wcout << L"Exit Thread" << std::endl;
            break;

        case EXIT_PROCESS_DEBUG_EVENT:
            // Display the process's exit code.
            std::wcout << L"Exit Process" << std::endl;

            return;

        case LOAD_DLL_DEBUG_EVENT:
        {
            _LOAD_DLL_DEBUG_INFO loadDllInfo = dEventInfo.u.LoadDll;
            DWORD64 baseAddrOfDll = reinterpret_cast<DWORD64>(loadDllInfo.lpBaseOfDll);

            void *scanResult = pm.readMemory<void *>(pi.hProcess, loadDllInfo.lpImageName);

            if (scanResult)
            {
                std::vector<unsigned char> stringBytes = pm.readMemoryBytes<unsigned char>(pi.hProcess, reinterpret_cast<LPVOID>(scanResult), 255);

                if (loadDllInfo.fUnicode)
                {
                    std::wstring str = bytesToWideString(stringBytes);
                    std::wcout << str << std::endl;
                }
                else
                {
                    std::string str = bytesToString(stringBytes);
                    std::cout << str << std::endl;
                }
            }

            std::cout << "Dll loaded at: " << baseAddrOfDll << std::endl;

            break;
        }

        case UNLOAD_DLL_DEBUG_EVENT:
            std::wcout << L"DLL Unloaded" << std::endl;

            break;

        case OUTPUT_DEBUG_STRING_EVENT:
            break;

        case RIP_EVENT:
            break;
        }

        this->currentHandle = OpenThread(
            THREAD_GET_CONTEXT | THREAD_SET_CONTEXT, // Add THREAD_SUSPEND_RESUME
            FALSE,
            this->dEventInfo.dwThreadId);

        std::cout << "Handle: " << currentHandle << std::endl;

        if (currentHandle != NULL)
        {
            memset(&ct, 0, sizeof(ct));
            ct.ContextFlags = CONTEXT_FULL;

            DWORD tmp = GetThreadContext(currentHandle, &ct);
            std::cout << "GetThreadContextMsg: " << tmp << std::endl;
            if (tmp)
            {
                std::cout << "RIP: " << std::hex << ct.Eip << std::endl;
            }
            else // GetThreadContext failed
            {
                DWORD error = GetLastError();
                std::cerr << "GetThreadContext failed with error code: " << error << std::endl;
            }

            std::string cmd = getUserInput();
            determinAction(cmd);
            CloseHandle(currentHandle);
        }
        else // OpenThread failed
        {
            DWORD error = GetLastError();
            std::cerr << "OpenThread failed with error code: " << error << std::endl;
        }

        // Resume executing the thread that reported the debugging event.

        ContinueDebugEvent(this->dEventInfo.dwProcessId,
                           this->dEventInfo.dwThreadId,
                           DBG_CONTINUE);
    }
}
