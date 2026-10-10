#include <errno.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX_ITERATIONS 100000000ULL

enum CalculationStatus {
    CALC_OK,
    CALC_NOT_CONVERGED
};

static int parse_epsilon(const char *text, double *epsilon) {
    char *end;
    double value;

    if (text == NULL || text[0] == '\0') {
        return 0;
    }

    errno = 0;
    value = strtod(text, &end);

    if (errno == ERANGE || end == text || *end != '\0' ||
        !isfinite(value) || value <= 0.0) {
        return 0;
    }

    *epsilon = value;
    return 1;
}

static int iterations_for(double epsilon,
                          double multiplier,
                          unsigned long long *iterations) {
    long double required = ceill((long double)multiplier / (long double)epsilon);

    /* A capped iteration count is not a valid convergence result. */
    if (!isfinite(required) || required > (long double)MAX_ITERATIONS) {
        return CALC_NOT_CONVERGED;
    }

    if (required < 1.0L) {
        required = 1.0L;
    }

    *iterations = (unsigned long long)required;
    return CALC_OK;
}

static int calculate_e_series(double epsilon, double *result) {
    double sum = 1.0;
    double term = 1.0;
    unsigned long long n;

    for (n = 1ULL; n <= MAX_ITERATIONS; ++n) {
        term /= (double)n;
        sum += term;

        if (term <= epsilon) {
            *result = sum;
            return CALC_OK;
        }
    }

    return CALC_NOT_CONVERGED;
}

static int calculate_e_equation(double epsilon, double *result) {
    double left = 2.0;
    double right = 3.0;
    unsigned long long iteration;

    for (iteration = 0ULL; iteration < MAX_ITERATIONS; ++iteration) {
        double middle = (left + right) / 2.0;

        if (log(middle) < 1.0) {
            left = middle;
        } else {
            right = middle;
        }

        if (right - left <= epsilon) {
            *result = (left + right) / 2.0;
            return CALC_OK;
        }
    }

    return CALC_NOT_CONVERGED;
}

static int calculate_e_limit(double epsilon, double *result) {
    unsigned long long n;
    long double number;

    if (iterations_for(epsilon, 2.0, &n) != CALC_OK) {
        return CALC_NOT_CONVERGED;
    }

    number = (long double)n;
    long double value = powl(1.0L + 1.0L / number, number);

    if (!isfinite((double)value)) {
        return CALC_NOT_CONVERGED;
    }

    *result = (double)value;
    return CALC_OK;
}

static int calculate_pi_series(double epsilon, double *result) {
    double sum = 0.0;
    unsigned long long n;

    for (n = 1ULL; n <= MAX_ITERATIONS; ++n) {
        double term = 4.0 / (2.0 * (double)n - 1.0);

        if (n % 2ULL == 1ULL) {
            sum += term;
        } else {
            sum -= term;
        }

        if (term <= epsilon) {
            *result = sum;
            return CALC_OK;
        }
    }

    return CALC_NOT_CONVERGED;
}

static int calculate_pi_equation(double epsilon, double *result) {
    /* cos(x) = -1 is equivalent to cos(x / 2) = 0 near pi.
       The latter has a sign change, so bisection applies directly. */
    double left = 3.0;
    double right = 3.3;
    unsigned long long iteration;

    for (iteration = 0ULL; iteration < MAX_ITERATIONS; ++iteration) {
        double middle = left + (right - left) / 2.0;
        if (middle == left || middle == right) break;
        double f_left = cos(left / 2.0);
        double f_middle = cos(middle / 2.0);

        if (f_left * f_middle <= 0.0) {
            right = middle;
        } else {
            left = middle;
        }

        if ((right - left) / 2.0 <= epsilon) {
            *result = left + (right - left) / 2.0;
            return CALC_OK;
        }
    }

    return CALC_NOT_CONVERGED;
}

static int calculate_pi_limit(double epsilon, double *result) {
    unsigned long long n;
    long double number;
    long double logarithm;
    long double value;

    if (iterations_for(epsilon, 1.0, &n) != CALC_OK) {
        return CALC_NOT_CONVERGED;
    }

    number = (long double)n;
    logarithm = 4.0L * (number * logl(2.0L) + lgammal(number + 1.0L))
                - logl(number) - 2.0L * lgammal(2.0L * number + 1.0L);
    value = expl(logarithm);

    if (!isfinite((double)value)) {
        return CALC_NOT_CONVERGED;
    }

    *result = (double)value;
    return CALC_OK;
}

static int calculate_ln2_series(double epsilon, double *result) {
    double sum = 0.0;
    unsigned long long n;

    for (n = 1ULL; n <= MAX_ITERATIONS; ++n) {
        double term = 1.0 / (double)n;

        if (n % 2ULL == 1ULL) {
            sum += term;
        } else {
            sum -= term;
        }

        if (term <= epsilon) {
            *result = sum;
            return CALC_OK;
        }
    }

    return CALC_NOT_CONVERGED;
}

