#include <limits.h>
#include <stdio.h>

static int read_number(const char *text, unsigned long long *number) {
    unsigned long long value = 0;
    int i = 0;

    if (text == NULL || number == NULL || text[0] == '\0') return 0;
    while (text[i] != '\0') {
        unsigned int digit;
        if (text[i] < '0' || text[i] > '9') return 0;
        digit = (unsigned int)(text[i] - '0');
        if (value > (ULLONG_MAX - digit) / 10) return 0;
        value = value * 10 + digit;
        i++;
    }
    *number = value;
    return 1;
}

static int is_flag(const char *text, char flag) {
    return text != NULL && (text[0] == '-' || text[0] == '/') &&
           text[1] == flag && text[2] == '\0';
}

static int is_prime(unsigned long long number) {
    unsigned long long divisor;
    if (number < 2) return 0;
    for (divisor = 2; divisor <= number / divisor; divisor++) {
        if (number % divisor == 0) return number == divisor;
    }
    return 1;
}

static int print_multiples(unsigned long long number) {
    unsigned long long multiple;
    int found = 0;
    if (number == 0) { puts("\204\253\357\040\055\150\040\247\255\240\347\245\255\250\245\040\170\040\244\256\253\246\255\256\040\241\353\342\354\040\241\256\253\354\350\245\040\255\343\253\357\056"); return 0; }
    for (multiple = number; multiple <= 100; multiple += number) {
        if (found) putchar(' ');
        printf("%llu", multiple);
        found = 1;
        if (multiple > 100 - number) break;
    }
    if (!found) puts("\215\240\342\343\340\240\253\354\255\353\345\040\347\250\341\245\253\054\040\252\340\240\342\255\353\345\040\170\040\250\040\255\245\040\257\340\245\242\353\350\240\356\351\250\345\040\061\060\060\054\040\255\245\342\056"); else putchar('\n');
    return 1;
}

static int print_hex_digits(unsigned long long number) {
    const char digits[] = "0123456789ABCDEF";
    char result[sizeof(unsigned long long) * 2];
    int length = 0;
    if (number == 0) { puts("0"); return 1; }
    while (number > 0) { result[length++] = digits[number % 16]; number /= 16; }
    while (length > 0) { length--; printf("%c", result[length]); if (length > 0) putchar(' '); }
    putchar('\n');
    return 1;
}

static int print_powers(unsigned long long exponent_limit) {
    unsigned long long base, exponent;
    if (exponent_limit == 0 || exponent_limit > 10) { puts("\204\253\357\040\055\145\040\247\255\240\347\245\255\250\245\040\170\040\244\256\253\246\255\256\040\255\240\345\256\244\250\342\354\341\357\040\242\040\250\255\342\245\340\242\240\253\245\040\061\056\056\061\060\056"); return 0; }
    for (base = 1; base <= 10; base++) {
        unsigned long long value = 1;
        printf("\256\341\255\256\242\240\255\250\245\040\045\154\154\165\072", base);
        for (exponent = 1; exponent <= exponent_limit; exponent++) {
            value *= base;
            printf(" %llu^%llu=%llu", base, exponent, value);
        }
        putchar('\n');
    }
    return 1;
}

static int print_sum(unsigned long long number) {
    unsigned long long first = number, second = number + 1;
    if (second == 0) { puts("\221\343\254\254\240\040\255\245\040\257\256\254\245\351\240\245\342\341\357\040\242\040\066\064\055\340\240\247\340\357\244\255\353\251\040\241\245\247\247\255\240\347\255\353\251\040\342\250\257\056"); return 0; }
    if (first % 2 == 0) first /= 2; else second /= 2;
    if (first > ULLONG_MAX / second) { puts("\221\343\254\254\240\040\255\245\040\257\256\254\245\351\240\245\342\341\357\040\242\040\066\064\055\340\240\247\340\357\244\255\353\251\040\241\245\247\247\255\240\347\255\353\251\040\342\250\257\056"); return 0; }
    printf("\341\343\254\254\240\050\061\056\056\045\154\154\165\051\040\075\040\045\154\154\165\134\156", number, first * second);
    return 1;
}

static int print_factorial(unsigned long long number) {
    unsigned long long result = 1, value;
    for (value = 2; value <= number; value++) {
        if (result > ULLONG_MAX / value) { puts("\224\240\252\342\256\340\250\240\253\040\255\245\040\257\256\254\245\351\240\245\342\341\357\040\242\040\066\064\055\340\240\247\340\357\244\255\353\251\040\241\245\247\247\255\240\347\255\353\251\040\342\250\257\056"); return 0; }
        result *= value;
    }
    printf("%llu! = %llu\n", number, result);
    return 1;
}

static int run_task(unsigned long long number, const char *flag) {
    if (is_flag(flag, 'h')) return print_multiples(number);
    if (is_flag(flag, 'p')) {
        if (number < 2) puts("\170\040\255\245\040\357\242\253\357\245\342\341\357\040\255\250\040\257\340\256\341\342\353\254\054\040\255\250\040\341\256\341\342\240\242\255\353\254");
        else if (is_prime(number)) puts("\170\040\055\040\257\340\256\341\342\256\245\040\347\250\341\253\256");
        else puts("\170\040\055\040\341\256\341\342\240\242\255\256\245\040\347\250\341\253\256");
        return 1;
    }
    if (is_flag(flag, 's')) return print_hex_digits(number);
    if (is_flag(flag, 'e')) return print_powers(number);
    if (is_flag(flag, 'a')) return print_sum(number);
    if (is_flag(flag, 'f')) return print_factorial(number);
    puts("\215\245\250\247\242\245\341\342\255\353\251\040\344\253\240\243\056\040\210\341\257\256\253\354\247\343\251\342\245\040\055\150\054\040\055\160\054\040\055\163\054\040\055\145\054\040\055\141\040\250\253\250\040\055\146\056");
    return 0;
}

static void print_usage(const char *program) { printf("\210\341\257\256\253\354\247\256\242\240\255\250\245\072\040\045\163\040\074\170\076\040\074\055\150\174\055\160\174\055\163\174\055\145\174\055\141\174\055\146\076\134\156", program); }

int main(int argc, char **argv) {
    unsigned long long number;
    if (argc != 3 || !read_number(argv[1], &number)) { print_usage(argv[0]); return 1; }
    return run_task(number, argv[2]) ? 0 : 1;
}
