#define __STDC_WANT_LIB_EXT1__ 1

#include "hashmap.h"
#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

FILE *open_file(const char *filename) {
    FILE *file = NULL;

    errno_t res = fopen_s(&file, filename, "r");
    if (res != 0) {
        char message[256];
        errno_t error_res = strerror_s(message, sizeof(message), res);
        assert(error_res == 0);
        fprintf(stderr, "Error opening file %s: %s\n", filename, message);
        assert(false);
    }

    return file;
}

struct Array {
    int size;
    int capacity;
    int64_t *data;
};

void array_new(struct Array *arr, int capacity) {
    arr->capacity = capacity;
    arr->size = 0;
    arr->data = malloc(sizeof(int64_t) * capacity);
}

void array_grow1(struct Array *arr) {
    if (arr->size == arr->capacity) {
        arr->capacity *= 2;
        arr->data = realloc(arr->data, sizeof(int64_t) * arr->capacity);
    }
}

void array_push(struct Array *arr, int64_t value) {
    array_grow1(arr);
    arr->data[arr->size] = value;
    arr->size++;
}

void array_insert(struct Array *arr, int index, int64_t value) {
    array_grow1(arr);
    for (int i = arr->size; i > index; i--) {
        arr->data[i] = arr->data[i - 1];
    }
    arr->data[index] = value;
    arr->size++;
}

void array_free(struct Array *arr) { free(arr->data); }

bool parseint(char **in, int64_t *out) {
    bool found = false;
    *out = 0;
    while (true) {
        if (!found && **in == ' ') {
            (*in)++;
            continue;
        } else if (**in >= '0' && (**in) <= '9') {
            found = true;
            *out *= 10;
            *out += **in - '0';
            (*in)++;
        } else {
            (*in)++;
            return found;
        }
    }
}

const size_t MAX_LINE_LENGTH = 50000;

int num_of_digits(int64_t num) {
    int count = 0;
    while (num > 0) {
        num /= 10;
        count++;
    }
    return count;
}

void split_digits(int64_t num, int n_digits, int64_t *left, int64_t *right) {
    int divisor = (int)pow(10., (float)n_digits / 2.);
    *right = num % divisor;
    *left = num / divisor;
}

void stones_sum_or_insert(struct hashmap_s *next_stones, int64_t *num,
                          int64_t *count) {
    int64_t *prev_count = hashmap_get(next_stones, num, sizeof(int64_t));
    if (prev_count != NULL) {
        // printf("Found previous count %lld\n", *prev_count);
        *count += *prev_count;
    }
    // printf("Inserting %lld with count %lld\n", *num, *count);
    assert(hashmap_put(next_stones, num, sizeof(int64_t), count) == 0);
}

int stones_run_step(void *const context, struct hashmap_element_s *const e) {
    struct hashmap_s *next_stones = (struct hashmap_s *)context;
    int64_t *num = (int64_t *)e->key;
    int64_t *count = (int64_t *)e->data;
    // printf("%lld %lld\n", *num, *count);
    if (*num == 0) {
        (*num)++;
        // printf("%lld with count %lld becomes %lld\n", *num - 1, *count,
        // *num);
        stones_sum_or_insert(next_stones, num, count);
        return -1;
    }
    int n_digits = num_of_digits(*num);
    int64_t *left = malloc(sizeof(int64_t));
    int64_t *right = malloc(sizeof(int64_t));
    if (n_digits % 2 == 0) {
        split_digits(*num, n_digits, left, right);
        // printf("%lld with count %lld becomes %lld and %lld\n", *num, *count,
        //        *left, *right);
        int64_t *right_count = malloc(sizeof(int64_t));
        *right_count = *count;
        stones_sum_or_insert(next_stones, left, count);
        stones_sum_or_insert(next_stones, right, right_count);
    } else {
        free(left);
        free(right);
        *num *= 2024;
        // printf("%lld with count %lld becomes %lld\n", *num / 2024, *count,
        //        *num);
        stones_sum_or_insert(next_stones, num, count);
    }
    return -1;
}

int print_stones(void *const context, struct hashmap_element_s *const e) {
    int64_t *num = (int64_t *)e->key;
    int64_t *count = (int64_t *)e->data;
    printf("%lld with count %lld\n", *num, *count);
    return 0;
}

int stones_sum(void *const context, void *const value) {
    int64_t *sum = (int64_t *)context;
    *sum += *(int64_t *)value;
    return 1;
}

int main() {
    FILE *file = open_file("input");

    char line[MAX_LINE_LENGTH];

    struct hashmap_s *stones = malloc(sizeof(struct hashmap_s));
    hashmap_create(128, stones);

    while (fgets(line, sizeof(line), file) != NULL) {
        char *cursor = line;
        int64_t stone_num;
        while (parseint(&cursor, &stone_num)) {
            int64_t *stone_num_ptr = malloc(sizeof(int64_t));
            *stone_num_ptr = stone_num;
            int64_t *count_ptr = malloc(sizeof(int64_t));
            *count_ptr = 1;
            hashmap_put(stones, stone_num_ptr, sizeof(int64_t), count_ptr);
        }
    }

    struct hashmap_s *next_stones = malloc(sizeof(struct hashmap_s));
    hashmap_create(128, next_stones);
    for (int i = 0; i < 75; i++) {
        assert(hashmap_iterate_pairs(stones, stones_run_step, next_stones) ==
               0);
        struct hashmap_s *tmp = next_stones;
        next_stones = stones;
        stones = tmp;
        // printf("\nAfter step %d:\n", i);
        // assert(hashmap_iterate_pairs(stones, print_stones, NULL) == 0);
        // printf("\n");
    }

    int64_t sum = 0;
    assert(hashmap_iterate(stones, stones_sum, &sum) == 0);

    printf("Result: %lld\n", sum);

    fclose(file);
}
