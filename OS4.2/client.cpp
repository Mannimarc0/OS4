#include <windows.h>
#include <cstdio>

#define PIPE_NAME "\\\\.\\pipe\\mypipe"
#define BUF_SIZE 512

char g_buf[BUF_SIZE];

// ПРОЦЕДУРА ЗАВЕРШЕНИЯ ВЫЗЫВАЕТСЯ КОГДА АСИНХРОННОЕ ЧТЕНИЕ ЗАКАНЧИВАЕТСЯ
void CALLBACK ReadComplete(DWORD err, DWORD bytes, LPOVERLAPPED) {
    if (err == 0 && bytes > 0) {
        g_buf[bytes] = 0;
        printf("Message from server: %s\n", g_buf);
        printf("Received %lu bytes\n", bytes);
    } else {
        printf("Read completed with error %lu\n", err);
    }
}

int main() {
    HANDLE hPipe = INVALID_HANDLE_VALUE;
    int choice;

    for (;;) {
        printf("\n=== CLIENT MENU ===\n");
        printf("1. Connect to pipe\n");
        printf("2. Receive message\n");
        printf("3. Exit\n");
        printf("Choice: ");
        if (scanf("%d", &choice) != 1) break;
        while (getchar() != '\n') {}  // ОЧИСТКА БУФЕРА ВВОДА

        if (choice == 1) {
            // ПОДКЛЮЧЕНИЕ К ИМЕНОВАННОМУ КАНАЛУ
            hPipe = CreateFile(PIPE_NAME, GENERIC_READ | GENERIC_WRITE, 0, nullptr,
                OPEN_EXISTING, FILE_FLAG_OVERLAPPED, nullptr);
            if (hPipe == INVALID_HANDLE_VALUE) {
                printf("CreateFile failed: %lu\n", GetLastError());
            } else {
                DWORD mode = PIPE_READMODE_MESSAGE;
                SetNamedPipeHandleState(hPipe, &mode, nullptr, nullptr);
                printf("Connected to pipe\n");
            }
        }
        else if (choice == 2) {
            if (hPipe == INVALID_HANDLE_VALUE) { printf("Connect to the pipe first\n"); continue; }
            OVERLAPPED ov{};

            // АСИНХРОННОЕ ЧТЕНИЕ ИЗ КАНАЛА С ПРОЦЕДУРОЙ ЗАВЕРШЕНИЯ
            if (!ReadFileEx(hPipe, g_buf, BUF_SIZE - 1, &ov, ReadComplete)) {
                printf("ReadFileEx failed: %lu\n", GetLastError());
                continue;
            }
            // ПЕРЕХОД В РЕЖИМ ОПОВЕЩАЕМОГО ОЖИДАНИЯ ДЛЯ ЗАПУСКА ПРОЦЕДУРЫ ЗАВЕРШЕНИЯ
            SleepEx(INFINITE, TRUE);
        }
        else if (choice == 3) {
            break;
        }
        else {
            printf("Unknown option\n");
        }
    }

    if (hPipe != INVALID_HANDLE_VALUE) CloseHandle(hPipe);
    return 0;
}