static int calculate_ln2_equation(double epsilon, double *result) {
    double value = 0.7;
    unsigned long long iteration;

    for (iteration = 0ULL; iteration < MAX_ITERATIONS; ++iteration) {
        double exponential = exp(value);
        double next = value - (exponential - 2.0) / exponential;

        if (!isfinite(next)) {
            return CALC_NOT_CONVERGED;
        }
        if (fabs(next - value) <= epsilon) {
            *result = next;
            return CALC_OK;
        }

        value = next;
    }

    return CALC_NOT_CONVERGED;
}

static int calculate_ln2_limit(double epsilon, double *result) {
    unsigned long long n;
    long double number;

    if (iterations_for(epsilon, 1.0, &n) != CALC_OK) {
        return CALC_NOT_CONVERGED;
    }

    number = (long double)n;
    long double value = number * (powl(2.0L, 1.0L / number) - 1.0L);

    if (!isfinite((double)value)) {
        return CALC_NOT_CONVERGED;
    }

    *result = (double)value;
    return CALC_OK;
}

static int calculate_sqrt2_product(double epsilon, double *result) {
    long double product = 1.0L;
    long double exponent = 0.25L;
    unsigned long long iteration;

    for (iteration = 0ULL; iteration < MAX_ITERATIONS; ++iteration) {
        long double term = powl(2.0L, exponent);

        product *= term;
        exponent /= 2.0L;

        if (fabsl(term - 1.0L) <= (long double)epsilon) {
            *result = (double)product;
            return isfinite(*result) ? CALC_OK : CALC_NOT_CONVERGED;
        }
    }

    return CALC_NOT_CONVERGED;
}

static int calculate_sqrt2_equation(double epsilon, double *result) {
    double value = 1.0;
    unsigned long long iteration;

    for (iteration = 0ULL; iteration < MAX_ITERATIONS; ++iteration) {
        double next = (value + 2.0 / value) / 2.0;

        if (fabs(next - value) <= epsilon) {
            *result = next;
            return CALC_OK;
        }

        value = next;
    }

    return CALC_NOT_CONVERGED;
}

static int calculate_sqrt2_limit(double epsilon, double *result) {
    double value = -0.5;
    unsigned long long iteration;

    for (iteration = 0ULL; iteration < MAX_ITERATIONS; ++iteration) {
        double next = value - value * value / 2.0 + 1.0;

        if (fabs(next - value) <= epsilon) {
            *result = next;
            return CALC_OK;
        }

        value = next;
    }

    return CALC_NOT_CONVERGED;
}

static int calculate_gamma_series(double epsilon, double *result) {
    double sum = 0.0;
    unsigned long long n;

    for (n = 1ULL; n <= MAX_ITERATIONS; ++n) {
        double inverse = 1.0 / (double)n;

        sum += inverse - log1p(inverse);

        if (inverse <= epsilon) {
            *result = sum;
            return CALC_OK;
        }
    }

    return CALC_NOT_CONVERGED;
}

/* The Euler constant is the unique root of
   x + integral_0^infinity(exp(-t) log(t) dt) = 0.
   With t = exp(u), the integral has a rapidly decaying integrand
   exp(-exp(u)) * u * exp(u). */
static double gamma_integrand(double u) {
    double t = exp(u);
    if (!isfinite(t)) {
        return 0.0;
    }
    return u * t * exp(-t);
}

static double simpson_value(double a, double b,
                            double fa, double fm, double fb) {
    return (b - a) * (fa + 4.0 * fm + fb) / 6.0;
}

static int adaptive_simpson_gamma(double a, double b,
                                  double fa, double fm, double fb,
                                  double whole, double tolerance, int depth,
                                  double *integral) {
    double middle = a + (b - a) / 2.0;
    double left_middle = a + (middle - a) / 2.0;
    double right_middle = middle + (b - middle) / 2.0;
    double flm = gamma_integrand(left_middle);
    double frm = gamma_integrand(right_middle);
    double left_value = simpson_value(a, middle, fa, flm, fm);
    double right_value = simpson_value(middle, b, fm, frm, fb);
    double delta = left_value + right_value - whole;

    if (fabs(delta) <= 15.0 * tolerance) {
        *integral = left_value + right_value + delta / 15.0;
        return CALC_OK;
    }
    if (depth == 0 || middle == a || middle == b) {
        return CALC_NOT_CONVERGED;
    }

    double left_integral;
    double right_integral;
    if (adaptive_simpson_gamma(a, middle, fa, flm, fm, left_value,
                               tolerance / 2.0, depth - 1, &left_integral) != CALC_OK ||
        adaptive_simpson_gamma(middle, b, fm, frm, fb, right_value,
                               tolerance / 2.0, depth - 1, &right_integral) != CALC_OK) {
        return CALC_NOT_CONVERGED;
    }
    *integral = left_integral + right_integral;
    return CALC_OK;
}

