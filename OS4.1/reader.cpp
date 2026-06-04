#include <windows.h>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <cstring>
#include "common.h"
#pragma comment(lib, "winmm.lib")

int main() {
    SYSTEM_INFO si;
    GetSystemInfo(&si);
    DWORD pageSize   = si.dwPageSize;
    DWORD bufferSize = pageSize * NUM_PAGES;

    // СОЗДАНИЕ ИЛИ ОТКРЫТИЕ ИМЕНОВАННОГО ПРОЕЦИРУЕМОГО ФАЙЛА
    HANDLE hMap = CreateFileMapping(INVALID_HANDLE_VALUE, nullptr, PAGE_READWRITE, 0, bufferSize, MAP_NAME);
    if (!hMap) { printf("CreateFileMapping failed: %lu\n", GetLastError()); return 1; }

    char *buffer = reinterpret_cast<char*>(MapViewOfFile(hMap, FILE_MAP_ALL_ACCESS, 0, 0, bufferSize));
    if (!buffer) { printf("MapViewOfFile failed: %lu\n", GetLastError()); return 1; }

    // БЛОКИРОВКА СТРАНИЦ БУФЕРА В ФИЗИЧЕСКОЙ ПАМЯТИ
    if (!VirtualLock(buffer, bufferSize))
        printf("VirtualLock failed: %lu\n", GetLastError());

    // СОЗДАНИЕ ИЛИ ОТКРЫТИЕ УПРАВЛЯЮЩЕГО ОТОБРАЖЕНИЯ ДЛЯ СЧЕТЧИКА ЧИТАТЕЛЕЙ
    HANDLE hCtrl    = CreateFileMapping(INVALID_HANDLE_VALUE, nullptr, PAGE_READWRITE, 0, sizeof(LONG), CTRL_NAME);
    LONG *readCount = reinterpret_cast<LONG*>(MapViewOfFile(hCtrl, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(LONG)));

    // СОЗДАНИЕ ОБЪЕКТОВ СИНХРОНИЗАЦИИ
    HANDLE rwSem      = CreateSemaphore(nullptr, 1, 1, RW_SEM_NAME);
    HANDLE countMutex = CreateMutex(nullptr, FALSE, COUNT_MUTEX_NAME);

    // ОТКРЫТИЕ ЖУРНАЛЬНОГО ФАЙЛА ТЕКУЩЕГО ПРОЦЕССА
    char logName[64];
    sprintf(logName, "reader_%lu.log", GetCurrentProcessId());
    FILE *log = fopen(logName, "w");

    srand(static_cast<unsigned>(time(nullptr) ^ GetCurrentProcessId()));
    printf("Reader process %lu started\n", GetCurrentProcessId());

    for (int i = 0; i < ITERATIONS; i++) {
        int  page = rand() % NUM_PAGES;
        DWORD dur = 500 + rand() % 1001;

        // СОСТОЯНИЕ НАЧАЛО ОЖИДАНИЯ
        fprintf(log, "%lu BEGIN_WAITING\n", timeGetTime());
        fflush(log);

        // ВХОД В СЕКЦИЮ ЧТЕНИЯ
        WaitForSingleObject(countMutex, INFINITE);
        (*readCount)++;
        if (*readCount == 1) WaitForSingleObject(rwSem, INFINITE);
        ReleaseMutex(countMutex);

        // СОСТОЯНИЕ ЧТЕНИЕ
        fprintf(log, "%lu READ %d\n", timeGetTime(), page);
        fflush(log);
        char sample[64];
        memcpy(sample, buffer + page * pageSize, 63);
        sample[63] = 0;
        printf("[Reader %lu] read page %d content: %s\n", GetCurrentProcessId(), page, sample);
        Sleep(dur);

        // СОСТОЯНИЕ НАЧАЛО ОСВОБОЖДЕНИЯ
        fprintf(log, "%lu BEGIN_RELEASE %d\n", timeGetTime(), page);
        fflush(log);

        // ВЫХОД ИЗ СЕКЦИИ ЧТЕНИЯ
        WaitForSingleObject(countMutex, INFINITE);
        (*readCount)--;
        if (*readCount == 0) ReleaseSemaphore(rwSem, 1, nullptr);
        ReleaseMutex(countMutex);

        Sleep(500 + rand() % 1001);
    }

    fclose(log);
    VirtualUnlock(buffer, bufferSize);
    UnmapViewOfFile(buffer);
    UnmapViewOfFile(readCount);
    CloseHandle(hMap);
    CloseHandle(hCtrl);
    CloseHandle(rwSem);
    CloseHandle(countMutex);
    printf("Reader process %lu finished\n", GetCurrentProcessId());
    return 0;
}
