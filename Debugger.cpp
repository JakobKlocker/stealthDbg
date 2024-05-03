#include "Debugger.hpp"
#include <limits>

std::string getUserInput()
{
    std::string ret;
    std::cout << "Type command ('stepin', 'bytes', 'go', 'quit')" << std::endl;
    std::cin >> ret;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
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
    if(!ret) {
        DWORD error = GetLastError();
        std::cout << "SetThread failed with error: " << error << std::endl;
    } else {
        std::cout << "Single-step flag set successfully" << std::endl;
    }
}

void Debugger::printBytes()
{
    std::vector<unsigned char> bytes = this->pm.readMemoryBytes<uint64_t>(pi.hProcess, reinterpret_cast<LPVOID>(ct.Eip), sizeof(uint64_t));
    std::cout << "Reading: " << reinterpret_cast<LPVOID>(ct.Eip) << std::endl;

    for( auto & b : bytes)
    {
        std::cout << std::hex << static_cast<int>(b) << " ";
    }
    std::cout << std::endl;
}

void Debugger::determinAction(const std::string &cmd)
{
    if(cmd == "q")
        std::exit(0);
    if(cmd == "s")
        SetpInto(currentHandle);
    if(cmd == "b")
        printBytes();
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

            std::cout << "loadDllInfo.lpImageName: " << loadDllInfo.lpImageName << std::endl; 

            void *firstScan = pm.readMemory<void*>(pi.hProcess, loadDllInfo.lpImageName);

            if (firstScan) 
            { 
                //Pretty disgusting, make it so it works for w_chars
                
                std::vector<unsigned char> stringBytes = pm.readMemoryBytes<unsigned char>(pi.hProcess, reinterpret_cast<LPVOID>(firstScan), 255);

                int i = 0;
                bool beforeNull = false;
                for(; i < stringBytes.size(); i++)
                {
                    if (stringBytes[i] == 0)
                    {
                        if(beforeNull == true)
                            break;
                        beforeNull = true;
                        continue;
                    }
                    beforeNull = false;
                }

                std::cout << "i = " << i << std::endl;



                std::string str(stringBytes.begin(), stringBytes.begin() + i);



                str.push_back('\0'); // Null terminate the string

                std::cout << str << std::endl;
            }
            std::wcout << L"DLL Loaded" << std::endl;
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
            THREAD_GET_CONTEXT | THREAD_SET_CONTEXT , // Add THREAD_SUSPEND_RESUME
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
