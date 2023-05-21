#include <locale.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <windows.h>

#define MAX_PATH_LENGTH 256 // Максимальная длина пути файла
#define d 256 // Размер алфавита d
#define q 4294967295 // Максимальное значение для unsigned int

// Алгоритм Рабина-Карпа для поиска подстроки в строке
void searchRabinKarp(char *pattern, char *text, char *currentdir, int stringnumber) {
    int m = strlen(pattern);
    int n = strlen(text);
    unsigned pattern_hash = 0;
    unsigned current_hash = 0;
    unsigned max_deg = 1;

    // Предварительное вычисление максимальной степени полинома, чтобы потом эффективнее работал алгоритм
    for (int i = 0; i < m - 1; i++)
        max_deg = (max_deg * d) % q;

    // Вычисление хеш-значения для шаблона и для первой подстроки длиной m в тексте
    for (int i = 0; i < m; i++) {
        pattern_hash = (d * pattern_hash + pattern[i]) % q;
        current_hash = (d * current_hash + text[i]) % q;
    }

    // Поиск шаблона в тексте
    for (int i = 0; i <= (n - m); i++) {
        if (pattern_hash == current_hash) {
            int flag = 0;

            for (int j = 0; j < m; j++) {
                if (text[i + j] != pattern[j]) {
                    flag = 1;
                    break;
                }
            }

            if (flag == 0)
                printf("Шаблон найден: в файле %s, строка %d, позиция %d\n", currentdir, stringnumber, i);
        }

        // Вычисление нового хэша для следующей итерации
        if (i < n - m) {
            current_hash = (d * (current_hash - text[i] * max_deg) + text[i + m]) % q;
        }
    }
}

// Рекурсивный поиск в папках
void searchSubstringInFiles(char *pattern, char *dirpath, int recursive) {
    DIR *dir;
    struct dirent *entry;

    // Открытие директории
    dir = opendir(dirpath);
    if (dir == NULL) {
        printf("Не удалось открыть запрашиваемую директорию: %s\n", dirpath);
        return;
    }

    // Поиск подстроки в каждом файле и директории
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        char filepath[MAX_PATH_LENGTH];
        snprintf(filepath, MAX_PATH_LENGTH, "%s/%s", dirpath, entry->d_name);

        struct stat st;
        if (stat(filepath, &st) == -1) {
            printf("Ошибка при получении информации о файле: %s\n", filepath);
            continue;
        }

        if (S_ISDIR(st.st_mode) && recursive == 1) { // Если это директория, рекурсивно вызываем функцию для нее
            searchSubstringInFiles(pattern, filepath, recursive);
        } else if (S_ISREG(st.st_mode)) { // Если это файл, выполняем поиск подстроки
            FILE *file = fopen(filepath, "r");
            if (file == NULL) {
                printf("Не удалось открыть файл: %s\n", filepath);
                continue;
            }

            // Поиск подстроки в файле
            char text[1024];
            int currentstring = 1;
            while (fgets(text, sizeof(text), file) != NULL) {
                searchRabinKarp(pattern, text, filepath, currentstring);
                currentstring += 1;
            }

            fclose(file);
        }
    }

    closedir(dir);
}

//вывести сообщение об ошибке на экран
void displayIncorrectInput() {
    printf("Неверный ввод аргументов, использование: rkmatcher [-r] \"шаблон\" \"путь директории\"\n");
    printf("-r: выполнять ли рекурсивный поиск всех поддиректорий, необязательный флаг\n");
    printf("\"шаблон\" - для поиска во всех файлах директории, обязательный параметр\n");
    printf("\"путь директории\" - путь к папке, в которой необходимо произвести поиск, обязательный параметр\n");
}

//превратить символ ~ в адрес текущей директории
char *parseHomeDir(char *path) {
    if (path[0] == '~') {
        char cwd[4096];
        if (getcwd(cwd, sizeof(cwd)) == NULL) {
            perror("Ошибка при получении текущей директории");
            return 0;
        }

        size_t resultLen = strlen(cwd) + strlen(path) - 1; // Итоговая длина строки
        char *result = malloc(resultLen + 1);
        strncpy(result, cwd, strlen(cwd));
        strncpy(result + strlen(cwd), path + 1, strlen(path) - 1);
        result[resultLen] = '\0';

        return result;
    } else {
        return path;
    }
}

int main(int count, char *argv[]) {
    system("chcp 65001");

    if (count < 3 || count > 4) {
        displayIncorrectInput();

        return 0;
    } else if (count == 3) {
        char *res = parseHomeDir(argv[2]);
        searchSubstringInFiles(argv[1], res, 0);

        return 0;
    } else if (count == 4) {
        if (strlen(argv[1]) != 2 || argv[1][0] != '-' || argv[1][1] != 'r') {
            displayIncorrectInput();
            return 0;
        }

        char *res = parseHomeDir(argv[3]);
        searchSubstringInFiles(argv[2], res, 1);

        return 0;
    }
}
