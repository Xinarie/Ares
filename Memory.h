#include <iostream>
#include <Windows.h>
#include <TlHelp32.h>

#ifndef ARES_MEMORY_H
#define ARES_MEMORY_H

class Memory {
public:
    Memory(const char* processName) {
        processId = GetProcess(processName);
        if (processId == 0) {
            std::cerr << "Failed to find process with name: " << processName << std::endl;
            exit(EXIT_FAILURE);
        }

        processHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processId);
        if (processHandle == NULL) {
            std::cerr << "Failed to open process with ID " << processId << std::endl;
            exit(EXIT_FAILURE);
        }
    }

    ~Memory() {
        if (processHandle != NULL) {
            CloseHandle(processHandle);
        }
    }

    template <typename T>
    T Read(DWORD address) {
        T buffer;
        ReadProcessMemory(processHandle, reinterpret_cast<LPCVOID>(address), &buffer, sizeof(T), NULL);
        return buffer;
    }

    template <typename T>
    void Write(DWORD address, T value) {
        WriteProcessMemory(processHandle, reinterpret_cast<LPVOID>(address), &value, sizeof(T), NULL);
    }

    DWORD GetModule(const char* moduleName) {
        MODULEENTRY32 moduleEntry;
        moduleEntry.dwSize = sizeof(MODULEENTRY32);

        HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, processId);
        if (snapshot == INVALID_HANDLE_VALUE) {
            return 0;
        }

        if (Module32First(snapshot, &moduleEntry)) {
            do {
                if (_stricmp(moduleEntry.szModule, moduleName) == 0) {
                    CloseHandle(snapshot);
                    return reinterpret_cast<uintptr_t>(moduleEntry.modBaseAddr);
                }
            } while (Module32Next(snapshot, &moduleEntry));
        }

        CloseHandle(snapshot);
        return 0;
    }

private:
    DWORD processId;
    HANDLE processHandle;

    DWORD GetProcess(const char* processName) {
        HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        if (snapshot == INVALID_HANDLE_VALUE) {
            return 0;
        }

        PROCESSENTRY32 entry;
        entry.dwSize = sizeof(PROCESSENTRY32);

        if (Process32First(snapshot, &entry)) {
            do {
                if (_stricmp(entry.szExeFile, processName) == 0) {
                    CloseHandle(snapshot);
                    return entry.th32ProcessID;
                }
            } while (Process32Next(snapshot, &entry));
        }

        CloseHandle(snapshot);
        return 0;
    }
};

#endif //ARES_MEMORY_H
