#include <errno.h>
#include <float.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

static int parse_double(const char *text, double *value) {
    char *end;
    double number;

    if (text == NULL || text[0] == '\0') {
        return 0;
    }

    errno = 0;
    number = strtod(text, &end);
    if (errno == ERANGE || end == text || *end != '\0' || !isfinite(number)) {
        return 0;
    }

    *value = number;
    return 1;
}

static int parse_integer(const char *text, long long *value) {
    char *end;
    long long number;

    if (text == NULL || text[0] == '\0') {
        return 0;
    }

    errno = 0;
    number = strtoll(text, &end, 10);
    if (errno == ERANGE || end == text || *end != '\0' || number == 0) {
        return 0;
    }

    *value = number;
    return 1;
}

static int is_flag(const char *text, char flag) {
    return text != NULL &&
           (text[0] == '-' || text[0] == '/') &&
           text[1] == flag && text[2] == '\0';
}

static int equal_double(double first, double second, double epsilon) {
    return fabs(first - second) <= epsilon;
}

static void print_root(double root, double epsilon) {
    if (fabs(root) <= epsilon) {
        root = 0.0;
    }
    printf("%.10g\n", root);
}

static int solve_quadratic(double a, double b, double c, double epsilon) {
    double discriminant;

    if (fabs(a) <= epsilon) {
        if (fabs(b) <= epsilon) {
            if (fabs(c) <= epsilon) {
                puts("\241\245\341\252\256\255\245\347\255\256 \254\255\256\243\256 \340\245\350\245\255\250\251");
            } else {
                puts("\340\245\350\245\255\250\251 \255\245\342");
            }
        } else {
            puts("\253\250\255\245\251\255\256\245 \343\340\240\242\255\245\255\250\245");
            print_root(-c / b, epsilon);
        }
        return 1;
    }

    discriminant = b * b - 4.0 * a * c;
    if (!isfinite(discriminant)) {
        puts("\256\350\250\241\252\240 \242\353\347\250\341\253\245\255\250\357");
        return 0;
    }

    if (discriminant > epsilon) {
        double root = sqrt(discriminant);
        print_root((-b - root) / (2.0 * a), epsilon);
        print_root((-b + root) / (2.0 * a), epsilon);
    } else if (fabs(discriminant) <= epsilon) {
        print_root(-b / (2.0 * a), epsilon);
    } else {
        puts("\244\245\251\341\342\242\250\342\245\253\354\255\353\345 \252\256\340\255\245\251 \255\245\342");
    }

    return 1;
}

static int already_printed(double printed[][3], int count,
                           const double current[3], double epsilon) {
    int row;
    int column;

    for (row = 0; row < count; ++row) {
        for (column = 0; column < 3; ++column) {
            if (!equal_double(printed[row][column], current[column], epsilon)) {
                break;
            }
        }
        if (column == 3) {
            return 1;
        }
    }

    return 0;
}

static int task_q(int argc, char **argv) {
    const int permutations[6][3] = {
        {0, 1, 2}, {0, 2, 1}, {1, 0, 2},
        {1, 2, 0}, {2, 0, 1}, {2, 1, 0}
    };
    double epsilon;
    double coefficient[3];
    double printed[6][3];
    int printed_count = 0;
    int i;

    if (argc != 6 || !parse_double(argv[2], &epsilon) || epsilon <= 0.0) {
        puts("\210\341\257\256\253\354\247\256\242\240\255\250\245: lab3 -q <\355\257\341\250\253\256\255> <a> <b> <c>");
        return 0;
    }

    for (i = 0; i < 3; ++i) {
        if (!parse_double(argv[i + 3], &coefficient[i])) {
            puts("\212\256\355\344\344\250\346\250\245\255\342\353 \244\256\253\246\255\353 \241\353\342\354 \252\256\255\245\347\255\353\254\250 \244\245\251\341\342\242\250\342\245\253\354\255\353\254\250 \347\250\341\253\240\254\250.");
            return 0;
        }
    }

    for (i = 0; i < 6; ++i) {
        double current[3];
        int j;

        for (j = 0; j < 3; ++j) {
            current[j] = coefficient[permutations[i][j]];
        }

        if (!already_printed(printed, printed_count, current, epsilon)) {
            printed[printed_count][0] = current[0];
            printed[printed_count][1] = current[1];
            printed[printed_count][2] = current[2];
            ++printed_count;

            printf("\223\340\240\242\255\245\255\250\245 %.10g*x^2%+.10g*x%+.10g = 0:\n",
                   current[0], current[1], current[2]);
            if (!solve_quadratic(current[0], current[1], current[2], epsilon)) {
                return 0;
            }
        }
    }

    return 1;
}

