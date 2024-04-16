#include "Debugger.hpp"

std::wstring TEST_PATH = L"C:\\Windows\\system32\\";
std::wstring TEST_APPLICATOIN = L"notepad.exe";

int main(int argc, wchar_t **argv)
{
    Debugger db((argc < 2) ? TEST_PATH + TEST_APPLICATOIN : argv[1]);

    db.run();
}