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
        int rpm = ReadProcessMemory(proc, addr, &ret, sizeof(T), NULL);
        std::cout << "RPM return: " << rpm << std::endl;
        if(rpm)
            return ret;
        return NULL;
    }

    template<typename T>
    std::vector<unsigned char> readMemoryBytes(HANDLE proc, LPVOID addr, size_t numBytes)
    {
        std::vector<unsigned char> bytes(numBytes);

        SIZE_T bytesRead;
        ReadProcessMemory(proc, addr, bytes.data(), numBytes, &bytesRead);

        if (bytesRead < numBytes) {
            bytes.resize(bytesRead);
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