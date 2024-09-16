#include <windows.h>
#include <pdh.h>
#include <vector>
#include <iostream>
#include <filesystem>
#include <numeric>
#include <mutex>

std::vector<double> cpuLoads;
std::mutex cpuLoadsMutex;

//Performance Data Helper, дескриптор запроса производительности и счетчика процессора
PDH_HQUERY cpuQuery;
PDH_HCOUNTER cpuTotal;

void initCpuLoadCounter() {
    PdhOpenQuery(NULL, NULL, &cpuQuery);
    PdhAddEnglishCounter(cpuQuery, "\\Processor(_Total)\\% Processor Time", NULL, &cpuTotal);
    PdhCollectQueryData(cpuQuery);
}

double getCurrentCpuLoad() {
    PDH_FMT_COUNTERVALUE counterVal;
    PdhCollectQueryData(cpuQuery);
    PdhGetFormattedCounterValue(cpuTotal, PDH_FMT_DOUBLE, NULL, &counterVal);
    return counterVal.doubleValue;
}

struct ThreadParams {
    HANDLE hFile;
    LARGE_INTEGER start;
    LARGE_INTEGER end;
    std::vector<char>* result;
};

DWORD WINAPI ThreadFunc(LPVOID lpParam) {
    auto params = static_cast<ThreadParams*>(lpParam);
    DWORD bytesRead;
    DWORD toRead = params->end.QuadPart - params->start.QuadPart;
    params->result->resize(toRead);
    SetFilePointerEx(params->hFile, params->start, NULL, FILE_BEGIN);

    if (!ReadFile(params->hFile, &(*params->result)[0], toRead, &bytesRead, NULL)) {
        std::cerr << "Failed to read file.\n";
        return 1;
    }
    double cpuLoadAfter = getCurrentCpuLoad();

    {
        std::lock_guard<std::mutex> lock(cpuLoadsMutex);
        cpuLoads.push_back(cpuLoadAfter);
    }
    return 0;
}

int main() {
    initCpuLoadCounter();
    std::string filename;
    std::cout << "Enter the file path: ";
    std::cin >> filename;

    std::filesystem::path filePath(filename);
    if (filePath.extension() != ".txt") {
        std::cerr << "Invalid file type. Please provide a .txt file.\n";
        return 1;
    }

    HANDLE hFile = CreateFile(filename.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        std::cerr << "Unable to open file.\n";
        return 1;
    }

    LARGE_INTEGER fileSize;
    GetFileSizeEx(hFile, &fileSize);

    int threadCountInp;
    std::cout << "Enter the number of threads: ";
    std::cin >> threadCountInp;
    if (threadCountInp < 1 || threadCountInp > 64) {
        std::cerr << "Invalid number of threads. Please enter a number between 1 and 64.\n";
        return 1;
    }

    if (fileSize.QuadPart < threadCountInp) {
        std::cerr << "The file size is less than the number of threads. Please enter a smaller number of threads.\n";
        return 1;
    }

    for (int threadCount = threadCountInp; threadCount <= threadCountInp; ++threadCount) {
        LARGE_INTEGER partSize;
        partSize.QuadPart = fileSize.QuadPart / threadCount;
        std::vector<HANDLE> threads(threadCount);
        LARGE_INTEGER startTime, endTime, freq;
        QueryPerformanceFrequency(&freq);
        QueryPerformanceCounter(&startTime);

        std::vector<ThreadParams*> paramsList(threadCount);

        for (int i = 0; i < threadCount; ++i) {
            ThreadParams* params = new ThreadParams;
            params->hFile = hFile;
            params->start.QuadPart = i * partSize.QuadPart;
            params->end.QuadPart = (i == threadCount - 1) ? fileSize.QuadPart : params->start.QuadPart + partSize.QuadPart;
            params->result = new std::vector<char>(params->end.QuadPart - params->start.QuadPart);

            threads[i] = CreateThread(NULL, 0, ThreadFunc, params, 0, NULL);
            paramsList[i] = params;
        }
        WaitForMultipleObjects(threadCount, &threads[0], TRUE, INFINITE);
        double averageCpuLoad = std::accumulate(cpuLoads.begin(), cpuLoads.end(), 0.0) / cpuLoads.size();
        std::cout << "Average CPU Load: " << averageCpuLoad << "%\n";

        QueryPerformanceCounter(&endTime);
        double elapsedTime = static_cast<double>(endTime.QuadPart - startTime.QuadPart) / freq.QuadPart;
        std::cout << "Threads: " << threadCount << " - Time: " << elapsedTime << " seconds\n";

        for (int i = 0; i < threadCount; ++i) {
            CloseHandle(threads[i]);
            delete paramsList[i]->result;
            delete paramsList[i];
        }
    }

    CloseHandle(hFile);
    return 0;
}

