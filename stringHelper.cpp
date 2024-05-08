#include "stringHelper.hpp"

std::wstring bytesToWideString(const std::vector<unsigned char>& bytes) {
    std::wstring wideString;

    for (size_t i = 0; i < bytes.size(); i += sizeof(wchar_t)) {
        wchar_t wideChar = *reinterpret_cast<const wchar_t*>(&bytes[i]);

        if (wideChar == L'\0') {
            if (i + sizeof(wchar_t) < bytes.size() && *reinterpret_cast<const wchar_t*>(&bytes[i + sizeof(wchar_t)]) == L'\0') {
                break;
            }
        }
        wideString.push_back(wideChar);
    }
    return wideString;
}

std::string bytesToString(const std::vector<unsigned char>& bytes) {
    std::string str;

    for (size_t i = 0; i < bytes.size(); i++) 
    {
        char ch = *reinterpret_cast<const char*>(&bytes[i]);

        if (ch == '0') 
            break;
        str.push_back(ch);
    }
    return str;
}
