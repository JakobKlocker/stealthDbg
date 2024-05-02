#include <Windows.h>
#include <vector>
#include <iostream>

class ProcessMemory
{
    public:
    template<typename T>
    T readMemory(HANDLE proc, LPVOID addr)
    {
        T ret;
        ReadProcessMemory(proc, addr, &ret, sizeof(T), NULL);
        return ret;
    }

    template<typename T>
    std::vector<unsigned char> readMemoryBytes(HANDLE proc, LPVOID addr)
    {
        std::vector<unsigned char> bytes(sizeof(T));
    
        for (size_t i = 0; i < sizeof(T); ++i)
        {
            ReadProcessMemory(proc, static_cast<LPBYTE>(addr) + i, &bytes[i], sizeof(unsigned char), nullptr);
        }
    
        return bytes;
    }

    template<typename T>
    T writeMemory(HANDLE proc, LPVOID addr, T var)
    {
        WriteProcessMemory(proc, addr, &var, sizeof(T), NULL);
    }

    template<typename T>
    DWORD protectMemory(HANDLE proc, LPVOID addr, DWORD prot)
    {
        DWORD oldProtect;
        VirtualProtectEx(proc, addr, sizeof(T), prot, &oldProtect);
        return oldProtect;
    }
};