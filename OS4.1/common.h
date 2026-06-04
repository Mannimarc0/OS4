#ifndef COMMON_H
#define COMMON_H

#include <windows.h>

// КОЛИЧЕСТВО СТРАНИЦ РАВНО СУММЕ ЦИФР НОМЕРА СТУДЕНЧЕСКОГО БИЛЕТА БЕЗ ПЕРВОЙ ЦИФРЫ
#define NUM_PAGES 16

// КОЛИЧЕСТВО РАБОЧИХ ИТЕРАЦИЙ НА ПРОЦЕСС
#define ITERATIONS 10

// ИМЕНА ОБЩИХ ОБЪЕКТОВ
#define MAP_NAME         "RW_BufferMapping"
#define CTRL_NAME        "RW_ControlMapping"
#define RW_SEM_NAME      "RW_ResourceSemaphore"
#define COUNT_MUTEX_NAME "RW_CountMutex"

#endif
