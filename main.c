#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_URL_LENGTH 2048
#define SHORT_CODE_LENGTH 7
#define LINE_BUFFER_SIZE 2100

unsigned long hash_url(const char *str) {
    unsigned long hash = 5381;
    int c;
    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c;
    }
    return hash;
}

void to_base62(unsigned long num, char *output) {
    const char charset[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    int i = 0;
    while (num > 0 && i < SHORT_CODE_LENGTH - 1) {
        output[i++] = charset[num % 62];
        num /= 62;
    }
    output[i] = '\0';
}

void shorten_url() {
    char long_url[MAX_URL_LENGTH];
    printf("Enter a long URL: ");
    fgets(long_url, MAX_URL_LENGTH, stdin);
    long_url[strcspn(long_url, "\n")] = '\0';

    unsigned long hash = hash_url(long_url);
    char short_code[SHORT_CODE_LENGTH];
    to_base62(hash, short_code);

    FILE *file = fopen("urls.txt", "a");
    if (!file) {
        perror("Failed to open urls.txt");
        return;
    }
    fprintf(file, "%s %s\n", short_code, long_url);
    fclose(file);

    printf("Short URL code: %s\n", short_code);
}

void retrieve_url() {
    char input_code[SHORT_CODE_LENGTH];
    printf("Enter short code: ");
    scanf("%s", input_code);

    FILE *file = fopen("urls.txt", "r");
    if (!file) {
        perror("Failed to open urls.txt");
        return;
    }

    char code[SHORT_CODE_LENGTH], url[MAX_URL_LENGTH];
    int found = 0;

    while (fscanf(file, "%s %s", code, url) != EOF) {
        if (strcmp(code, input_code) == 0) {
            printf("Original URL: %s\n", url);
            found = 1;
            break;
        }
    }

    if (!found) {
        printf("Short code not found.\n");
    }

    fclose(file);
}

int main() {
    int choice;
    printf("1. Shorten URL\n2. Retrieve URL\nChoose option: ");
    scanf("%d", &choice);
    getchar(); // Consume newline

    if (choice == 1) {
        shorten_url();
    } else if (choice == 2) {
        retrieve_url();
    } else {
        printf("Invalid option.\n");
    }

    return 0;
}
