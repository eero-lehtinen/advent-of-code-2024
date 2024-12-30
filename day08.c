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
    char *data;
};

void array_new(struct Array *arr, int capacity) {
    arr->capacity = capacity;
    arr->size = 0;
    arr->data = malloc(sizeof(char) * capacity);
}

void array_push(struct Array *arr, char value) {
    if (arr->size == arr->capacity) {
        arr->capacity *= 2;
        arr->data = realloc(arr->data, sizeof(char) * arr->capacity);
    }
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

void pos_array_free(struct PosArray *arr) { free(arr->data); }

const size_t MAX_LINE_LENGTH = 50000;

bool in_bounds(int width, int height, struct Pos pos) {
    return pos.x >= 0 && pos.x < width && pos.y >= 0 && pos.y < height;
}

struct FindAntinodesContext {
    struct hashmap_s *antinodes;
    int width;
    int height;
};

static int find_antinodes(void *const context, void *const value) {
    struct FindAntinodesContext *ctx = (struct FindAntinodesContext *)context;
    struct PosArray *v = (struct PosArray *)value;
    // printf("v: %d\n", v->size);
    // printf("width: %d, height: %d\n", ctx->width, ctx->height);
    // for (int i = 0; i < v->size; i++) {
    //     printf("v[%d]: %d, %d\n", i, v->data[i].x, v->data[i].y);
    // }

    for (int i = 0; i < v->size; i++) {
        struct Pos a = v->data[i];
        for (int j = 0; j < v->size; j++) {
            if (i == j) {
                continue;
            }
            struct Pos b = v->data[j];
            struct Pos offset = {b.x - a.x, b.y - a.y};

            // printf("a: %d, %d\n", a.x, a.y);
            // printf("b: %d, %d\n", b.x, b.y);
            // printf("offset: %d, %d\n", offset.x, offset.y);

            struct Pos *antinode_pos = malloc(sizeof(struct Pos));
            antinode_pos->x = a.x - offset.x;
            antinode_pos->y = a.y - offset.y;

            if (in_bounds(ctx->width, ctx->height, *antinode_pos)) {
                bool *tru = malloc(sizeof(bool));
                *tru = true;
                assert(hashmap_put(ctx->antinodes, antinode_pos,
                                   sizeof(struct Pos), tru) == 0);
            }
        }
    }
    return 1;
}

int main() {
    FILE *file = open_file("input");

    char line[MAX_LINE_LENGTH];

    struct hashmap_s nodes_by_name;
    assert(hashmap_create(10, &nodes_by_name) == 0);

    struct hashmap_s antinodes;
    assert(hashmap_create(10, &antinodes) == 0);

    struct Array map;
    array_new(&map, 8);
    int width = 0;
    int height = 0;

    while (fgets(line, sizeof(line), file) != NULL) {
        char *cursor = line;
        int w = 0;
        while (*cursor != '\n' && *cursor != '\0') {
            array_push(&map, *cursor);
            if (*cursor != '.') {
                char *key = substring(cursor, cursor + 1);
                struct PosArray *n = hashmap_get(&nodes_by_name, key, 1);
                if (n == NULL) {
                    struct PosArray *nodes = malloc(sizeof(struct PosArray));
                    pos_array_new(nodes, 2);
                    struct Pos pos = {w, height};
                    pos_array_push(nodes, pos);
                    assert(hashmap_put(&nodes_by_name, key, 1, nodes) == 0);
                } else {
                    struct Pos pos = {w, height};
                    pos_array_push(n, pos);
                }
            }
            w++;
            cursor++;
        }
        if (height == 0) {
            width = w;
        }
        height++;
    }

    struct FindAntinodesContext context = {
        .antinodes = &antinodes,
        .width = width,
        .height = height,
    };
    assert(hashmap_iterate(&nodes_by_name, find_antinodes, &context) == 0);

    printf("Result %u\n", hashmap_num_entries(&antinodes));

    fclose(file);
}
