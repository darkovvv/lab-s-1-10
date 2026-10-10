#include <ctype.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int digit_value(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'A' && c <= 'Z') return c - 'A' + 10;
    return -1;
}

int read_number(const char *text, int base, long long *number) {
    int negative = 0;
    unsigned long long value = 0;
    unsigned long long limit;
    int digit;

    if (*text == '+' || *text == '-') {
        negative = (*text == '-');
        text++;
    }
    if (*text == '\0') return 0;

    limit = negative ? (unsigned long long)LLONG_MAX + 1ULL : (unsigned long long)LLONG_MAX;
    while (*text != '\0') {
        digit = digit_value(*text);
        if (digit < 0 || digit >= base || value > (limit - (unsigned long long)digit) / (unsigned long long)base) return 0;
        value = value * (unsigned long long)base + (unsigned long long)digit;
        text++;
    }

    if (negative) {
        if (value == (unsigned long long)LLONG_MAX + 1ULL) *number = LLONG_MIN;
        else *number = -(long long)value;
    } else {
        *number = (long long)value;
    }
    return 1;
}

unsigned long long absolute_value(long long number) {
    if (number < 0) return (unsigned long long)(-(number + 1)) + 1ULL;
    return (unsigned long long)number;
}

void print_in_base(long long number, int base) {
    char digits[70];
    int length = 0;
    unsigned long long value = absolute_value(number);
    const char symbols[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";

    do {
        digits[length] = symbols[value % (unsigned long long)base];
        length++;
        value /= (unsigned long long)base;
    } while (value > 0);

    if (number < 0) putchar('-');
    while (length > 0) {
        length--;
        putchar(digits[length]);
    }
    putchar('\n');
}

int main(void) {
    char line[512];
    char *end;
    long base_value;
    int base;
    int has_number = 0;
    long long number;
    long long maximum = 0;
    long long sum = 0;
    int sum_overflowed = 0;
    unsigned long long max_absolute = 0;
    const int output_bases[] = {9, 18, 27, 36};
    int i;

    printf("\202\242\245\244\250\342\245 \256\341\255\256\242\240\255\250\245 \341\250\341\342\245\254\353 \341\347\250\341\253\245\255\250\357 (2..36): ");
    if (fgets(line, sizeof(line), stdin) == NULL) {
        puts("\216\350\250\241\252\240 \242\242\256\244\240.");
        return 1;
    }
    base_value = strtol(line, &end, 10);
    while (isspace((unsigned char)*end)) end++;
    if (end == line || (*end != '\0' && *end != '\n') || base_value < 2 || base_value > 36) {
        puts("\215\245\244\256\257\343\341\342\250\254\256\245 \256\341\255\256\242\240\255\250\245 \341\250\341\342\245\254\353 \341\347\250\341\253\245\255\250\357.");
        return 1;
    }
    base = (int)base_value;

    puts("\202\242\245\244\250\342\245 \347\250\341\253\240 \242 \355\342\256\251 \341\250\341\342\245\254\245 \341\347\250\341\253\245\255\250\357, \257\256 \256\244\255\256\254\343 \242 \341\342\340\256\252\245. \202\242\245\244\250\342\245 \302\253\320\241\321\202\320\276\320\277\302\273 \244\253\357 \247\240\242\245\340\350\245\255\250\357.");
    while (fgets(line, sizeof(line), stdin) != NULL) {
        size_t length = strlen(line);
        while (length > 0 && isspace((unsigned char)line[length - 1])) {
            line[--length] = '\0';
        }
        char *text = line;
        while (isspace((unsigned char)*text)) text++;

        if (strcmp(text, "\221\342\256\257") == 0) break;
        if (!read_number(text, base, &number)) {
            puts("\215\245\244\256\257\343\341\342\250\254\256\245 \346\245\253\256\245 \347\250\341\253\256; \257\256\257\340\256\241\343\251\342\245 \245\351\361 \340\240\247.");
            continue;
        }
        if (!sum_overflowed &&
            ((number > 0 && sum > LLONG_MAX - number) ||
             (number < 0 && sum < LLONG_MIN - number))) {
            puts("\217\245\340\245\257\256\253\255\245\255\250\245 \341\343\254\254\353: \341\343\254\254\343 \242\341\245\345 \242\242\245\244\361\255\255\353\345 \347\250\341\245\253 \255\245\242\256\247\254\256\246\255\256 \257\340\245\244\341\342\240\242\250\342\354.");
            sum_overflowed = 1;
        } else if (!sum_overflowed) {
            sum += number;
        }
        if (!has_number || absolute_value(number) > max_absolute) {
            maximum = number;
            max_absolute = absolute_value(number);
            has_number = 1;
        }
    }

    if (!has_number) {
        puts("\227\250\341\253\240 \255\245 \242\242\245\244\245\255\353.");
        return 0;
    }

    printf("\214\240\252\341\250\254\240\253\354\255\256\245 \257\256 \254\256\244\343\253\356 \347\250\341\253\256: %lld\n", maximum);
    if (sum_overflowed) {
        puts("\221\343\254\254\240 \255\245\244\256\341\342\343\257\255\240 \250\247-\247\240 \257\245\340\245\257\256\253\255\245\255\250\357.");
        return 1;
    }
    printf("\221\343\254\254\240: %lld\n", sum);
    for (i = 0; i < 4; i++) {
        printf("\214\240\252\341\250\254\240\253\354\255\256\245 \347\250\341\253\256 \242 \341\250\341\342\245\254\245 \341\347\250\341\253\245\255\250\357 \341 \256\341\255\256\242\240\255\250\245\254 %d: ", output_bases[i]);
        print_in_base(maximum, output_bases[i]);
        printf("\221\343\254\254\240 \242 \341\250\341\342\245\254\245 \341\347\250\341\253\245\255\250\357 \341 \256\341\255\256\242\240\255\250\245\254 %d: ", output_bases[i]);
        print_in_base(sum, output_bases[i]);
    }
    return 0;
}

