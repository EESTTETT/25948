#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

typedef struct {
    off_t offset;
    size_t length;
} LineInfo;

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

    LineInfo *lines = NULL;
    size_t num_lines = 0;
    size_t capacity = 0;
    off_t line_start = 0;
    size_t line_length = 0;
    char ch;
    ssize_t bytes_read;

    while ((bytes_read = read(fd, &ch, 1)) > 0) {
        off_t current_pos = lseek(fd, 0L, SEEK_CUR);

        if (current_pos == -1) {
            perror("lseek");
            free(lines);
            close(fd);
            return 1;
        }

        if (ch == '\n') {
            if (num_lines == capacity) {
                size_t new_capacity = capacity == 0 ? 8 : capacity * 2;
                LineInfo *temp = realloc(lines, new_capacity * sizeof(LineInfo));

                if (temp == NULL) {
                    perror("realloc");
                    free(lines);
                    close(fd);
                    return 1;
                }

                lines = temp;
                capacity = new_capacity;
            }

            lines[num_lines].offset = line_start;
            lines[num_lines].length = line_length;
            num_lines++;

            line_start = current_pos;
            line_length = 0;
        } else {
            line_length++;
        }
    }

    if (bytes_read == -1) {
        perror("read");
        free(lines);
        close(fd);
        return 1;
    }

    if (line_length > 0) {
        if (num_lines == capacity) {
            size_t new_capacity = capacity == 0 ? 8 : capacity * 2;
            LineInfo *temp = realloc(lines, new_capacity * sizeof(LineInfo));

            if (temp == NULL) {
                perror("realloc");
                free(lines);
                close(fd);
                return 1;
            }

            lines = temp;
            capacity = new_capacity;
        }

        lines[num_lines].offset = line_start;
        lines[num_lines].length = line_length;
        num_lines++;
    }

    for (size_t i = 0; i < num_lines; i++) {
        printf("Line %zu: Offset = %lld, Length = %zu\n",
               i + 1, (long long)lines[i].offset, lines[i].length);
    }

    int line_number;

    while (1) {
        printf("Enter line number (0 to quit): ");

        if (scanf("%d", &line_number) != 1) {
            break;
        }

        if (line_number == 0) {
            break;
        }

        if (line_number < 1 || (size_t)line_number > num_lines) {
            printf("Invalid line number\n");
            continue;
        }

        LineInfo line = lines[line_number - 1];

        if (lseek(fd, line.offset, SEEK_SET) == -1) {
            perror("lseek");
            break;
        }

        char *buffer = malloc(line.length + 1);

        if (buffer == NULL) {
            perror("malloc");
            break;
        }

        size_t total_read = 0;

        while (total_read < line.length) {
            bytes_read = read(fd, buffer + total_read,
                              line.length - total_read);

            if (bytes_read <= 0) {
                break;
            }

            total_read += (size_t)bytes_read;
        }

        buffer[total_read] = '\0';
        printf("%s\n", buffer);

        free(buffer);
    }

    free(lines);
    close(fd);

    return 0;
}
