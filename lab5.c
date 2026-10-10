#include <math.h>
#include <stdio.h>
#include <stdlib.h>

static int read_double(const char *text, double *value) {
    char *end;
    double number;

    if (text == NULL || text[0] == '\0') {
        return 0;
    }

    number = strtod(text, &end);
    if (*end != '\0' || !isfinite(number)) {
        return 0;
    }

    *value = number;
    return 1;
}

static int sum_a(double x, double epsilon, double *result) {
    double sum = 1.0;
    double term = 1.0;
    int n;

    for (n = 1; n <= 1000000; ++n) {
        term *= x / (double)n;
        sum += term;
        if (fabs(term) <= epsilon) {
            *result = sum;
            return 1;
        }
    }
    return 0;
}

static int sum_b(double x, double epsilon, double *result) {
    double sum = 1.0;
    double term = 1.0;
    int n;

    for (n = 0; n < 1000000; ++n) {
        double first = 2.0 * (double)n + 1.0;
        double second = 2.0 * (double)n + 2.0;
        term *= -x * x / (first * second);
        sum += term;
        if (fabs(term) <= epsilon) {
            *result = sum;
            return 1;
        }
    }
    return 0;
}

static int sum_c(double x, double epsilon, double *result) {
    double sum = 1.0;
    double term = 1.0;
    int n;

    if (fabs(x) >= 1.0) {
        return 0;
    }
    for (n = 0; n < 1000000; ++n) {
        double number = (double)n;
        double numerator = 27.0 * (number + 1.0) * (number + 1.0) *
                           (number + 1.0) * x * x;
        double denominator = (3.0 * number + 1.0) * (3.0 * number + 2.0) * (3.0 * number + 3.0);
        term *= numerator / denominator;
        sum += term;
        if (fabs(term) <= epsilon) {
            *result = sum;
            return 1;
        }
    }
    return 0;
}

static int sum_d(double x, double epsilon, double *result) {
    double sum;
    double term;
    int n;

    if (fabs(x) >= 1.0) {
        return 0;
    }
    term = -x * x / 2.0;
    sum = 1.0 + term;
    if (fabs(term) <= epsilon) {
        *result = sum;
        return 1;
    }
    for (n = 1; n < 1000000; ++n) {
        term *= -x * x * (2.0 * (double)n + 1.0) / (2.0 * (double)n + 2.0);
        sum += term;
        if (fabs(term) <= epsilon) {
            *result = sum;
            return 1;
        }
    }
    return 0;
}

static double integral_function(int number, double x) {
    if (number == 1) return x == 0.0 ? 1.0 : log1p(x) / x;
    if (number == 2) return exp(-x * x / 2.0);
    return pow(x, x);
}

static double simpson_value(double a, double b,
                            double fa, double fm, double fb) {
    return (b - a) * (fa + 4.0 * fm + fb) / 6.0;
}

static int adaptive_simpson(int number, double a, double b,
                            double fa, double fm, double fb,
                            double whole, double tolerance, int depth,
                            double *result) {
    double middle = a + (b - a) / 2.0;
    double left_middle = a + (middle - a) / 2.0;
    double right_middle = middle + (b - middle) / 2.0;
    double flm = integral_function(number, left_middle);
    double frm = integral_function(number, right_middle);
    double left_value = simpson_value(a, middle, fa, flm, fm);
    double right_value = simpson_value(middle, b, fm, frm, fb);
    double delta = left_value + right_value - whole;

    /* Simpson's local error estimate; recursively halve the error budget. */
    if (fabs(delta) <= 15.0 * tolerance) {
        *result = left_value + right_value + delta / 15.0;
        return 1;
    }
    if (depth == 0 || middle == a || middle == b) return 0;

    double left_result, right_result;
    if (!adaptive_simpson(number, a, middle, fa, flm, fm, left_value,
                          tolerance / 2.0, depth - 1, &left_result) ||
        !adaptive_simpson(number, middle, b, fm, frm, fb, right_value,
                          tolerance / 2.0, depth - 1, &right_result)) return 0;
    *result = left_result + right_result;
    return 1;
}

static int integral(double epsilon, int number, double *result) {
    /* This is an integrable logarithmic endpoint singularity. Its integral
       is evaluated analytically, rather than sampling the undefined endpoint. */
    if (number == 3) {
        *result = 1.0;
        return 1;
    }

    double a = 0.0, b = 1.0, middle = 0.5;
    double fa = integral_function(number, a);
    double fm = integral_function(number, middle);
    double fb = integral_function(number, b);
    double whole = simpson_value(a, b, fa, fm, fb);
    return adaptive_simpson(number, a, b, fa, fm, fb, whole,
                            epsilon / 2.0, 40, result);
}

