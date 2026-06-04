#ifndef COMMON_H
#define COMMON_H

#include <windows.h>

// КОЛИЧЕСТВО СТРАНИЦ РАВНО СУММЕ ЦИФР НОМЕРА СТУДЕНЧЕСКОГО БИЛЕТА БЕЗ ПЕРВОЙ ЦИФРЫ
// 431624 -> 3+1+6+2+4 = 16
#define NUM_PAGES 16

// КОЛИЧЕСТВО РАБОЧИХ ИТЕРАЦИЙ НА ПРОЦЕСС
#define ITERATIONS 10

// ИМЯ ОБЩЕГО ПРОЕЦИРУЕМОГО ФАЙЛА
#define MAP_NAME "RW_BufferMapping"

// ПРЕФИКС ИМЕНИ СЕМАФОРА СТРАНИЦЫ К ПРЕФИКСУ ДОБАВЛЯЕТСЯ НОМЕР СТРАНИЦЫ
#define SEM_PREFIX "RW_PageSem_"

#endif
