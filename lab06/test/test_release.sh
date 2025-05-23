#!/bin/bash
# Скрипт для тестирования всего конвейера проекта

# 1. Генерация файла с заданным числом записей.
#    В данном случае 131072 записей (131072 кратно 256 и размер файла будет ~2МБ)
echo "Генерация файла с 131072 записями..."
out/release/gen 131072 test/testfile

# 2. Первичный просмотр файла.
#    Выводим только первые 10 строк и последние 10 строк.
echo "Просмотр файла до сортировки (первая часть):"
out/release/view test/testfile | head -n 10
echo "Просмотр файла до сортировки (последняя часть):"
out/release/view test/testfile | tail -n 10

# 3. Сортировка файла.
#    Параметры: memsize = 1048576 байт (1 МБ), granul = 4 (2^4=16 блоков), threads = 8.
echo "Сортировка файла..."
out/release/sort_index 1048576 4 8 test/testfile

# 4. Просмотр файла после сортировки.
echo "Просмотр файла после сортировки (первая часть):"
out/release/view test/testfile | head -n 10
echo "Просмотр файла после сортировки (последняя часть):"
out/release/view test/testfile | tail -n 10
