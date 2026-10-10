#include <limits.h>
#include <math.h>
#include <stdarg.h>
#include <stdio.h>

static double cross_product(double ax, double ay, double bx, double by,
                            double cx, double cy)
{
    return (bx - ax) * (cy - ay) - (by - ay) * (cx - ax);
}

static int point_on_segment(double eps, double ax, double ay,
                            double bx, double by, double px, double py)
{
    double min_x;
    double max_x;
    double min_y;
    double max_y;

    if (fabs(cross_product(ax, ay, bx, by, px, py)) > eps) {
        return 0;
    }

    min_x = fmin(ax, bx) - eps;
    max_x = fmax(ax, bx) + eps;
    min_y = fmin(ay, by) - eps;
    max_y = fmax(ay, by) + eps;
    return px >= min_x && px <= max_x && py >= min_y && py <= max_y;
}

static int segments_intersect(double eps,
                              double ax, double ay, double bx, double by,
                              double cx, double cy, double dx, double dy)
{
    double first = cross_product(ax, ay, bx, by, cx, cy);
    double second = cross_product(ax, ay, bx, by, dx, dy);
    double third = cross_product(cx, cy, dx, dy, ax, ay);
    double fourth = cross_product(cx, cy, dx, dy, bx, by);

    if (((first > eps && second < -eps) || (first < -eps && second > eps)) &&
        ((third > eps && fourth < -eps) || (third < -eps && fourth > eps))) {
        return 1;
    }

    return (fabs(first) <= eps && point_on_segment(eps, ax, ay, bx, by, cx, cy)) ||
           (fabs(second) <= eps && point_on_segment(eps, ax, ay, bx, by, dx, dy)) ||
           (fabs(third) <= eps && point_on_segment(eps, cx, cy, dx, dy, ax, ay)) ||
           (fabs(fourth) <= eps && point_on_segment(eps, cx, cy, dx, dy, bx, by));
}

static int is_simple_polygon(double eps, int count,
                             const double *x, const double *y)
{
    int i;
    int j;

    for (i = 0; i < count; i++) {
        int next = (i + 1) % count;

        for (j = i + 1; j < count; j++) {
            int other_next = (j + 1) % count;

            /* Adjacent edges meet at their common polygon vertex by design. */
            if (next == j || other_next == i) {
                continue;
            }
            if (segments_intersect(eps,
                                    x[i], y[i], x[next], y[next],
                                    x[j], y[j], x[other_next], y[other_next])) {
                return 0;
            }
        }
    }
    return 1;
}

static int is_convex(double eps, int count, ...)
{
    double x[100];
    double y[100];
    va_list args;
    int i;
    int sign = 0;

    if (!isfinite(eps) || eps <= 0.0 || count < 3 || count > 100) {
        return -1;
    }

    va_start(args, count);
    for (i = 0; i < count; i++) {
        x[i] = va_arg(args, double);
        y[i] = va_arg(args, double);
        if (!isfinite(x[i]) || !isfinite(y[i])) {
            va_end(args);
            return -1;
        }
    }
    va_end(args);

    if (!is_simple_polygon(eps, count, x, y)) {
        return 0;
    }

    for (i = 0; i < count; i++) {
        int next = (i + 1) % count;
        int after = (i + 2) % count;
        double ax = x[next] - x[i];
        double ay = y[next] - y[i];
        double bx = x[after] - x[next];
        double by = y[after] - y[next];
        double cross = ax * by - ay * bx;

        if (cross > eps) {
            if (sign < 0) {
                return 0;
            }
            sign = 1;
        } else if (cross < -eps) {
            if (sign > 0) {
                return 0;
            }
            sign = -1;
        }
    }

    return sign != 0;
}

static int polynomial(double point, int degree, double *answer, ...)
{
    va_list args;
    double value;
    int i;

    if (answer == NULL || !isfinite(point) || degree < 0 || degree > 100) {
        return 0;
    }

    va_start(args, answer);
    value = va_arg(args, double);
    for (i = degree - 1; i >= 0; i--) {
        value = value * point + va_arg(args, double);
    }
    va_end(args);

    if (!isfinite(value)) {
        return 0;
    }
    *answer = value;
    return 1;
}

