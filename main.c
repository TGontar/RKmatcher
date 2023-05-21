#include <locale.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>

#define MAX_PATH_LENGTH 256 // ������������ ����� ���� �����
#define d 256 // ������ �������� d
#define q 4294967295 // ������������ �������� ��� unsigned int

// �������� ������-����� ��� ������ ��������� � ������
void searchRabinKarp(char *pattern, char *text, char *currentdir, int stringnumber) {
    int m = strlen(pattern);
    int n = strlen(text);
    unsigned pattern_hash = 0;
    unsigned current_hash = 0;
    unsigned max_deg = 1;

    // ��������������� ���������� ������������ ������� ��������, ����� ����� ����������� ������� ��������
    for (int i = 0; i < m - 1; i++)
        max_deg = (max_deg * d) % q;

    // ���������� ���-�������� ��� ������� � ��� ������ ��������� ������ m � ������
    for (int i = 0; i < m; i++) {
        pattern_hash = (d * pattern_hash + pattern[i]) % q;
        current_hash = (d * current_hash + text[i]) % q;
    }

    // ����� ������� � ������
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
                printf("������ ������: � ����� %s, ������ %d, ������� %d\n", currentdir, stringnumber, i);
        }

        // ���������� ������ ���� ��� ��������� ��������
        if (i < n - m) {
            current_hash = (d * (current_hash - text[i] * max_deg) + text[i + m]) % q;
        }
    }
}

// ����������� ����� � ������
void searchSubstringInFiles(char *pattern, char *dirpath, int recursive) {
    DIR *dir;
    struct dirent *entry;

    // �������� ����������
    dir = opendir(dirpath);
    if (dir == NULL) {
        printf("�� ������� ������� ������������� ����������: %s\n", dirpath);
        return;
    }

    // ����� ��������� � ������ ����� � ����������
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        char filepath[MAX_PATH_LENGTH];
        snprintf(filepath, MAX_PATH_LENGTH, "%s/%s", dirpath, entry->d_name);

        struct stat st;
        if (stat(filepath, &st) == -1) {
            printf("������ ��� ��������� ���������� � �����: %s\n", filepath);
            continue;
        }

        if (S_ISDIR(st.st_mode) && recursive == 1) { // ���� ��� ����������, ���������� �������� ������� ��� ���
            searchSubstringInFiles(pattern, filepath, recursive);
        } else if (S_ISREG(st.st_mode)) { // ���� ��� ����, ��������� ����� ���������
            FILE *file = fopen(filepath, "r");
            if (file == NULL) {
                printf("�� ������� ������� ����: %s\n", filepath);
                continue;
            }

            // ����� ��������� � �����
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

//������� ��������� �� ������ �� �����
void displayIncorrectInput() {
    printf("�������� ���� ����������, �������������: rkmatcher [-r] \"������\" \"���� ����������\"\n");
    printf("-r: ��������� �� ����������� ����� ���� �������������, �������������� ����\n");
    printf("\"������\" - ��� ������ �� ���� ������ ����������, ������������ ��������\n");
    printf("\"���� ����������\" - ���� � �����, � ������� ���������� ���������� �����, ������������ ��������\n");
}

//���������� ������ ~ � ����� ������� ����������
char *parseHomeDir(char *path) {
    if (path[0] == '~') {
        char cwd[4096];
        if (getcwd(cwd, sizeof(cwd)) == NULL) {
            perror("������ ��� ��������� ������� ����������");
            return 0;
        }

        size_t resultLen = strlen(cwd) + strlen(path) - 1; // �������� ����� ������
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
    setlocale(LC_ALL, "Russian");

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
