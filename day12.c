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

char *substring(const char *start, const char *end) {
    assert(start != NULL || end != NULL || start < end);

    size_t length = end - start;

    char *dest = malloc(length + 1);
    assert(dest != NULL);

    errno_t res = strncpy_s(dest, length + 1, start, length);
    assert(res == 0);

    dest[length] = '\0';

    return dest;
}

struct Array {
    int size;
    int capacity;
    char *data;
};

void array_new(struct Array *arr, int capacity) {
    arr->capacity = capacity;
    arr->size = 0;
    arr->data = malloc(sizeof(char) * capacity);
}

void array_grow1(struct Array *arr) {
    if (arr->size == arr->capacity) {
        arr->capacity *= 2;
        arr->data = realloc(arr->data, sizeof(char) * arr->capacity);
    }
}

void array_push(struct Array *arr, char value) {
    array_grow1(arr);
    arr->data[arr->size] = value;
    arr->size++;
}

void array_free(struct Array *arr) { free(arr->data); }

struct Pos {
    int x;
    int y;
};

struct PosArray {
    int size;
    int capacity;
    struct Pos *data;
};

void pos_array_new(struct PosArray *arr, int capacity) {
    arr->capacity = capacity;
    arr->size = 0;
    arr->data = malloc(sizeof(struct Pos) * capacity);
}

void pos_array_push(struct PosArray *arr, struct Pos value) {
    if (arr->size == arr->capacity) {
        arr->capacity *= 2;
        arr->data = realloc(arr->data, sizeof(struct Pos) * arr->capacity);
    }
    arr->data[arr->size] = value;
    arr->size++;
}

struct Pos pos_array_pop(struct PosArray *arr) {
    arr->size--;
    return arr->data[arr->size];
}

void pos_array_free(struct PosArray *arr) { free(arr->data); }

struct Region {
    char name;
    int size;
    int perimeter;
};

struct RegionArray {
    int size;
    int capacity;
    struct Region **data;
};

void reg_array_new(struct RegionArray *arr, int capacity) {
    arr->capacity = capacity;
    arr->size = 0;
    arr->data = malloc(sizeof(struct Region *) * capacity);
}

void reg_array_grow1(struct RegionArray *arr) {
    if (arr->size == arr->capacity) {
        arr->capacity *= 2;
        arr->data = realloc(arr->data, sizeof(struct Region *) * arr->capacity);
    }
}

void reg_array_push(struct RegionArray *arr, struct Region *value) {
    reg_array_grow1(arr);
    arr->data[arr->size] = value;
    arr->size++;
}

const size_t MAX_LINE_LENGTH = 50000;

bool in_bounds(int width, int height, int x, int y) {
    return x >= 0 && x < width && y >= 0 && y < height;
}

const int DIRECTIONS[4][2] = {
    {0, 1},
    {0, -1},
    {1, 0},
    {-1, 0},
};

int sum_prices(void *const context, void *const value) {
    int64_t *sum = (int64_t *)context;
    struct Region *v = (struct Region *)value;
    printf("%d * %d\n", v->perimeter, v->size);
    *sum += v->perimeter * v->size;
    return 1;
}

int main() {
    FILE *file = open_file("input");

    char line[MAX_LINE_LENGTH];

    struct Array map;
    array_new(&map, 8);
    int width = 0;
    int height = 0;

    while (fgets(line, sizeof(line), file) != NULL) {
        char *cursor = line;
        int w = 0;
        while (*cursor != '\n' && *cursor != '\0') {
            array_push(&map, *cursor);
            w++;
            cursor++;
        }
        if (height == 0) {
            width = w;
        }
        height++;
    }

    struct hashmap_s visited;
    hashmap_create(128, &visited);

    struct RegionArray regions;
    reg_array_new(&regions, 8);

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            // printf("%d, %d\n", x, y);
            char name = map.data[y * width + x];
            struct Pos *pos = malloc(sizeof(struct Pos));
            pos->x = x;
            pos->y = y;
            struct Region *region = hashmap_get(&visited, pos, sizeof(struct Pos));

            if (region == NULL) {
                region = malloc(sizeof(struct Region));
                region->name = name;
                region->size = 1;
                region->perimeter = 0;
                assert(hashmap_put(&visited, pos, sizeof(struct Pos), region) == 0);
                reg_array_push(&regions, region);

                struct PosArray stack;
                pos_array_new(&stack, 8);
                pos_array_push(&stack, *pos);
                while (stack.size > 0) {
                    struct Pos cur = pos_array_pop(&stack);
                    for (int i = 0; i < 4; i++) {
                        struct Pos *np = malloc(sizeof(struct Pos));
                        np->x = cur.x + DIRECTIONS[i][0];
                        np->y = cur.y + DIRECTIONS[i][1];

                        if (in_bounds(width, height, np->x, np->y) &&
                            map.data[np->y * width + np->x] == name) {
                            if (hashmap_get(&visited, np, sizeof(struct Pos)) == NULL) {
                                assert(hashmap_put(&visited, np, sizeof(struct Pos), region) == 0);
                                pos_array_push(&stack, *np);
                                region->size += 1;
                            }
                        } else {
                            region->perimeter += 1;
                        }
                    }
                }
            }
        }
    }

    int64_t sum = 0;
    for (int i = 0; i < regions.size; i++) {
        struct Region *r = regions.data[i];
        // printf("%c: %d * %d\n", r->name, r->size, r->perimeter);
        sum += r->perimeter * r->size;
    }

    printf("Result: %lld\n", sum);

    fclose(file);
}
