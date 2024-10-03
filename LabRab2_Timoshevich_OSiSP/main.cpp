#include <windows.h>
#include <stdio.h>
#include <string.h>

#define BUFFER_SIZE 1048576
#define NUM_THREADS 10

typedef struct {
    char *buffer;
    DWORD bytesRead;
    int letter_count[26];
} ThreadData;

double get_time_in_seconds() {
    LARGE_INTEGER frequency, currentTime;
    QueryPerformanceFrequency(&frequency);
    QueryPerformanceCounter(&currentTime);
    return (double)currentTime.QuadPart / frequency.QuadPart;
}

void process_buffer(const char *buffer, DWORD bytesRead, int letter_count[26]) {
    for (DWORD i = 0; i < bytesRead; i++) {
        char ch = buffer[i];
        if (ch >= 'A' && ch <= 'Z') {
            letter_count[ch - 'A']++;
        } else if (ch >= 'a' && ch <= 'z') {
            letter_count[ch - 'a']++;
        }
    }
}

DWORD WINAPI thread_function(LPVOID param) {
    ThreadData *data = (ThreadData *)param;
    process_buffer(data->buffer, data->bytesRead, data->letter_count);
    return 0;
}

void memory_mapped_file_processing(const char *filename) {
    HANDLE hFile = CreateFile(
            filename,
            GENERIC_READ,
            FILE_SHARE_READ,
            NULL,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            NULL
    );

    if (hFile == INVALID_HANDLE_VALUE) {
        printf("Error opening file: %d\n", GetLastError());
        return;
    }

    HANDLE hMapping = CreateFileMapping( hFile,NULL,PAGE_READONLY,0,0,NULL);

    if (hMapping == NULL) {
        printf("Error creating file mapping: %d\n", GetLastError());
        CloseHandle(hFile);
        return;
    }

    char *mappedView = (char *)MapViewOfFile(hMapping,FILE_MAP_READ,0,0,0);

    if (mappedView == NULL) {
        printf("Error mapping view of file: %d\n", GetLastError());
        CloseHandle(hMapping);
        CloseHandle(hFile);
        return;
    }

    DWORD fileSize = GetFileSize(hFile, NULL);
    int totalLetterCount[26] = { 0 };
    DWORD bytesPerThread = fileSize / NUM_THREADS;
    HANDLE threads[NUM_THREADS];
    ThreadData threadData[NUM_THREADS];

    double startTime = get_time_in_seconds();

    for (int i = 0; i < NUM_THREADS; i++) {
        threadData[i].buffer = &mappedView[i * bytesPerThread];
        threadData[i].bytesRead = (i == NUM_THREADS - 1) ? (fileSize - (i * bytesPerThread)) : bytesPerThread;
        ZeroMemory(threadData[i].letter_count, sizeof(threadData[i].letter_count));
        threads[i] = CreateThread(NULL, 0, thread_function, &threadData[i], 0, NULL);
    }

    WaitForMultipleObjects(NUM_THREADS, threads, TRUE, INFINITE);

    for (int i = 0; i < NUM_THREADS; i++) {
        for (int j = 0; j < 26; j++) {
            totalLetterCount[j] += threadData[i].letter_count[j];
        }
    }

    double endTime = get_time_in_seconds();
    UnmapViewOfFile(mappedView);
    CloseHandle(hMapping);
    CloseHandle(hFile);

    printf("\nMemory-mapped approach with %d threads - Statistics of occurrences of English letters:\n", NUM_THREADS);
    for (int i = 0; i < 26; i++) {
        printf("%c: %d\n", 'A' + i, totalLetterCount[i]);
    }
    printf("Running time of the memory-mapped approach: %.2f seconds\n", endTime - startTime);
}

void traditional_file_processing(const char *filename) {
    HANDLE hFile;
    DWORD bytesRead = 0;
    int letter_count[26] = { 0 };
    double startTime, endTime;

    hFile = CreateFile(filename,GENERIC_READ,0,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);

    if (hFile == INVALID_HANDLE_VALUE) {
        printf("Error opening file: %d\n", GetLastError());
        return;
    }
    startTime = get_time_in_seconds();
    char buffer[BUFFER_SIZE];
    while (TRUE) {
        BOOL result = ReadFile(hFile, buffer, sizeof(buffer), &bytesRead, NULL);
        if (!result || bytesRead == 0) {
            break;
        }
        process_buffer(buffer, bytesRead, letter_count);
    }

    endTime = get_time_in_seconds();
    CloseHandle(hFile);

    printf("\nTraditional approach - Statistics of occurrences of English letters:\n");
    for (int i = 0; i < 26; i++) {
        printf("%c: %d\n", 'A' + i, letter_count[i]);
    }
    printf("Running time of the traditional approach: %.2f seconds\n", endTime - startTime);
}

void multithreaded_traditional_file_processing(const char *filename) {
    HANDLE hFile;
    DWORD bytesRead = 0;
    int totalLetterCount[26] = { 0 };
    double startTime, endTime;

    hFile = CreateFile(filename,GENERIC_READ,0,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);

    if (hFile == INVALID_HANDLE_VALUE) {
        printf("Error opening file: %d\n", GetLastError());
        return;
    }

    DWORD fileSize = GetFileSize(hFile, NULL);
    DWORD bytesPerThread = fileSize / NUM_THREADS;
    HANDLE threads[NUM_THREADS];
    ThreadData threadData[NUM_THREADS];

    startTime = get_time_in_seconds();

    for (int i = 0; i < NUM_THREADS; i++) {
        char *buffer = new char[bytesPerThread];
        ReadFile(hFile, buffer, bytesPerThread, &bytesRead, NULL);
        threadData[i].buffer = buffer;
        threadData[i].bytesRead = bytesRead;
        ZeroMemory(threadData[i].letter_count, sizeof(threadData[i].letter_count));
        threads[i] = CreateThread(NULL, 0, thread_function, &threadData[i], 0, NULL);
    }

    WaitForMultipleObjects(NUM_THREADS, threads, TRUE, INFINITE);

    for (int i = 0; i < NUM_THREADS; i++) {
        for (int j = 0; j < 26; j++) {
            totalLetterCount[j] += threadData[i].letter_count[j];
        }
        delete[] threadData[i].buffer;
    }

    endTime = get_time_in_seconds();
    CloseHandle(hFile);

    printf("\nMultithreaded traditional approach - Statistics of occurrences of English letters:\n");
    for (int i = 0; i < 26; i++) {
        printf("%c: %d\n", 'A' + i, totalLetterCount[i]);
    }
    printf("Running time of the multithreaded traditional approach: %.2f seconds\n", endTime - startTime);
}

int main() {
    const char *filename = "D:\\TEST\\text.txt";
    traditional_file_processing(filename);
    memory_mapped_file_processing(filename);
    multithreaded_traditional_file_processing(filename);
    return 0;
}


