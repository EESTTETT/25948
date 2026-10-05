#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

void print_uids() {
    printf("Real UID: %d\n", getuid());
    printf("Effective: %d\n", geteuid());
}

void open_file(const char *filename) {
    FILE *f = fopen(filename, "r");
    if (f == NULL) {
        perror("Результат fopen");
    } else {
        printf("Файл открыт успешно\n");
        fclose(f);
    }
}

int main() {
    const char *filename = "data.txt";

    printf("------------- ДО сброса привилегий -------------\n");
    print_uids();
    open_file(filename);

    printf("\nВызов setuid...\n");
    if (setuid(getuid()) == -1) {
        perror("Ошибка setuid");
        exit(EXIT_FAILURE);
    }

    printf("\n------------- ПОСЛЕ сброса привилегий -------------\n");
    print_uids();
    open_file(filename);

    return EXIT_SUCCESS;
}
