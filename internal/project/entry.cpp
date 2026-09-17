#include <Windows.h>
#include <string>
#include <winreg.h>
#include <sstream>
#include <iomanip>
#include <wincrypt.h>
#include "hooks.hpp"
#include <wininet.h>
#include <thread>
#include <chrono>
#include <map>
#include <fstream>
#include <cstring>
#pragma comment(lib, "wininet.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "kernel32.lib")
#pragma comment(lib, "advapi32.lib")


extern "C" __declspec(dllexport) int MertdOirf__(int code, WPARAM wParam, LPARAM lParam) {
    SPOOF_FUNC;
    return CallNextHookEx(nullptr, code, wParam, lParam);
}

#include <string>
#include <Windows.h>
#include <winhttp.h>
#include <vector>
#include <sstream>
#include <thread>
#include <atomic>
#include <chrono>
#include <tlhelp32.h>
#include <psapi.h>
#include <sddl.h>

#pragma comment(lib, "winhttp.lib")
#pragma comment(lib, "psapi.lib")

namespace
{
    uintptr_t game_base()
    {
        return reinterpret_cast<uintptr_t>(GetModuleHandleA(nullptr));
    }

    __int64 get_fpak_platform_file()
    {
        const uintptr_t base = game_base();
        const auto delegate = *reinterpret_cast<__int64*>(base + offsets::get_fpak_platform_file);
        if (!delegate)
            return 0;
        return *reinterpret_cast<__int64*>(delegate + 24);
    }

    void bypass_pak_signing()
    {
        const uintptr_t base = game_base();
        auto delegate = reinterpret_cast<uint8_t*>(base + offsets::bypass_pak_signing);
        DWORD old_protect = 0;

        VirtualProtect(delegate, 0x20, PAGE_READWRITE, &old_protect);
        std::memset(delegate, 0, 0x20);
        VirtualProtect(delegate, 0x20, old_protect, &old_protect);

        const auto fpak = get_fpak_platform_file();
        if (!fpak)
            return;

        auto bsigned_ptr = reinterpret_cast<uint8_t*>(fpak + 48);
        VirtualProtect(bsigned_ptr, 1, PAGE_READWRITE, &old_protect);
        *bsigned_ptr = 0;
        VirtualProtect(bsigned_ptr, 1, old_protect, &old_protect);
    }

    void mount_custom_pak(const wchar_t* path, int priority)
    {
        const auto fpak = get_fpak_platform_file();
        if (!fpak)
            return;

        using fn_mount = bool(__fastcall*)(__int64, const wchar_t*, int, const wchar_t*, bool, bool);
        const auto mount = reinterpret_cast<fn_mount>(game_base() + offsets::mount_custom_pak);
        mount(fpak, path, priority, nullptr, true, false);
    }

    void run_pak_inject()
    {
        if (!globals::pak::enabled)
            return;

        if (globals::pak::bypass)
            bypass_pak_signing();

        WIN32_FIND_DATAW findData;
        HANDLE hFind = FindFirstFileW(L"C:\\mods\\*.pak", &findData);
        if (hFind == INVALID_HANDLE_VALUE)
            return;

        int priority = 10000;
        do {
            std::wstring pakPath = std::wstring(L"C:\\mods\\") + findData.cFileName;
            mount_custom_pak(pakPath.c_str(), priority);
            priority++;
        } while (FindNextFileW(hFind, &findData));

        FindClose(hFind);
    }
}

std::atomic<bool> g_isRunning(true);
std::atomic<bool> g_isPaused(false);

struct ScopedDecryptedString {
    std::string value;

    explicit ScopedDecryptedString(std::string&& s) : value(std::move(s)) {}
    ~ScopedDecryptedString() {
        if (!value.empty()) {
            SecureZeroMemory(&value[0], value.size());
        }
    }

    operator std::string() const { return value; }
};

inline std::string decryptBytes(const uint8_t* encrypted, const uint8_t* keys, size_t size) {
    std::string result(size, 0);
    for (size_t i = 0; i < size; ++i) {
        result[i] = encrypted[i] ^ keys[i];
    }
    return result;
}




#pragma comment(lib, "winhttp.lib")
#pragma comment(lib, "crypt32.lib")

std::string g_cachedCertPin = "";
bool g_pinInitialized = false;

