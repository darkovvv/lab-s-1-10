#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

static int parse_int(const char *text, int *value) {
    char *end = NULL;
    long parsed;

    if (text == NULL || text[0] == '\0') {
        return 0;
    }
    errno = 0;
    parsed = strtol(text, &end, 10);
    if (errno == ERANGE || end == text || *end != '\0' ||
        parsed < INT_MIN || parsed > INT_MAX) {
        return 0;
    }
    *value = (int)parsed;
    return 1;
}

static int random_in_range(int minimum, int maximum) {
    double fraction = (double)rand() / ((double)RAND_MAX + 1.0);
    long long range = (long long)maximum - (long long)minimum + 1LL;
    return (int)((long long)minimum + (long long)(fraction * (double)range));
}

static void print_array(const char *name, const int *array, size_t size) {
    size_t shown = size < 20U ? size : 20U;
    size_t i;

    printf("%s (\340\240\247\254\245\340 %zu):", name, size);
    for (i = 0U; i < shown; ++i) {
        printf(" %d", array[i]);
    }
    if (shown < size) {
        printf(" ...");
    }
    putchar('\n');
}

static int find_and_swap_min_max(int *array, size_t size, int *minimum, int *maximum) {
    size_t minimum_index = 0U;
    size_t maximum_index = 0U;
    size_t i;

    if (array == NULL || minimum == NULL || maximum == NULL || size == 0U) {
        return 0;
    }
    for (i = 1U; i < size; ++i) {
        if (array[i] < array[minimum_index]) {
            minimum_index = i;
        }
        if (array[i] > array[maximum_index]) {
            maximum_index = i;
        }
    }
    *minimum = array[minimum_index];
    *maximum = array[maximum_index];
    {
        int temporary = array[minimum_index];
        array[minimum_index] = array[maximum_index];
        array[maximum_index] = temporary;
    }
    return 1;
}

static int *create_random_array(size_t size, int minimum, int maximum) {
    int *array = (int *)malloc(size * sizeof(*array));
    size_t i;

    if (array == NULL) {
        return NULL;
    }
    for (i = 0U; i < size; ++i) {
        array[i] = random_in_range(minimum, maximum);
    }
    return array;
}

static int make_array_c(const int *array_a, size_t size_a,
                        const int *array_b, size_t size_b, int **array_c) {
    int *result;
    size_t i;

    if (array_a == NULL || array_b == NULL || array_c == NULL || size_a == 0U || size_b == 0U) {
        return 0;
    }
    result = (int *)malloc(size_a * sizeof(*result));
    if (result == NULL) {
        return 0;
    }
    for (i = 0U; i < size_a; ++i) {
        size_t j;
        int nearest = array_b[0];
        int best_distance = abs(array_a[i] - nearest);

        for (j = 1U; j < size_b; ++j) {
            int distance = abs(array_a[i] - array_b[j]);
            if (distance < best_distance) {
                best_distance = distance;
                nearest = array_b[j];
            }
        }
        result[i] = array_a[i] + nearest;
    }
    *array_c = result;
    return 1;
}

int main(int argc, char **argv) {
    const size_t fixed_size = 10U;
    int lower;
    int upper;
    int fixed_array[10];
    int minimum;
    int maximum;
    size_t size_a;
    size_t size_b;
    int *array_a = NULL;
    int *array_b = NULL;
    int *array_c = NULL;
    size_t i;

    if (argc != 3 || !parse_int(argv[1], &lower) || !parse_int(argv[2], &upper) || lower > upper) {
        fprintf(stderr, "\210\341\257\256\253\354\247\256\242\240\255\250\245: %s <a> <b>, \243\244\245 a <= b\n", argv[0]);
        return 1;
    }
    srand((unsigned int)time(NULL));

    for (i = 0U; i < fixed_size; ++i) {
        fixed_array[i] = random_in_range(lower, upper);
    }
    print_array("\224\250\252\341\250\340\256\242\240\255\255\353\251 \254\240\341\341\250\242 \244\256", fixed_array, fixed_size);
    if (!find_and_swap_min_max(fixed_array, fixed_size, &minimum, &maximum)) {
        puts("\215\245 \343\244\240\253\256\341\354 \256\241\340\240\241\256\342\240\342\354 \344\250\252\341\250\340\256\242\240\255\255\353\251 \254\240\341\341\250\242.");
        return 1;
    }
    printf("\214\250\255\250\254\343\254 = %d, \254\240\252\341\250\254\343\254 = %d\n", minimum, maximum);
    print_array("\224\250\252\341\250\340\256\242\240\255\255\353\251 \254\240\341\341\250\242 \257\256\341\253\245", fixed_array, fixed_size);

    size_a = (size_t)random_in_range(10, 10000);
    size_b = (size_t)random_in_range(10, 10000);
    array_a = create_random_array(size_a, -1000, 1000);
    array_b = create_random_array(size_b, -1000, 1000);
    if (array_a == NULL || array_b == NULL) {
        fprintf(stderr, "\215\245\244\256\341\342\240\342\256\347\255\256 \257\240\254\357\342\250 \244\253\357 \244\250\255\240\254\250\347\245\341\252\250\345 \254\240\341\341\250\242\256\242.\n");
        free(array_a);
        free(array_b);
        return 1;
    }
    if (!make_array_c(array_a, size_a, array_b, size_b, &array_c)) {
        fprintf(stderr, "\215\245 \343\244\240\253\256\341\354 \341\344\256\340\254\250\340\256\242\240\342\354 \254\240\341\341\250\242 C.\n");
        free(array_a);
        free(array_b);
        return 1;
    }
    print_array("A", array_a, size_a);
    print_array("B", array_b, size_b);
    print_array("C", array_c, size_a);

    free(array_c);
    free(array_b);
    free(array_a);
    return 0;
}