static int task_m(int argc, char **argv) {
    long long first;
    long long second;

    if (argc != 4 || !parse_integer(argv[2], &first) ||
        !parse_integer(argv[3], &second)) {
        puts("\210\341\257\256\253\354\247\256\242\240\255\250\245: lab3 -m <\255\245\255\343\253\245\242\256\245 \346\245\253\256\245> <\255\245\255\343\253\245\242\256\245 \346\245\253\256\245>");
        return 0;
    }

    if ((first == LLONG_MIN && second == -1) || first % second == 0) {
        puts("\217\245\340\242\256\245 \347\250\341\253\256 \244\245\253\250\342\341\357 \255\240 \242\342\256\340\256\245.");
    } else {
        puts("\217\245\340\242\256\245 \347\250\341\253\256 \255\245 \244\245\253\250\342\341\357 \255\240 \242\342\256\340\256\245.");
    }

    return 1;
}

static int task_t(int argc, char **argv) {
    double epsilon;
    double side[3];
    double sum;
    double longest_square;
    int longest = 0;
    int i;

    if (argc != 6 || !parse_double(argv[2], &epsilon) || epsilon <= 0.0) {
        puts("\210\341\257\256\253\354\247\256\242\240\255\250\245: lab3 -t <\355\257\341\250\253\256\255> <\341\342\256\340\256\255\2401> <\341\342\256\340\256\255\2402> <\341\342\256\340\256\255\2403>");
        return 0;
    }

    for (i = 0; i < 3; ++i) {
        if (!parse_double(argv[i + 3], &side[i]) || side[i] <= 0.0) {
            puts("\221\342\256\340\256\255\353 \342\340\245\343\243\256\253\354\255\250\252\240 \244\256\253\246\255\353 \241\353\342\354 \257\256\253\256\246\250\342\245\253\354\255\353\254\250 \252\256\255\245\347\255\353\254\250 \244\245\251\341\342\242\250\342\245\253\354\255\353\254\250 \347\250\341\253\240\254\250.");
            return 0;
        }
        if (side[i] > sqrt(DBL_MAX)) {
            puts("\221\342\256\340\256\255\353 \342\340\245\343\243\256\253\354\255\250\252\240 \341\253\250\350\252\256\254 \242\245\253\250\252\250.");
            return 0;
        }
        if (side[i] > side[longest]) {
            longest = i;
        }
    }

    sum = 0.0;
    for (i = 0; i < 3; ++i) {
        if (i != longest) {
            sum += side[i] * side[i];
        }
    }
    longest_square = side[longest] * side[longest];

    if (equal_double(sum, longest_square, epsilon)) {
        puts("\210\247 \355\342\250\345 \341\342\256\340\256\255 \254\256\246\255\256 \341\256\341\342\240\242\250\342\354 \257\340\357\254\256\343\243\256\253\354\255\353\251 \342\340\245\343\243\256\253\354\255\250\252.");
    } else {
        puts("\210\247 \355\342\250\345 \341\342\256\340\256\255 \255\245\253\354\247\357 \341\256\341\342\240\242\250\342\354 \257\340\357\254\256\343\243\256\253\354\255\353\251 \342\340\245\343\243\256\253\354\255\250\252.");
    }

    return 1;
}

int main(int argc, char **argv) {
    if (argc < 2) {
        puts("\210\341\257\256\253\354\247\256\242\240\255\250\245: lab3 -q|-m|-t ...");
        return 1;
    }

    if (is_flag(argv[1], 'q')) {
        return task_q(argc, argv) ? 0 : 1;
    }
    if (is_flag(argv[1], 'm')) {
        return task_m(argc, argv) ? 0 : 1;
    }
    if (is_flag(argv[1], 't')) {
        return task_t(argc, argv) ? 0 : 1;
    }

    puts("\215\245\250\247\242\245\341\342\255\353\251 \344\253\240\243. \210\341\257\256\253\354\247\343\251\342\245 -q, -m \250\253\250 -t.");
    return 1;
}
