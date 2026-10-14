#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Node {
    char *data;
    struct Node *next;
} Node;

void append(Node **head, const char *str) {
    Node *new_node = malloc(sizeof(Node));

    if (new_node == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    new_node->data = malloc(strlen(str) + 1);

    if (new_node->data == NULL) {
        perror("malloc");
        free(new_node);
        exit(EXIT_FAILURE);
    }

    strcpy(new_node->data, str);
    new_node->next = NULL;

    if (*head == NULL) {
        *head = new_node;
        return;
    }

    Node *current = *head;

    while (current->next != NULL) {
        current = current->next;
    }

    current->next = new_node;
}

void print_list(Node *head) {
    Node *current = head;

    while (current != NULL) {
        printf("%s\n", current->data);
        current = current->next;
    }
}

void free_list(Node *head) {
    while (head != NULL) {
        Node *temp = head;
        head = head->next;
        free(temp->data);
        free(temp);
    }
}

int main(void) {
    Node *head = NULL;
    char buffer[1024];

    while (fgets(buffer, sizeof(buffer), stdin) != NULL) {
        size_t length = strlen(buffer);

        if (length > 0 && buffer[length - 1] == '\n') {
            buffer[length - 1] = '\0';
        }

        if (buffer[0] == '.') {
            break;
        }

        append(&head, buffer);
    }

    print_list(head);
    free_list(head);

    return 0;
}
