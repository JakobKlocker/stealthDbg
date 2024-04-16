#include "Debugger.hpp"

Debugger::Debugger(const std::wstring target) : targetPath(target)
{
    std::wcout << "Target: " << target << std::endl;
}

void Debugger::run()
{
    STARTUPINFOW si;
    PROCESS_INFORMATION pi;
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

            case EXCEPTION_SINGLE_STEP:
                // First chance: Update the display of the
                // current instruction and register values.
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

        HANDLE handle = OpenThread(
            THREAD_GET_CONTEXT , // Add THREAD_SUSPEND_RESUME
            FALSE,
            this->dEventInfo.dwThreadId);

        std::cout << "Handle: " << handle << std::endl;

        if (handle != NULL)
        {
			CONTEXT ct;
			memset(&ct, 0, sizeof(ct));
			ct.ContextFlags = CONTEXT_ALL;

            DWORD tmp = GetThreadContext(handle, &ct);
            std::cout << "GetThreadContextMsg: " << tmp << std::endl;
            if (tmp)
            {
                std::cout << "RIP: " << ct.Esp << std::endl;
            }
            else // GetThreadContext failed
            {
                DWORD error = GetLastError();
                std::cerr << "GetThreadContext failed with error code: " << error << std::endl;
            }

            CloseHandle(handle);  // Close the thread handle
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
