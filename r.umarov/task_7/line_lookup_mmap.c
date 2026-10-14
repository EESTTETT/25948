#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/mman.h>
#include <sys/stat.h>

typedef struct {
    size_t offset;
    size_t length;
} LineInfo;

static char *global_map;
static size_t global_size;

void alarm_handler(int sig) {
    (void)sig;

    const char message[] =
        "\n[TIMEOUT] Time is up! Printing full file content...\n";

    write(STDOUT_FILENO, message, sizeof(message) - 1);

    size_t written = 0;

    while (written < global_size) {
        ssize_t n = write(STDOUT_FILENO, global_map + written,
                          global_size - written);
        if (n <= 0) {
            _exit(EXIT_FAILURE);
        }
        written += (size_t)n;
    }

    _exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s filename\n", argv[0]);
        return 1;
    }

    int fd = open(argv[1], O_RDONLY);
    if (fd == -1) {
        perror("open");
        return 1;
    }

    struct stat st;
    if (fstat(fd, &st) == -1) {
        perror("fstat");
        close(fd);
        return 1;
    }

    if (st.st_size == 0) {
        printf("File is empty.\n");
        close(fd);
        return 0;
    }

    global_size = (size_t)st.st_size;

    global_map = mmap(NULL, global_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (global_map == MAP_FAILED) {
        perror("mmap");
        close(fd);
        return 1;
    }

    close(fd);

    LineInfo *lines = NULL;
    size_t num_lines = 0;
    size_t capacity = 0;
    size_t line_start = 0;
    size_t line_length = 0;

    for (size_t i = 0; i < global_size; i++) {
        if (global_map[i] == '\n') {
            if (num_lines == capacity) {
                size_t new_capacity = capacity == 0 ? 8 : capacity * 2;
                LineInfo *temp = realloc(
                    lines, new_capacity * sizeof(LineInfo));

                if (temp == NULL) {
                    perror("realloc");
                    free(lines);
                    munmap(global_map, global_size);
                    return 1;
                }

                lines = temp;
                capacity = new_capacity;
            }

            lines[num_lines].offset = line_start;
            lines[num_lines].length = line_length;
            num_lines++;

            line_start = i + 1;
            line_length = 0;
        } else {
            line_length++;
        }
    }

    if (line_length > 0) {
        if (num_lines == capacity) {
            size_t new_capacity = capacity == 0 ? 8 : capacity * 2;
            LineInfo *temp = realloc(
                lines, new_capacity * sizeof(LineInfo));

            if (temp == NULL) {
                perror("realloc");
                free(lines);
                munmap(global_map, global_size);
                return 1;
            }

            lines = temp;
            capacity = new_capacity;
        }

        lines[num_lines].offset = line_start;
        lines[num_lines].length = line_length;
        num_lines++;
    }

    printf("--- Debug: Line Table ---\n");

    for (size_t i = 0; i < num_lines; i++) {
        printf("Line %zu: Offset = %zu, Length = %zu\n",
               i + 1, lines[i].offset, lines[i].length);
    }

    printf("-------------------------\n");

    if (signal(SIGALRM, alarm_handler) == SIG_ERR) {
        perror("signal");
        free(lines);
        munmap(global_map, global_size);
        return 1;
    }

    int line_number;

    while (1) {
        printf("Enter line number (0 to quit, 5 sec timeout): ");
        fflush(stdout);

        alarm(5);

        if (scanf("%d", &line_number) != 1) {
            alarm(0);

            int c;
            while ((c = getchar()) != '\n' && c != EOF) {
            }

            if (feof(stdin)) {
                break;
            }

            continue;
        }

        alarm(0);

        if (line_number == 0) {
            break;
        }

        if (line_number < 1 || (size_t)line_number > num_lines) {
            printf("Invalid line number\n");
            continue;
        }

        LineInfo line = lines[line_number - 1];

        fwrite(global_map + line.offset, 1, line.length, stdout);
        printf("\n");
    }

    free(lines);
    munmap(global_map, global_size);

    return 0;
}
