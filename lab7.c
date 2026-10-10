#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int is_flag(const char *text, char letter)
{
    return text != NULL && (text[0] == '-' || text[0] == '/') &&
           text[1] == letter && text[2] == '\0';
}

typedef enum {
    TOKEN_ERROR = -1,
    TOKEN_EOF = 0,
    TOKEN_OK = 1
} TokenStatus;

static TokenStatus read_token(FILE *file, char **result)
{
    size_t capacity = 32U;
    size_t length = 0U;
    int current;
    char *token;

    if (file == NULL || result == NULL) {
        return TOKEN_ERROR;
    }
    *result = NULL;
    token = (char *)malloc(capacity);
    if (token == NULL) {
        return TOKEN_ERROR;
    }
    do {
        current = fgetc(file);
        if (current == EOF) {
            free(token);
            return ferror(file) ? TOKEN_ERROR : TOKEN_EOF;
        }
    } while (isspace((unsigned char)current));

    while (current != EOF && !isspace((unsigned char)current)) {
        if (length + 1U >= capacity) {
            char *resized;
            if (capacity > (size_t)-1 / 2U) {
                free(token);
                return TOKEN_ERROR;
            }
            capacity *= 2U;
            resized = (char *)realloc(token, capacity);
            if (resized == NULL) {
                free(token);
                return TOKEN_ERROR;
            }
            token = resized;
        }
        token[length++] = (char)current;
        current = fgetc(file);
    }
    if (current == EOF && ferror(file)) {
        free(token);
        return TOKEN_ERROR;
    }
    token[length] = '\0';
    *result = token;
    return TOKEN_OK;
}

static int write_base_number(FILE *output, unsigned int value, unsigned int base)
{
    const char digits[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    char buffer[32];
    size_t length = 0U;

    if (base < 2U || base > 36U) {
        return 0;
    }
    if (value == 0U) {
        return fputc('0', output) != EOF;
    }
    while (value > 0U) {
        buffer[length++] = digits[value % base];
        value /= base;
    }
    while (length > 0U) {
        if (fputc((unsigned char)buffer[--length], output) == EOF) {
            return 0;
        }
    }
    return 1;
}

static int write_ascii_base(FILE *output, unsigned char symbol, unsigned int base)
{
    return write_base_number(output, (unsigned int)symbol, base);
}

static int write_plain_token(FILE *output, const char *token)
{
    return fputs(token, output) >= 0;
}

static int write_transformed_token(FILE *output, const char *token, unsigned long position)
{
    size_t i;

    if (position % 10UL == 0UL) {
        for (i = 0U; token[i] != '\0'; ++i) {
            unsigned char symbol = (unsigned char)tolower((unsigned char)token[i]);
            if (!write_ascii_base(output, symbol, 4U)) {
                return 0;
            }
        }
        return 1;
    }
    if (position % 5UL == 0UL) {
        for (i = 0U; token[i] != '\0'; ++i) {
            if (!write_ascii_base(output, (unsigned char)token[i], 8U)) {
                return 0;
            }
        }
        return 1;
    }
    if (position % 2UL == 0UL) {
        for (i = 0U; token[i] != '\0'; ++i) {
            if (fputc((unsigned char)tolower((unsigned char)token[i]), output) == EOF) {
                return 0;
            }
        }
        return 1;
    }
    return fputs(token, output) >= 0;
}

static int run_alternating(FILE *file1, FILE *file2, FILE *output)
{
    int first_output = 1;
    int first_finished = 0;
    int second_finished = 0;
    unsigned long position = 0UL;

    while (!first_finished || !second_finished) {
        FILE *selected;
        char *token;
        TokenStatus status;

        if (position % 2UL == 0UL) {
            selected = file1;
            if (first_finished && !second_finished) {
                selected = file2;
            }
        } else {
            selected = file2;
            if (second_finished && !first_finished) {
                selected = file1;
            }
        }

        status = read_token(selected, &token);
        if (status == TOKEN_ERROR) {
            return 0;
        }
        if (status == TOKEN_EOF) {
            if (selected == file1) {
                first_finished = 1;
            } else {
                second_finished = 1;
            }
            continue;
        }
        if (!first_output && fputc(' ', output) == EOF) {
            free(token);
            return 0;
        }
        ++position;
        if (!write_plain_token(output, token)) {
            free(token);
            return 0;
        }
        first_output = 0;
        free(token);
    }
    return fputc('\n', output) != EOF;
}

static int run_single(FILE *input, FILE *output)
{
    char *token = NULL;
    unsigned long position = 0UL;
    int first_output = 1;

    for (;;) {
        TokenStatus status = read_token(input, &token);
        if (status == TOKEN_ERROR) {
            return 0;
        }
        if (status == TOKEN_EOF) {
            break;
        }
        ++position;
        if (!first_output && fputc(' ', output) == EOF) {
            free(token);
            return 0;
        }
        if (!write_transformed_token(output, token, position)) {
            free(token);
            return 0;
        }
        first_output = 0;
        free(token);
        token = NULL;
    }
    return fputc('\n', output) != EOF;
}

int main(int argc, char **argv)
{
    const char *output_path;
    FILE *file1 = NULL;
    FILE *file2 = NULL;
    FILE *output = NULL;
    int success;

    if (argc == 5 && is_flag(argv[1], 'r')) {
        file1 = fopen(argv[2], "rb");
        file2 = fopen(argv[3], "rb");
        output_path = argv[4];
    } else if (argc == 4 && is_flag(argv[1], 'a')) {
        file1 = fopen(argv[2], "rb");
        output_path = argv[3];
    } else {
        puts("\210\341\257\256\253\354\247\256\242\240\255\250\245: lab7 -r <\344\240\251\2531> <\344\240\251\2532> <\242\353\345\256\244\255\256\251_\344\240\251\253>\n   \250\253\250: lab7 -a <\344\240\251\253> <\242\353\345\256\244\255\256\251_\344\240\251\253>");
        return 1;
    }

    if (file1 == NULL || (argc == 5 && file2 == NULL)) {
        fprintf(stderr, "\215\245 \343\244\240\253\256\341\354 \256\342\252\340\353\342\354 \242\345\256\244\255\256\251 \344\240\251\253: %s\n", strerror(errno));
        if (file1 != NULL) {
            fclose(file1);
        }
        if (file2 != NULL) {
            fclose(file2);
        }
        return 1;
    }
    output = fopen(output_path, "wb");
    if (output == NULL) {
        fprintf(stderr, "\215\245 \343\244\240\253\256\341\354 \256\342\252\340\353\342\354 \242\353\345\256\244\255\256\251 \344\240\251\253 \302\253%s\302\273: %s\n", output_path, strerror(errno));
        fclose(file1);
        if (file2 != NULL) {
            fclose(file2);
        }
        return 1;
    }

    success = argc == 5 ? run_alternating(file1, file2, output) : run_single(file1, output);
    if (fclose(output) != 0) {
        success = 0;
    }
    if (fclose(file1) != 0) {
        success = 0;
    }
    if (file2 != NULL && fclose(file2) != 0) {
        success = 0;
    }

    if (!success) {
        fprintf(stderr, "\215\245 \343\244\240\253\256\341\354 \256\241\340\240\241\256\342\240\342\354 \344\240\251\253\353.\n");
        return 1;
    }
    printf("\220\245\247\343\253\354\342\240\342 \341\256\345\340\240\255\361\255 \242 \344\240\251\253 %s\n", output_path);
    return 0;
}