static int calculate_gamma_equation(double epsilon, double *result) {
    double lower = -20.0;
    double upper = log(40.0);
    double a, b, middle, fa, fm, fb, whole, integral;
    double left = 0.0;
    double right = 1.0;
    unsigned long long iteration;

    /* Grow the finite integration interval until the omitted tails are
       below a fraction of the requested error. */
    while ((1.0 - lower) * exp(lower) > epsilon / 16.0) {
        lower -= 2.0;
        if (lower < -740.0) return CALC_NOT_CONVERGED;
    }
    while (exp(-exp(upper)) * (upper + 2.0) > epsilon / 16.0) {
        upper += 0.5;
        if (upper > log(740.0)) return CALC_NOT_CONVERGED;
    }

    a = lower; b = upper; middle = a + (b - a) / 2.0;
    fa = gamma_integrand(a); fm = gamma_integrand(middle); fb = gamma_integrand(b);
    whole = simpson_value(a, b, fa, fm, fb);
    if (adaptive_simpson_gamma(a, b, fa, fm, fb, whole,
                               epsilon / 16.0, 40, &integral) != CALC_OK) {
        return CALC_NOT_CONVERGED;
    }

    /* Bisection solves x + integral = 0; this avoids differentiating tgamma. */
    for (iteration = 0ULL; iteration < 128ULL; ++iteration) {
        double x = left + (right - left) / 2.0;
        if (x + integral > 0.0) right = x;
        else left = x;
        if ((right - left) / 2.0 <= epsilon / 2.0) {
            *result = left + (right - left) / 2.0;
            return CALC_OK;
        }
    }
    return CALC_NOT_CONVERGED;
}

static int calculate_gamma_limit(double epsilon, double *result) {
    unsigned long long n;
    double sum = 0.0;

    if (iterations_for(epsilon, 1.0, &n) != CALC_OK) {
        return CALC_NOT_CONVERGED;
    }
    unsigned long long k;

    for (k = 1ULL; k <= n; ++k) {
        sum += 1.0 / (double)k;
    }

    *result = sum - log((double)n);
    return isfinite(*result) ? CALC_OK : CALC_NOT_CONVERGED;
}

static void print_result(const char *name, int status, double value) {
    if (status == CALC_OK) {
        printf("%-15s %.12g\n", name, value);
    } else {
        printf("%-15s \242\353\347\250\341\253\245\255\250\245 \255\245 \341\256\350\253\256\341\354\n", name);
    }
}

static void print_calculation(const char *name,
                              int (*calculate)(double, double *),
                              double epsilon) {
    double value = 0.0;
    int status = calculate(epsilon, &value);

    print_result(name, status, value);
}

static void print_report(double epsilon) {
    printf("\355\257\341\250\253\256\255 = %.12g\n\n", epsilon);

    print_calculation("\340\357\244 \244\253\357 e", calculate_e_series, epsilon);
    print_calculation("\343\340\240\242\255\245\255\250\245 \244\253\357 e", calculate_e_equation, epsilon);
    print_calculation("\257\340\245\244\245\253 \244\253\357 e", calculate_e_limit, epsilon);
    putchar('\n');

    print_calculation("\340\357\244 \244\253\357 \257\250", calculate_pi_series, epsilon);
    print_calculation("\343\340\240\242\255\245\255\250\245 \244\253\357 \257\250", calculate_pi_equation, epsilon);
    print_calculation("\257\340\245\244\245\253 \244\253\357 \257\250", calculate_pi_limit, epsilon);
    putchar('\n');

    print_calculation("\340\357\244 \244\253\357 ln(2)", calculate_ln2_series, epsilon);
    print_calculation("\343\340\240\242\255\245\255\250\245 \244\253\357 ln(2)", calculate_ln2_equation, epsilon);
    print_calculation("\257\340\245\244\245\253 \244\253\357 ln(2)", calculate_ln2_limit, epsilon);
    putchar('\n');

    print_calculation("\257\340\256\250\247\242\245\244\245\255\250\245 \244\253\357 \252\256\340\255\357 \250\247 2", calculate_sqrt2_product, epsilon);
    print_calculation("\343\340\240\242\255\245\255\250\245 \244\253\357 \252\256\340\255\357 \250\247 2", calculate_sqrt2_equation, epsilon);
    print_calculation("\257\340\245\244\245\253 \244\253\357 \252\256\340\255\357 \250\247 2", calculate_sqrt2_limit, epsilon);
    putchar('\n');

    print_calculation("\340\357\244 \244\253\357 \243\240\254\254\240", calculate_gamma_series, epsilon);
    print_calculation("\343\340\240\242\255\245\255\250\245 \244\253\357 \243\240\254\254\240", calculate_gamma_equation, epsilon);
    print_calculation("\257\340\245\244\245\253 \244\253\357 \243\240\254\254\240", calculate_gamma_limit, epsilon);
}

int main(int argc, char **argv) {
    double epsilon;

    if (argc != 2 || !parse_epsilon(argv[1], &epsilon)) {
        fprintf(stderr, "\210\341\257\256\253\354\247\256\242\240\255\250\245: %s <\257\256\253\256\246\250\342\245\253\354\255\353\251 \355\257\341\250\253\256\255>\n", argv[0]);
        return 1;
    }

    print_report(epsilon);
    return 0;
}