static int digit(char c)
{
    if (c >= '0' && c <= '9') {
        return c - '0';
    }
    if (c >= 'A' && c <= 'Z') {
        return c - 'A' + 10;
    }
    if (c >= 'a' && c <= 'z') {
        return c - 'a' + 10;
    }
    return -1;
}

static int number_from_string(const char *text, int base,
                              unsigned long long *number)
{
    unsigned long long value = 0;
    size_t i;

    if (text == NULL || text[0] == '\0' || number == NULL ||
        base < 2 || base > 36) {
        return 0;
    }

    for (i = 0; text[i] != '\0'; i++) {
        int d = digit(text[i]);
        if (d < 0 || d >= base ||
            value > (ULLONG_MAX - (unsigned long long)d) / (unsigned)base) {
            return 0;
        }
        value = value * (unsigned)base + (unsigned)d;
    }
    *number = value;
    return 1;
}

static int is_kaprekar(const char *text, int base)
{
    unsigned long long number = 0;
    unsigned long long original;
    unsigned long long square;
    unsigned long long divisor = 1;
    int i;
    int length;

    if (!number_from_string(text, base, &number) ||
        (number != 0 && number > ULLONG_MAX / number)) {
        return -1;
    }

    original = number;
    square = number * number;
    length = 0;
    do {
        length++;
        number /= (unsigned)base;
    } while (number != 0);

    for (i = 0; i < length; i++) {
        if (divisor > ULLONG_MAX / (unsigned)base) {
            return -1;
        }
        divisor *= (unsigned)base;
    }

    return square / divisor + square % divisor == original;
}

static int find_kaprekar(int base, const char **result, int count, ...)
{
    va_list args;
    int i;
    int found = 0;

    if (base < 2 || base > 36 || count < 0 || result == NULL) {
        return -1;
    }

    va_start(args, count);
    for (i = 0; i < count; i++) {
        const char *text = va_arg(args, const char *);
        int is_number = is_kaprekar(text, base);
        if (is_number < 0) {
            va_end(args);
            return -1;
        }
        if (is_number) {
            result[found] = text;
            found++;
        }
    }
    va_end(args);
    return found;
}

static int geometric_mean(double *answer, int count, ...)
{
    va_list args;
    double product = 1.0;
    int i;

    if (answer == NULL || count <= 0) {
        return 0;
    }

    va_start(args, count);
    for (i = 0; i < count; i++) {
        double value = va_arg(args, double);
        if (!isfinite(value) || value <= 0.0) {
            va_end(args);
            return 0;
        }
        product *= value;
        if (!isfinite(product)) {
            va_end(args);
            return 0;
        }
    }
    va_end(args);

    *answer = pow(product, 1.0 / count);
    return isfinite(*answer);
}

static int fast_power_positive(double base, unsigned long long exponent,
                               double *answer)
{
    double half;

    if (exponent == 0) {
        *answer = 1.0;
        return 1;
    }
    if (!fast_power_positive(base, exponent / 2, &half)) {
        return 0;
    }
    *answer = half * half;
    if (!isfinite(*answer)) {
        return 0;
    }
    if (exponent % 2 != 0) {
        *answer *= base;
    }
    return isfinite(*answer);
}

static int fast_power(double base, long long exponent, double *answer)
{
    unsigned long long positive_exponent;
    double value;

    if (answer == NULL || !isfinite(base) ||
        (base == 0.0 && exponent < 0)) {
        return 0;
    }

    if (exponent < 0) {
        positive_exponent = (unsigned long long)(-(exponent + 1)) + 1;
    } else {
        positive_exponent = (unsigned long long)exponent;
    }

    if (!fast_power_positive(base, positive_exponent, &value)) {
        return 0;
    }
    *answer = exponent < 0 ? 1.0 / value : value;
    return isfinite(*answer);
}

typedef double (*Equation)(double);

