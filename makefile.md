# Задание

Написать на С++ класс быстрого прямого и обратного преобразования Фурье комплексных значений с возможной длиной преобразования кратной 2, 3, 5.
Запустить для случайных комплексных входных данных сначала прямое, а потом обратное преобразование Фурье.
Сравнить ошибку между входными и выходными данными.


# Используемые ресурсы

The Cooley-Tukey Algorithm

[algorithm-archive](https://www.algorithm-archive.org/contents/cooley_tukey/cooley_tukey.html)

[wikipedia](https://en.wikipedia.org/wiki/Cooley%E2%80%93Tukey_FFT_algorithm)

# Руководство по сборке проекта FFT

## Структура проекта

```
project/
├── fft.cpp            # класс FFT + генератор CSV-таблиц
├── error_rate.cpp     # анализатор ошибок по CSV-таблицам
├── CMakeLists.txt     # конфигурация сборки CMake
├── Makefile           # удобная обёртка над CMake
└── README.md        # этот файл
```

Система сборки двухуровневая: **Makefile** служит удобным интерфейсом, а
всю реальную компиляцию выполняет **CMake**. Использовать напрямую можно
любой из двух уровней — оба описаны ниже.

---

## Требования

| Инструмент | Минимальная версия |
|---|---|
| GCC или Clang | поддержка C++17 |
| CMake | 3.14 |
| Make | любая |


---

## Быстрый старт

```bash
make        # скомпилировать оба бинарника
make run    # скомпилировать → запустить fft → запустить error_rate
```

---

## Использование Makefile

Все команды вводятся из корневой директории проекта (там, где лежит `Makefile`).

### Сборка

```bash
make
```

Выполняет Release-сборку обоих бинарников с флагом `-O2`.
После успешной компиляции в директории `build/` появляются:

```
build/
├── fft           # генератор CSV
└── error_rate    # анализатор ошибок
```

При первом запуске CMake автоматически создаёт директорию `build/` и
выполняет конфигурацию проекта. При повторных вызовах `make` пересобираются
только изменившиеся файлы.

---

### Debug-сборка

```bash
make debug
```

Компилирует с флагами `-O0 -g` (без оптимизаций, с отладочными символами).
Полезно при работе с отладчиком (gdb, lldb) или профилировщиком.
Также генерирует `build/compile_commands.json` для поддержки clangd и IDE.

---

### Запуск программ

#### Запустить всё сразу

```bash
make run
```

Последовательно выполняет три шага:
1. компиляция (если бинарники устарели);
2. запуск `fft` — генерирует CSV-таблицы в директории `build/`;
3. запуск `error_rate` — считывает все CSV и выводит ошибки в терминал.

#### Только генерация CSV

```bash
make run_fft
```

Запускает `build/fft`, который создаёт файлы:

```
build/fft_8.csv
build/fft_12.csv
build/fft_16.csv
build/fft_18.csv
build/fft_20.csv
build/fft_30.csv
build/fft_60.csv
build/fft_120.csv
build/fft_360.csv
build/fft_1800.csv
```

Каждый файл содержит результаты прямого и обратного ДПФ для одной длины N.

#### Только анализ ошибок

```bash
make run_errors
```

Запускает `build/error_rate` по всем десяти CSV-файлам и выводит для каждого:

```
----------------------------------------
 File: fft_360.csv
----------------------------------------
 Data rows (N):                 360
 Max absolute error:            2.482883e-15
 Mean absolute error (MAE):     6.107429e-16
 Root mean square error (RMSE): 7.077950e-16
 Max relative error:            1.024054e-14
```

---

### Очистка и пересборка

```bash
make clean      # удалить директорию build/
make rebuild    # clean + полная пересборка с нуля
```

`make clean` полностью удаляет `build/`, включая все CSV-файлы и бинарники.
`make rebuild` удобен, когда нужно гарантированно пересобрать всё с нуля,
например после изменения флагов компилятора.

---

### Справка прямо в терминале

```bash
make help
```

Выводит краткий список всех доступных целей.

---

### Запуск `error_rate` вручную для произвольных файлов

`error_rate` принимает любое количество CSV-файлов через аргументы
командной строки и выводит отдельный блок ошибок для каждого:

```bash
# Один файл
./build/error_rate build/fft_8.csv

# Несколько файлов
./build/error_rate build/fft_8.csv build/fft_30.csv build/fft_1800.csv

# Все файлы через glob
./build/error_rate build/fft_*.csv
```

---

## Использование CMake напрямую

Если Makefile недоступен или нужен более тонкий контроль, можно работать
с CMake напрямую.

### Конфигурация и сборка

```bash
# Создать директорию build/ и сконфигурировать проект
cmake -S . -B build

# Скомпилировать (параллельно на всех ядрах)
cmake --build build -j$(nproc)
```

### Debug-сборка через CMake

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j$(nproc)
```

### Запуск через кастомную цель CMake

```bash
cmake --build build --target run
```

Эквивалентно `make run` — последовательно выполняет `fft` и `error_rate`.

### Явное указание типа сборки

```bash
# Release
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release -j$(nproc)

# Debug
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build --config Debug -j$(nproc)
```


---

## Краткая справочная таблица

| Команда | Что делает |
|---|---|
| `make` | Release-сборка обоих бинарников |
| `make debug` | Debug-сборка (`-O0 -g`) |
| `make run` | Сборка + генерация CSV + анализ ошибок |
| `make run_fft` | Сборка + только генерация CSV |
| `make run_errors` | Сборка + только анализ ошибок |
| `make clean` | Удалить `build/` |
| `make rebuild` | Удалить `build/` и пересобрать заново |
| `make help` | Показать справку в терминале |
| `cmake -S . -B build` | Конфигурация через CMake напрямую |
| `cmake --build build -j$(nproc)` | Компиляция через CMake напрямую |
| `cmake --build build --target run` | Запуск fft + error_rate через CMake |
