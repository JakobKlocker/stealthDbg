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
    // HANDLE processHandle = OpenProcess(PROCESS_VM_READ, FALSE, this->dEventInfo.dwProcessId);
    std::vector<unsigned char> bytes = this->pm.readMemoryBytes<uint64_t>(pi.hProcess, reinterpret_cast<LPVOID>(ct.Eip));

    std::cout << "Reading: " << reinterpret_cast<LPVOID>(ct.Eip) << std::endl;

    for( auto & b : bytes)
    {
        std::cout << std::hex << static_cast<int>(b) << " ";
    }
    std::cout << std::endl;
}

void Debugger::determinAction(const std::string &cmd)
{
    if(cmd == "quit")
        std::exit(0);
    if(cmd == "stepin")
        SetpInto(currentHandle);
    if(cmd == "bytes")
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
            // Process the exception code. When handling
            // exceptions, remember to set the continuation
            // status parameter (dwContinueStatus). This value
            // is used by the ContinueDebugEvent function.

            switch (this->dEventInfo.u.Exception.ExceptionRecord.ExceptionCode)
            {

            case EXCEPTION_SINGLE_STEP:
                std::cout << "Single Step Called" << std::endl;
                std::cout << "RIP: " << std::hex << ct.Eip << std::endl;
                
                break;

            case EXCEPTION_ACCESS_VIOLATION:
                // First chance: Pass this on to the system.
                // Last chance: Display an appropriate error.
                break;

            case EXCEPTION_BREAKPOINT:
                // First chance: Display the current
                // instruction and register values.
                break;

            case EXCEPTION_DATATYPE_MISALIGNMENT:
                // First chance: Pass this on to the system.
                // Last chance: Display an appropriate error.
                break;



            case DBG_CONTROL_C:
                // First chance: Pass this on to the system.
                // Last chance: Display an appropriate error.
                break;

            default:
                // Handle other exceptions.
                break;
            }

            break;

        case CREATE_THREAD_DEBUG_EVENT:
            // As needed, examine or change the thread's registers
            // with the GetThreadContext and SetThreadContext functions;
            // and suspend and resume thread execution with the
            // SuspendThread and ResumeThread functions.
            std::wcout << L"Create Thread" << std::endl;

            break;

        case CREATE_PROCESS_DEBUG_EVENT:
            // As needed, examine or change the registers of the
            // process's initial thread with the GetThreadContext and
            // SetThreadContext functions; read from and write to the
            // process's virtual memory with the ReadProcessMemory and
            // WriteProcessMemory functions; and suspend and resume
            // thread execution with the SuspendThread and ResumeThread
            // functions. Be sure to close the handle to the process image
            // file with CloseHandle.

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
            // Read the debugging information included in the newly
            // loaded DLL. Be sure to close the handle to the loaded DLL
            // with CloseHandle.
            std::wcout << L"DLL Loaded" << std::endl;
            break;

        case UNLOAD_DLL_DEBUG_EVENT:
            std::wcout << L"DLL Unloaded" << std::endl;
            // Display a message that the DLL has been unloaded.

            break;

        case OUTPUT_DEBUG_STRING_EVENT:
            // Display the output debugging string.

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
