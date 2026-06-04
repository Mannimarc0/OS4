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

    // СЕМАФОР РЕСУРСА ОБЕСПЕЧИВАЕТ МОНОПОЛЬНЫЙ ДОСТУП ПИСАТЕЛЕЙ
    HANDLE rwSem = CreateSemaphore(nullptr, 1, 1, RW_SEM_NAME);

    // ОТКРЫТИЕ ЖУРНАЛЬНОГО ФАЙЛА ТЕКУЩЕГО ПРОЦЕССА
    char logName[64];
    sprintf(logName, "writer_%lu.log", GetCurrentProcessId());
    FILE *log = fopen(logName, "w");

    srand(static_cast<unsigned>(time(nullptr) ^ GetCurrentProcessId()));
    printf("Writer process %lu started\n", GetCurrentProcessId());

    for (int i = 0; i < ITERATIONS; i++) {
        int  page = rand() % NUM_PAGES;
        DWORD dur = 500 + rand() % 1001;

        // СОСТОЯНИЕ НАЧАЛО ОЖИДАНИЯ
        fprintf(log, "%lu BEGIN_WAITING\n", timeGetTime());
        fflush(log);

        // ПИСАТЕЛЬ ЗАХВАТЫВАЕТ МОНОПОЛЬНЫЙ ДОСТУП К БУФЕРУ
        WaitForSingleObject(rwSem, INFINITE);

        // СОСТОЯНИЕ ЗАПИСЬ
        DWORD t = timeGetTime();
        fprintf(log, "%lu WRITE %d\n", t, page);
        fflush(log);
        char *p = buffer + page * pageSize;
        sprintf(p, "PAGE %d WRITTEN BY PID %lu AT %lu", page, GetCurrentProcessId(), t);
        printf("[Writer %lu] wrote page %d\n", GetCurrentProcessId(), page);
        Sleep(dur);

        // СОСТОЯНИЕ НАЧАЛО ОСВОБОЖДЕНИЯ
        fprintf(log, "%lu BEGIN_RELEASE %d\n", timeGetTime(), page);
        fflush(log);

        // ОСВОБОЖДЕНИЕ РЕСУРСА
        ReleaseSemaphore(rwSem, 1, nullptr);

        Sleep(500 + rand() % 1001);
    }

    fclose(log);
    VirtualUnlock(buffer, bufferSize);
    UnmapViewOfFile(buffer);
    CloseHandle(hMap);
    CloseHandle(rwSem);
    printf("Writer process %lu finished\n", GetCurrentProcessId());
    return 0;
}