static int bisection(double left, double right, double eps,
                     Equation function, double *root)
{
    double left_value;
    double right_value;
    int i;

    if (root == NULL || function == NULL || !isfinite(left) ||
        !isfinite(right) || !isfinite(eps) || eps <= 0.0 || left >= right) {
        return 0;
    }

    left_value = function(left);
    right_value = function(right);
    if (!isfinite(left_value) || !isfinite(right_value) ||
        (left_value > 0.0 && right_value > 0.0) ||
        (left_value < 0.0 && right_value < 0.0)) {
        return 0;
    }

    for (i = 0; i < 100000; i++) {
        double middle = left + (right - left) / 2.0;
        double middle_value = function(middle);

        if (!isfinite(middle_value)) {
            return 0;
        }
        if (fabs(middle_value) <= eps || right - left <= eps) {
            *root = middle;
            return 1;
        }
        if ((left_value <= 0.0 && middle_value >= 0.0) ||
            (left_value >= 0.0 && middle_value <= 0.0)) {
            right = middle;
        } else {
            left = middle;
            left_value = middle_value;
        }
    }
    return 0;
}

static double equation_square(double x)
{
    return x * x - 2.0;
}

static double equation_cosine(double x)
{
    return cos(x) - x;
}

static double equation_exponential(double x)
{
    return exp(x) - 3.0;
}

static void show_root(const char *name, Equation function,
                      double left, double right, double eps)
{
    double root;

    if (bisection(left, right, eps, function, &root)) {
        printf("%s: %.10g\n", name, root);
    } else {
        printf("%s: \252\256\340\245\255\354 \255\245 \255\240\251\244\245\255\n", name);
    }
}

int main(void)
{
    double answer;
    int convex;
    const char *kaprekar[8];
    int kaprekar_count;

    convex = is_convex(1e-9, 4,
                       0.0, 0.0, 4.0, 0.0,
                       4.0, 3.0, 0.0, 3.0);
    if (convex == 1) {
        puts("1. \214\255\256\243\256\343\243\256\253\354\255\250\252 \242\353\257\343\252\253\353\251");
    } else {
        puts("1. \214\255\256\243\256\343\243\256\253\354\255\250\252 \255\245\242\353\257\343\252\253\353\251 \250\253\250 \242\345\256\244\255\353\245 \244\240\255\255\353\245 \255\245\244\256\257\343\341\342\250\254\353");
    }

    if (polynomial(2.0, 3, &answer, 1.0, -2.0, 0.0, 5.0)) {
        printf("2. \207\255\240\347\245\255\250\245 \254\255\256\243\256\347\253\245\255\240: %.10g\n", answer);
    } else {
        puts("2. \202\345\256\244\255\353\245 \244\240\255\255\353\245 \254\255\256\243\256\347\253\245\255\240 \255\245\244\256\257\343\341\342\250\254\353");
    }

    printf("3. \227\250\341\253\240 \212\240\257\340\245\252\240\340\240: ");
    kaprekar_count = find_kaprekar(10, kaprekar, 8,
                                   "1", "9", "10", "45",
                                   "55", "99", "100", "101");
    if (kaprekar_count < 0) {
        puts("\242\345\256\244\255\353\245 \244\240\255\255\353\245 \255\245\244\256\257\343\341\342\250\254\353");
    } else {
        for (convex = 0; convex < kaprekar_count; convex++) {
            if (convex > 0) {
                putchar(' ');
            }
            printf("%s", kaprekar[convex]);
        }
        putchar('\n');
    }

    if (geometric_mean(&answer, 4, 1.0, 2.0, 4.0, 8.0)) {
        printf("4. \221\340\245\244\255\245\245 \243\245\256\254\245\342\340\250\347\245\341\252\256\245: %.10g\n", answer);
    } else {
        puts("4. \202\345\256\244\255\353\245 \244\240\255\255\353\245 \244\253\357 \341\340\245\244\255\245\243\256 \243\245\256\254\245\342\340\250\347\245\341\252\256\243\256 \255\245\244\256\257\343\341\342\250\254\353");
    }

    if (fast_power(2.0, -5, &answer)) {
        printf("5. \207\255\240\347\245\255\250\245 \341\342\245\257\245\255\250: %.10g\n", answer);
    } else {
        puts("5. \202\345\256\244\255\353\245 \244\240\255\255\353\245 \244\253\357 \242\256\247\242\245\244\245\255\250\357 \242 \341\342\245\257\245\255\354 \255\245\244\256\257\343\341\342\250\254\353");
    }

    show_root("6a. x^2 - 2", equation_square, 0.0, 2.0, 1e-9);
    show_root("6b. cos(x) - x", equation_cosine, 0.0, 1.0, 1e-9);
    show_root("6c. exp(x) - 3", equation_exponential, 0.0, 2.0, 1e-9);
    return 0;
}
