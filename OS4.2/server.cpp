#include <windows.h>
#include <cstdio>
#include <cstring>

#define PIPE_NAME "\\\\.\\pipe\\mypipe"
#define BUF_SIZE 512

int main() {
    HANDLE hPipe  = INVALID_HANDLE_VALUE;
    HANDLE hEvent = nullptr;
    int  choice;
    char buf[BUF_SIZE];

    for (;;) {
        printf("\n=== SERVER MENU ===\n");
        printf("1. Create named pipe and event\n");
        printf("2. Wait for client connection\n");
        printf("3. Send message to client\n");
        printf("4. Disconnect client\n");
        printf("5. Exit\n");
        printf("Choice: ");
        if (scanf("%d", &choice) != 1) break;
        while (getchar() != '\n') {}  // ОЧИСТКА БУФЕРА ВВОДА

        if (choice == 1) {
            // СОЗДАНИЕ ИМЕНОВАННОГО КАНАЛА С ФЛАГОМ АСИНХРОННОГО ВВОДА ВЫВОДА
            hPipe = CreateNamedPipe(PIPE_NAME,
                PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED,
                PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
                1, BUF_SIZE, BUF_SIZE, 0, nullptr);
            if (hPipe == INVALID_HANDLE_VALUE)
                printf("CreateNamedPipe failed: %lu\n", GetLastError());
            else
                printf("Named pipe created\n");

            // СОЗДАНИЕ ОБЪЕКТА СОБЫТИЕ ДЛЯ АСИНХРОННЫХ ОПЕРАЦИЙ
            hEvent = CreateEvent(nullptr, TRUE, FALSE, nullptr);
            if (!hEvent)
                printf("CreateEvent failed: %lu\n", GetLastError());
            else
                printf("Event object created\n");
        }
        else if (choice == 2) {
            if (hPipe == INVALID_HANDLE_VALUE) { printf("Create the pipe first\n"); continue; }
            OVERLAPPED ov{};
            ov.hEvent = hEvent;
            ResetEvent(hEvent);

            // ОЖИДАНИЕ ПОДКЛЮЧЕНИЯ КЛИЕНТА
            BOOL  ok  = ConnectNamedPipe(hPipe, &ov);
            DWORD err = GetLastError();
            if (!ok && err == ERROR_IO_PENDING) {
                printf("Waiting for client...\n");
                WaitForSingleObject(hEvent, INFINITE);
                printf("Client connected\n");
            } else if (!ok && err == ERROR_PIPE_CONNECTED) {
                printf("Client already connected\n");
            } else if (ok) {
                printf("Client connected\n");
            } else {
                printf("ConnectNamedPipe failed: %lu\n", err);
            }
        }
        else if (choice == 3) {
            if (hPipe == INVALID_HANDLE_VALUE) { printf("Create the pipe first\n"); continue; }
            printf("Enter message: ");
            if (!fgets(buf, BUF_SIZE, stdin)) continue;

            // УДАЛЕНИЕ СИМВОЛА НОВОЙ СТРОКИ
            size_t len = strlen(buf);
            if (len && buf[len - 1] == '\n') { buf[len - 1] = 0; len--; }

            OVERLAPPED ov{};
            ov.hEvent = hEvent;
            ResetEvent(hEvent);

            // АСИНХРОННАЯ ЗАПИСЬ СООБЩЕНИЯ В КАНАЛ
            BOOL  ok  = WriteFile(hPipe, buf, static_cast<DWORD>(len + 1), nullptr, &ov);
            DWORD err = GetLastError();
            if (!ok && err == ERROR_IO_PENDING) {
                // ОЖИДАНИЕ ЗАВЕРШЕНИЯ ОПЕРАЦИИ ЗАПИСИ
                WaitForSingleObject(hEvent, INFINITE);
            } else if (!ok) {
                printf("WriteFile failed: %lu\n", err);
                continue;
            }
            DWORD written = 0;
            GetOverlappedResult(hPipe, &ov, &written, FALSE);
            printf("Sent %lu bytes\n", written);
        }
        else if (choice == 4) {
            if (hPipe == INVALID_HANDLE_VALUE) { printf("Create the pipe first\n"); continue; }
            // ОТКЛЮЧЕНИЕ КЛИЕНТА ОТ КАНАЛА
            if (DisconnectNamedPipe(hPipe))
                printf("Client disconnected\n");
            else
                printf("DisconnectNamedPipe failed: %lu\n", GetLastError());
        }
        else if (choice == 5) {
            break;
        }
        else {
            printf("Unknown option\n");
        }
    }

    if (hPipe != INVALID_HANDLE_VALUE) CloseHandle(hPipe);
    if (hEvent) CloseHandle(hEvent);
    return 0;
}