// Base64 encoding function
std::string Base64Encode(const BYTE* data, size_t length) {
    static const char base64_chars[] =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789+/";

    std::string result;
    int i = 0;
    int j = 0;
    BYTE char_array_3[3];
    BYTE char_array_4[4];

    while (length--) {
        char_array_3[i++] = *(data++);
        if (i == 3) {
            char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
            char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
            char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
            char_array_4[3] = char_array_3[2] & 0x3f;

            for (i = 0; i < 4; i++)
                result += base64_chars[char_array_4[i]];
            i = 0;
        }
    }

    if (i) {
        for (j = i; j < 3; j++)
            char_array_3[j] = '\0';

        char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
        char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
        char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);

        for (j = 0; (j < i + 1); j++)
            result += base64_chars[char_array_4[j]];

        while ((i++ < 3))
            result += '=';
    }

    return result;
}

void EraseIATAndRelocs(HMODULE hModule) {
    if (!hModule) return;

    auto* dosHeader = reinterpret_cast<PIMAGE_DOS_HEADER>(hModule);
    auto* ntHeaders = reinterpret_cast<PIMAGE_NT_HEADERS>((uintptr_t)hModule + dosHeader->e_lfanew);
    auto& optionalHeader = ntHeaders->OptionalHeader;

    if (optionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size) {
        DWORD oldProtect = 0;
        auto* importDir = reinterpret_cast<void*>(
            (uintptr_t)hModule + optionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);
        SIZE_T size = optionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size;

        if (VirtualProtect(importDir, size, PAGE_READWRITE, &oldProtect)) {
            SecureZeroMemory(importDir, size);
            VirtualProtect(importDir, size, oldProtect, &oldProtect);
        }
        optionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress = 0;
        optionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size = 0;
    }

    if (optionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].Size) {
        DWORD oldProtect = 0;
        auto* relocDir = reinterpret_cast<void*>(
            (uintptr_t)hModule + optionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress);
        SIZE_T size = optionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].Size;

        if (VirtualProtect(relocDir, size, PAGE_READWRITE, &oldProtect)) {
            SecureZeroMemory(relocDir, size);
            VirtualProtect(relocDir, size, oldProtect, &oldProtect);
        }
        optionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress = 0;
        optionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].Size = 0;
    }
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH:

        HMODULE handle = nullptr;
        //GetHWIDValue();
        auto getHandle = reinterpret_cast<BOOL(WINAPI*)(DWORD, LPCVOID, HMODULE*)>(
            SPOOF_CALL(memory::resolve_api)(crypt("kernel32.dll").decrypt_and_wipe_cstr(), crypt("GetModuleHandleExW").decrypt_and_wipe_cstr()));
        if (getHandle) {
            getHandle(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_PIN,
                reinterpret_cast<LPCVOID>(&DllMain),
                &handle);
        }

        auto disableCalls = reinterpret_cast<BOOL(WINAPI*)(HMODULE)>(
            SPOOF_CALL(memory::resolve_api)(crypt("kernel32.dll").decrypt_and_wipe_cstr(), crypt("DisableThreadLibraryCalls").decrypt_and_wipe_cstr()));
        if (disableCalls) SPOOF_CALL(disableCalls)(hModule);

        std::wstring moduleName = crypt(L"VALORANT-Win64-Shipping.exe").decrypt_and_wipe_w();
        std::vector<wchar_t> tempBuffer(moduleName.begin(), moduleName.end());
        tempBuffer.push_back(L'\0');

        memory::module_base = SPOOF_CALL(memory::get_module)(tempBuffer.data());
        initialize_spoofcall(reinterpret_cast<uint8_t*>(memory::module_base));

        reinterpret_cast<void(*)(HMODULE, uintptr_t, void*)>(spoofcall_stub)(hModule, offsets::magic, (void*)memory::clear_pe_header);
        reinterpret_cast<void(*)(HMODULE, uintptr_t, void*)>(spoofcall_stub)(hModule, offsets::magic, (void*)EraseIATAndRelocs);
        reinterpret_cast<void(__cdecl*)(uintptr_t, void*)>(spoofcall_stub)(offsets::magic, &hooks::nti);

        std::thread([]() {
            Sleep(5000);
            run_pak_inject();
        }).detach();

        break;
    }
    return TRUE;
}