int main(int argc, char **argv) {
    double epsilon;
    double x = 0.5;
    double value;

    if ((argc != 2 && argc != 3) || !read_double(argv[1], &epsilon) || epsilon <= 0.0) {
        fprintf(stderr, "\210\341\257\256\253\354\247\256\242\240\255\250\245: %s <\257\256\253\256\246\250\342\245\253\354\255\353\251 \355\257\341\250\253\256\255> [x, \257\256 \343\254\256\253\347\240\255\250\356 0.5]\n", argv[0]);
        return 1;
    }
    if (argc == 3 && !read_double(argv[2], &x)) {
        fprintf(stderr, "x \244\256\253\246\255\256 \241\353\342\354 \252\256\255\245\347\255\353\254 \244\245\251\341\342\242\250\342\245\253\354\255\353\254 \347\250\341\253\256\254.\n");
        return 1;
    }

    printf("\355\257\341\250\253\256\255 = %.12g, x = %.12g\n\n", epsilon, x);

    if (sum_a(x, epsilon, &value)) {
        printf("\341\343\254\254\240 a = %.12g\n", value);
    } else {
        printf("\341\343\254\254\240 a: \340\357\244 \255\245 \341\256\350\361\253\341\357 \250\253\250 \244\256\341\342\250\243\255\343\342 \257\340\245\244\245\253 \347\250\341\253\240 \250\342\245\340\240\346\250\251\n");
    }
    if (sum_b(x, epsilon, &value)) {
        printf("\341\343\254\254\240 b = %.12g\n", value);
    } else {
        printf("\341\343\254\254\240 b: \340\357\244 \255\245 \341\256\350\361\253\341\357 \250\253\250 \244\256\341\342\250\243\255\343\342 \257\340\245\244\245\253 \347\250\341\253\240 \250\342\245\340\240\346\250\251\n");
    }
    if (sum_c(x, epsilon, &value)) {
        printf("\341\343\254\254\240 c = %.12g\n", value);
    } else {
        printf("\341\343\254\254\240 c: \340\357\244 \255\245 \341\345\256\244\250\342\341\357 \257\340\250 |x| >= 1\n");
    }
    if (sum_d(x, epsilon, &value)) {
        printf("\341\343\254\254\240 d = %.12g\n", value);
    } else {
        printf("\341\343\254\254\240 d: \340\357\244 \255\245 \341\345\256\244\250\342\341\357 \257\340\250 |x| >= 1\n");
    }
    putchar('\n');

    if (integral(epsilon, 1, &value)) {
        printf("\250\255\342\245\243\340\240\253 a = %.12g\n", value);
    } else {
        printf("\250\255\342\245\243\340\240\253 a: \347\250\341\253\245\255\255\256\245 \250\255\342\245\243\340\250\340\256\242\240\255\250\245 \255\245 \244\256\341\342\250\243\253\256 \247\240\244\240\255\255\256\243\256 \355\257\341\250\253\256\255\240\n");
    }
    if (integral(epsilon, 2, &value)) {
        printf("\250\255\342\245\243\340\240\253 b = %.12g\n", value);
    } else {
        printf("\250\255\342\245\243\340\240\253 b: \347\250\341\253\245\255\255\256\245 \250\255\342\245\243\340\250\340\256\242\240\255\250\245 \255\245 \244\256\341\342\250\243\253\256 \247\240\244\240\255\255\256\243\256 \355\257\341\250\253\256\255\240\n");
    }
    if (integral(epsilon, 3, &value)) {
        printf("\250\255\342\245\243\340\240\253 c = %.12g\n", value);
    } else {
        printf("\250\255\342\245\243\340\240\253 c: \347\250\341\253\245\255\255\256\245 \250\255\342\245\243\340\250\340\256\242\240\255\250\245 \255\245 \244\256\341\342\250\243\253\256 \247\240\244\240\255\255\256\243\256 \355\257\341\250\253\256\255\240\n");
    }
    if (integral(epsilon, 4, &value)) {
        printf("\250\255\342\245\243\340\240\253 d = %.12g\n", value);
    } else {
        printf("\250\255\342\245\243\340\240\253 d: \347\250\341\253\245\255\255\256\245 \250\255\342\245\243\340\250\340\256\242\240\255\250\245 \255\245 \244\256\341\342\250\243\253\256 \247\240\244\240\255\255\256\243\256 \355\257\341\250\253\256\255\240\n");
    }

    return 0;
}
