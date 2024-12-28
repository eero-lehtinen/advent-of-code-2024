#define __STDC_WANT_LIB_EXT1__ 1

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
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
    char *data;
};

void array_new(struct Array *arr, int capacity) {
    arr->capacity = capacity;
    arr->size = 0;
    arr->data = malloc(sizeof(int) * capacity);
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

void rot_neg_90(int *x, int *y) {
    // (0, -1) -> (1, 0) -> (0, 1) -> (-1, 0)
    int tmp = *x;
    *x = -*y;
    *y = tmp;
}

const size_t MAX_LINE_LENGTH = 50000;

struct Dir {
    int x;
    int y;
};

bool in_bounds(int width, int height, int x, int y) {
    return x >= 0 && x < width && y >= 0 && y < height;
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
            w++;
            array_push(&map, *cursor);
            cursor++;
        }
        if (height == 0) {
            width = w;
        }
        height++;
    }

    // for (int y = 0; y < height; y++) {
    //     for (int x = 0; x < width; x++) {
    //         printf("%c", map.data[y * width + x]);
    //     }
    //     printf("\n");
    // }
    // printf("\n");

    int start_x = -1;
    int start_y = -1;
    for (int yy = 0; yy < height; yy++) {
        for (int xx = 0; xx < width; xx++) {
            if (map.data[yy * width + xx] == '^') {
                start_x = xx;
                start_y = yy;
                break;
            }
        }
    }

    int result = 0;

    for (int yy = 0; yy < height; yy++) {
        for (int xx = 0; xx < width; xx++) {
            if (map.data[yy * width + xx] != '.') {
                continue;
            }
            map.data[yy * width + xx] = '#';

            int i = 0;
            int x = start_x;
            int y = start_y;
            int dx = 0;
            int dy = -1;
            while (in_bounds(width, height, x, y)) {
                int nx = x + dx;
                int ny = y + dy;
                while (in_bounds(width, height, nx, ny) &&
                       map.data[ny * width + nx] == '#') {
                    rot_neg_90(&dx, &dy);
                    nx = x + dx;
                    ny = y + dy;
                }
                x = nx;
                y = ny;

                if (++i == 50000) {
                    result++;
                    break;
                }
            }
            map.data[yy * width + xx] = '.';
        }
    }

    printf("%d", result);

    fclose(file);
}
