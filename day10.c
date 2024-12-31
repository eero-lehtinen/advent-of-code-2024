#define __STDC_WANT_LIB_EXT1__ 1

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
    int *data;
};

void array_new(struct Array *arr, int capacity) {
    arr->capacity = capacity;
    arr->size = 0;
    arr->data = malloc(sizeof(int) * capacity);
}

void array_push(struct Array *arr, int value) {
    if (arr->size == arr->capacity) {
        arr->capacity *= 2;
        arr->data = realloc(arr->data, sizeof(int) * arr->capacity);
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

struct Pos pos_array_pop(struct PosArray *arr) {
    arr->size--;
    return arr->data[arr->size];
}

void pos_array_free(struct PosArray *arr) { free(arr->data); }

const struct Pos DIRECTIONS[4] = {
    {0, 1},
    {1, 0},
    {0, -1},
    {-1, 0},
};
const int DIRS_COUNT = sizeof(DIRECTIONS) / sizeof(struct Pos);

const size_t MAX_LINE_LENGTH = 50000;

int main() {
    FILE *file = open_file("input");

    char line[MAX_LINE_LENGTH];

    struct PosArray trailheads;
    pos_array_new(&trailheads, 8);

    struct Array map;
    array_new(&map, 8);
    int width = 0;
    int height = 0;

    while (fgets(line, sizeof(line), file) != NULL) {
        char *cursor = line;
        int w = 0;
        while (*cursor != '\n' && *cursor != '\0') {
            int value = *cursor - '0';
            array_push(&map, value);
            if (value == 0) {
                struct Pos pos = {w, height};
                pos_array_push(&trailheads, pos);
            }
            w++;
            cursor++;
        }
        if (height == 0) {
            width = w;
        }
        height++;
    }

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            printf("%d", map.data[y * width + x]);
        }
        printf("\n");
    }
    printf("\n");

    int trails = 0;

    for (int i = 0; i < trailheads.size; i++) {
        struct Pos start = trailheads.data[i];
        struct PosArray stack;
        pos_array_new(&stack, 8);
        pos_array_push(&stack, start);
        int cur_trails = 0;

        while (stack.size > 0) {
            struct Pos cur = pos_array_pop(&stack);
            int cur_val = map.data[cur.y * width + cur.x];
            for (int j = 0; j < DIRS_COUNT; j++) {
                struct Pos dir = DIRECTIONS[j];
                struct Pos next = {cur.x + dir.x, cur.y + dir.y};
                if (next.x < 0 || next.x >= width || next.y < 0 ||
                    next.y >= height) {
                    continue;
                }
                int next_val = map.data[next.y * width + next.x];
                if (next_val != cur_val + 1) {
                    continue;
                }

                if (next_val == 9) {
                    cur_trails++;
                    continue;
                }

                pos_array_push(&stack, next);
            }
        }

        trails += cur_trails;
        // printf("Trail: %d\n", cur_trails);

        pos_array_free(&stack);
    }

    printf("Result: %d\n", trails);

    fclose(file);
}
