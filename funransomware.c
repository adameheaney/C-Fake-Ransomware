#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <signal.h>

#define MAX_LINE 1024

// Global variables for use in signal handler
char *encrypted_global = NULL;
int shift_global = 0;
const char *filename_global = "HOLD_ME_FOR_RANSOM.txt";

// Caesar cipher function
void caesar_cipher(char *message, int shift, char *output) {
    for (int i = 0; message[i] != '\0'; i++) {
        char c = message[i];
        if (isalpha(c)) {
            char base = islower(c) ? 'a' : 'A';
            output[i] = ((c - base + shift) % 26) + base;
        } else {
            output[i] = c;
        }
    }
    output[strlen(message)] = '\0';
}

// Signal handler for graceful interruption
void handle_exit(int sig) {
    if (encrypted_global && filename_global) {
        char decrypted[MAX_LINE];
        caesar_cipher(encrypted_global, 26 - shift_global, decrypted);

        FILE *file = fopen(filename_global, "w");
        if (file) {
            fprintf(file, "%s", decrypted);
            fclose(file);
            printf("\n\n[!] Program interrupted. Original message restored to '%s'.\n", filename_global);
        } else {
            printf("\n\n[!] Failed to restore message to file.\n");
        }
    }
    exit(1);
}

// Reads entire file into a string
char* read_file_to_string(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        printf("Failed to open file: %s\n", filename);
        return NULL;
    }

    char buffer[MAX_LINE];
    size_t total_size = 1;
    char *content = malloc(total_size);
    content[0] = '\0';

    while (fgets(buffer, sizeof(buffer), file)) {
        size_t len = strlen(buffer);
        total_size += len;
        content = realloc(content, total_size);
        strcat(content, buffer);
    }

    fclose(file);
    return content;
}

int main() {
    signal(SIGINT, handle_exit); // catch Ctrl+C

    srand(time(NULL)); // seed random

    char *original = read_file_to_string(filename_global);
    if (!original) return 1;

    // Generate random Caesar shift
    char secret_letter = 'a' + (rand() % 26);
    int shift = secret_letter - 'a';
    shift_global = shift;

    // Encrypt message
    size_t len = strlen(original);
    char *encrypted = malloc(len + 1);
    char *decrypted = malloc(len + 1);
    caesar_cipher(original, shift, encrypted);
    encrypted_global = encrypted;

    printf("Encrypted message:\n%s\n", encrypted);

    // Overwrite file with encrypted message
    FILE *fout = fopen(filename_global, "w");
    if (fout) {
        fprintf(fout, "%s", encrypted);
        fclose(fout);
    } else {
        printf("Failed to write encrypted message to file.\n");
        return 1;
    }

    // Guessing loop
    char guess;
    while (1) {
        printf("Guess the lowercase letter used to encrypt it: ");
        scanf(" %c", &guess);

        if (!islower(guess)) {
            printf("Please enter a lowercase letter.\n");
            continue;
        }

        if (guess == secret_letter) {
            caesar_cipher(encrypted, 26 - shift, decrypted);
            printf("Correct! Decoded message:\n%s\n", decrypted);

            // Restore original message to file
            FILE *fout2 = fopen(filename_global, "w");
            if (fout2) {
                fprintf(fout2, "%s", decrypted);
                fclose(fout2);
                printf("Original message restored to '%s'.\n", filename_global);
            } else {
                printf("Failed to write decrypted message to file.\n");
            }

            break;
        } else {
            printf("Incorrect. Try again.\n");
        }
    }

    free(original);
    free(encrypted);
    free(decrypted);
    return 0;
}

