#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int parse_flag(const char *flag, char *mode, int *has_output_name) {
    int position = 1;

    if (flag == NULL || mode == NULL || has_output_name == NULL ||
        (flag[0] != '-' && flag[0] != '/') || flag[1] == '\0') {
        return 0;
    }

    *has_output_name = 0;
    if (flag[position] == 'n') {
        *has_output_name = 1;
        ++position;
    }

    if ((flag[position] != 'd' && flag[position] != 'i' &&
         flag[position] != 's' && flag[position] != 'a') ||
        flag[position + 1] != '\0') {
        return 0;
    }

    *mode = flag[position];
    return 1;
}

static char *make_output_name(const char *input_path) {
    const char *last_slash = strrchr(input_path, '/');
    const char *last_backslash = strrchr(input_path, '\\');
    const char *separator = last_slash;
    const char *name;
    size_t directory_length;
    size_t name_length;
    char *result;

    if (last_backslash != NULL && (separator == NULL || last_backslash > separator)) {
        separator = last_backslash;
    }
    name = separator == NULL ? input_path : separator + 1;
    directory_length = separator == NULL ? 0U : (size_t)(name - input_path);
    name_length = strlen(name);
    result = (char *)malloc(directory_length + 4U + name_length + 1U);
    if (result == NULL) {
        return NULL;
    }
    memcpy(result, input_path, directory_length);
    memcpy(result + directory_length, "out_", 4U);
    memcpy(result + directory_length + 4U, name, name_length + 1U);
    return result;
}

static int is_digit(int symbol) {
    return symbol >= '0' && symbol <= '9';
}

static int is_latin_letter(int symbol) {
    return (symbol >= 'A' && symbol <= 'Z') ||
           (symbol >= 'a' && symbol <= 'z');
}

static int write_line_result(FILE *output, char mode,
                             unsigned long letters, unsigned long special) {
    if (mode == 'i') {
        return fprintf(output, "%lu\n", letters) >= 0;
    }
    if (mode == 's') {
        return fprintf(output, "%lu\n", special) >= 0;
    }
    return fputc('\n', output) != EOF;
}

static int process_file(FILE *input, FILE *output, char mode) {
    int current;
    unsigned long letters = 0UL;
    unsigned long special = 0UL;
    int line_started = 0;

    while ((current = fgetc(input)) != EOF) {
        if (current == '\n') {
            if (!write_line_result(output, mode, letters, special)) {
                return 0;
            }
            letters = 0UL;
            special = 0UL;
            line_started = 0;
            continue;
        }

        /* Treat CRLF as one newline, but keep a standalone CR as input. */
        if (current == '\r') {
            int next = fgetc(input);
            if (next == '\n') {
                current = '\n';
            } else if (next != EOF && ungetc(next, input) == EOF) {
                return 0;
            }
        }

        if (current == '\n') {
            if (!write_line_result(output, mode, letters, special)) {
                return 0;
            }
            letters = 0UL;
            special = 0UL;
            line_started = 0;
            continue;
        }

        line_started = 1;
        if (mode == 'd') {
            if (!is_digit(current) && fputc(current, output) == EOF) {
                return 0;
            }
        } else if (mode == 'i') {
            if (is_latin_letter(current)) {
                ++letters;
            }
        } else if (mode == 's') {
            if (!is_latin_letter(current) && !is_digit(current) &&
                current != ' ') {
                ++special;
            }
        } else if (mode == 'a') {
            if (is_digit(current)) {
                if (fputc(current, output) == EOF) {
                    return 0;
                }
            } else if (fprintf(output, "%02X", (unsigned int)(unsigned char)current) < 0) {
                return 0;
            }
        }
    }

    if (ferror(input)) {
        return 0;
    }
    if (line_started && !write_line_result(output, mode, letters, special)) {
        return 0;
    }
    return !ferror(output);
}

int main(int argc, char **argv) {
    char mode;
    int has_output_name;
    const char *input_path;
    const char *output_path;
    char *generated_output = NULL;
    FILE *input;
    FILE *output;
    int success;

    if (argc < 2 || !parse_flag(argv[1], &mode, &has_output_name)) {
        puts("\210\341\257\256\253\354\247\256\242\240\255\250\245: lab4 <-d|-i|-s|-a> <\242\345\256\244\255\256\251_\344\240\251\253>\n"
             "   \250\253\250: lab4 <-nd|-ni|-ns|-na> <\242\345\256\244\255\256\251_\344\240\251\253> <\242\353\345\256\244\255\256\251_\344\240\251\253>");
        return 1;
    }

    if ((has_output_name && argc != 4) ||
        (!has_output_name && argc != 3)) {
        puts("\212\256\253\250\347\245\341\342\242\256 \240\340\243\343\254\245\255\342\256\242 \252\256\254\240\255\244\255\256\251 \341\342\340\256\252\250 \255\245 \341\256\256\342\242\245\342\341\342\242\343\245\342 \344\253\240\243\343.");
        return 1;
    }

    input_path = argv[2];
    if (has_output_name) {
        output_path = argv[3];
    } else {
        generated_output = make_output_name(input_path);
        if (generated_output == NULL) {
            puts("\215\245\244\256\341\342\240\342\256\347\255\256 \257\240\254\357\342\250 \244\253\357 \250\254\245\255\250 \242\353\345\256\244\255\256\243\256 \344\240\251\253\240.");
            return 1;
        }
        output_path = generated_output;
    }

    input = fopen(input_path, "rb");
    if (input == NULL) {
        fprintf(stderr, "\215\245 \343\244\240\253\256\341\354 \256\342\252\340\353\342\354 \242\345\256\244\255\256\251 \344\240\251\253 \302\253%s\302\273: %s\n",
                input_path, strerror(errno));
        free(generated_output);
        return 1;
    }

    output = fopen(output_path, "wb");
    if (output == NULL) {
        fprintf(stderr, "\215\245 \343\244\240\253\256\341\354 \256\342\252\340\353\342\354 \242\353\345\256\244\255\256\251 \344\240\251\253 \302\253%s\302\273: %s\n",
                output_path, strerror(errno));
        fclose(input);
        free(generated_output);
        return 1;
    }

    success = process_file(input, output, mode);
    if (fclose(output) != 0) {
        success = 0;
    }
    if (fclose(input) != 0) {
        success = 0;
    }

    if (success) {
        printf("\220\245\247\343\253\354\342\240\342 \341\256\345\340\240\255\361\255 \242 \344\240\251\253 %s\n", output_path);
    } else {
        fprintf(stderr, "\215\245 \343\244\240\253\256\341\354 \256\241\340\240\241\256\342\240\342\354 \344\240\251\253: %s\n", strerror(errno));
    }

    free(generated_output);
    return success ? 0 : 1;
}
