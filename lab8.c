#include <ctype.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int digit_value(char ch) {
    if (ch >= '0' && ch <= '9') return ch - '0';
    if (ch >= 'A' && ch <= 'Z') return ch - 'A' + 10;
    if (ch >= 'a' && ch <= 'z') return ch - 'a' + 10;
    return -1;
}

int read_word(FILE *file, char **word) {
    size_t size = 16;
    size_t length = 0;
    int ch;
    char *text = malloc(size);

    if (text == NULL) return -1;

    do {
        ch = fgetc(file);
    } while (ch != EOF && isspace((unsigned char)ch));

    if (ch == EOF) {
        free(text);
        return ferror(file) ? -1 : 0;
    }

    while (ch != EOF && !isspace((unsigned char)ch)) {
        if (length + 1 >= size) {
            char *new_text;
            size *= 2;
            new_text = realloc(text, size);
            if (new_text == NULL) {
                free(text);
                return -1;
            }
            text = new_text;
        }
        text[length++] = (char)ch;
        ch = fgetc(file);
    }

    if (ch == EOF && ferror(file)) {
        free(text);
        return -1;
    }

    text[length] = '\0';
    *word = text;
    return 1;
}

int convert_number(char *word, int *base, unsigned long long *value) {
    size_t start = 0;
    size_t i;
    int max_digit = 0;
    unsigned long long result = 0;

    while (word[start] == '0' && word[start + 1] != '\0') start++;

    for (i = start; word[i] != '\0'; i++) {
        int digit = digit_value(word[i]);
        if (digit < 0) return 0;
        if (digit > max_digit) max_digit = digit;
    }

    *base = max_digit + 1;
    if (*base < 2) *base = 2;

    for (i = start; word[i] != '\0'; i++) {
        int digit = digit_value(word[i]);
        if (result > (ULLONG_MAX - (unsigned long long)digit) / (unsigned long long)*base) {
            return 0;
        }
        result = result * (unsigned long long)*base + (unsigned long long)digit;
        if (word[i] >= 'a' && word[i] <= 'z') word[i] -= 'a' - 'A';
    }

    {
        size_t length = 0;
        while (word[start + length] != '\0') length++;
        for (i = 0; i <= length; i++) word[i] = word[start + i];
    }
    *value = result;
    return 1;
}

int main(int argc, char *argv[]) {
    FILE *input;
    FILE *output;
    char *word;
    int status;
    int ok = 1;

    if (argc != 3) {
        fprintf(stderr, "\210\341\257\256\253\354\247\256\242\240\255\250\245: %s <\242\345\256\244\255\256\251_\344\240\251\253> <\242\353\345\256\244\255\256\251_\344\240\251\253>\n", argv[0]);
        return 1;
    }

    input = fopen(argv[1], "r");
    if (input == NULL) {
        fprintf(stderr, "\215\245 \343\244\240\253\256\341\354 \256\342\252\340\353\342\354 \242\345\256\244\255\256\251 \344\240\251\253.\n");
        return 1;
    }
    output = fopen(argv[2], "w");
    if (output == NULL) {
        fprintf(stderr, "\215\245 \343\244\240\253\256\341\354 \256\342\252\340\353\342\354 \242\353\345\256\244\255\256\251 \344\240\251\253.\n");
        fclose(input);
        return 1;
    }

    while ((status = read_word(input, &word)) == 1) {
        int base;
        unsigned long long value;

        if (convert_number(word, &base, &value)) {
            if (fprintf(output, "%s %d %llu\n", word, base, value) < 0) ok = 0;
        } else {
            if (fprintf(output, "%s \255\245\244\256\257\343\341\342\250\254\256\n", word) < 0) ok = 0;
        }
        free(word);
        if (!ok) break;
    }

    if (status < 0) ok = 0;
    if (fclose(input) != 0) ok = 0;
    if (fclose(output) != 0) ok = 0;

    if (!ok) {
        fprintf(stderr, "\216\350\250\241\252\240 \257\340\250 \256\241\340\240\241\256\342\252\245 \344\240\251\253\256\242.\n");
        return 1;
    }

    printf("\220\245\247\343\253\354\342\240\342 \341\256\345\340\240\255\361\255 \242 \344\240\251\253 %s\n", argv[2]);
    return 0;
}




