#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_URL_LENGTH 2048
#define SHORT_CODE_LENGTH 7

// Simple hash function (not secure, just for demo)
unsigned long hash_url(const char *str) {
    unsigned long hash = 5381;
    int c;

    while ((c = *str++))
        hash = ((hash << 5) + hash) + c;

    return hash;
}

// Convert hash to base62 short code
void to_base62(unsigned long num, char *output) {
    const char charset[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    int i = 0;
    while (num > 0 && i < SHORT_CODE_LENGTH - 1) {
        output[i++] = charset[num % 62];
        num /= 62;
    }
    output[i] = '\0';
}

int main() {
    char long_url[MAX_URL_LENGTH];
    printf("Enter a long URL: ");
    fgets(long_url, MAX_URL_LENGTH, stdin);
    long_url[strcspn(long_url, "\n")] = '\0'; // Remove newline

    // Generate short code
    unsigned long hash = hash_url(long_url);
    char short_code[SHORT_CODE_LENGTH];
    to_base62(hash, short_code);

    // Save to file
    FILE *file = fopen("urls.txt", "a");
    if (file == NULL) {
        perror("Failed to open urls.txt");
        return 1;
    }
    fprintf(file, "%s %s\n", short_code, long_url);
    fclose(file);

    printf("Short URL code: %s\n", short_code);
    return 0;
}
