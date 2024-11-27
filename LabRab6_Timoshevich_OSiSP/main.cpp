#include <windows.h>
#include <iostream>
#include <tchar.h>
#include <string>

void PrintSystemInfo() {
    SYSTEM_INFO si;
    GetSystemInfo(&si);

    std::cout << "Hardware Information:" << std::endl;
    std::cout << "  Processor Architecture: ";
    switch (si.wProcessorArchitecture) {
        case PROCESSOR_ARCHITECTURE_AMD64:
            std::cout << "x64 (AMD or Intel)" << std::endl;
            break;
        case PROCESSOR_ARCHITECTURE_INTEL:
            std::cout << "x86" << std::endl;
            break;
        case PROCESSOR_ARCHITECTURE_ARM64:
            std::cout << "ARM64" << std::endl;
            break;
        default:
            std::cout << "Unknown" << std::endl;
    }
    std::cout << "  Number of Processors: " << si.dwNumberOfProcessors << std::endl;
    std::cout << "  Page Size: " << si.dwPageSize << " bytes" << std::endl;
}

void PrintMemoryInfo() {
    MEMORYSTATUSEX memInfo;
    memInfo.dwLength = sizeof(MEMORYSTATUSEX);
    if (GlobalMemoryStatusEx(&memInfo)) {
        std::cout << "\nMemory Information:" << std::endl;
        std::cout << "  Total Physical Memory: " << memInfo.ullTotalPhys / (1024 * 1024) << " MB" << std::endl;
        std::cout << "  Available Physical Memory: " << memInfo.ullAvailPhys / (1024 * 1024) << " MB" << std::endl;
        std::cout << "  Total Virtual Memory: " << memInfo.ullTotalVirtual / (1024 * 1024) << " MB" << std::endl;
        std::cout << "  Available Virtual Memory: " << memInfo.ullAvailVirtual / (1024 * 1024) << " MB" << std::endl;
    } else {
        std::cerr << "Failed to retrieve memory information." << std::endl;
    }
}

void PrintOSVersion() {
    OSVERSIONINFOEX osInfo;
    osInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
    if (GetVersionEx((OSVERSIONINFO*)&osInfo)) {
        std::cout << "\nOS Information:" << std::endl;
        std::cout << "  Major Version: " << osInfo.dwMajorVersion << std::endl;
        std::cout << "  Minor Version: " << osInfo.dwMinorVersion << std::endl;
        std::cout << "  Build Number: " << osInfo.dwBuildNumber << std::endl;
        std::cout << "  Platform: " << (osInfo.dwPlatformId == VER_PLATFORM_WIN32_NT ? "NT" : "Other") << std::endl;
    } else {
        std::cerr << "Failed to retrieve OS version information." << std::endl;
    }
}

int main() {
    std::cout << "System Information Utility" << std::endl;
    PrintSystemInfo();
    PrintMemoryInfo();
    PrintOSVersion();
    return 0;
}
