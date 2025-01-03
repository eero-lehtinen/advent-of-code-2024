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

int mod(int a, int b) {
    int r = a % b;
    return r < 0 ? r + b : r;
}

void print_map(struct Array *map, struct Pos robot, int width, int height) {
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            if (robot.x == x && robot.y == y) {
                printf("@");
            } else {
                printf("%c", map->data[y * width + x]);
            }
        }
        printf("\n");
    }
    printf("\n");
}

struct Pos add_pos(struct Pos a, struct Pos b) { return (struct Pos){a.x + b.x, a.y + b.y}; }
struct Pos sub_pos(struct Pos a, struct Pos b) { return (struct Pos){a.x - b.x, a.y - b.y}; }

const size_t MAX_LINE_LENGTH = 50000;

int main() {
    FILE *file = open_file("input");

    char line[MAX_LINE_LENGTH];

    struct Array map;
    array_new(&map, 8);
    int width = 0;
    int height = 0;

    struct Array moves;
    array_new(&moves, 8);

    struct Pos robot;
    bool map_part = true;

    while (fgets(line, sizeof(line), file) != NULL) {
        char *cursor = line;
        int w = 0;
        if (map_part) {
            if (*line == '\n') {
                map_part = false;
                continue;
            }
            while (*cursor != '\n' && *cursor != '\0') {
                if (*cursor == '@') {
                    robot.x = w;
                    robot.y = height;
                    array_push(&map, '.');
                } else {

                    array_push(&map, *cursor);
                }
                w++;
                cursor++;
            }
            if (height == 0) {
                width = w;
            }
            height++;
        } else {
            while (*cursor != '\n' && *cursor != '\0') {
                array_push(&moves, *cursor);
                cursor++;
            }
        }
    }

    // print_map(&map, robot, width, height);
    //
    // for (int i = 0; i < moves.size; i++) {
    //     printf("%c", moves.data[i]);
    // }
    // printf("\n");

    for (int i = 0; i < moves.size; i++) {
        // print_map(&map, robot, width, height);
        //
        // printf("%c\n", moves.data[i]);

        struct Pos dir;
        char move = moves.data[i];
        if (move == '^') {
            dir = (struct Pos){0, -1};
        } else if (move == '>') {
            dir = (struct Pos){1, 0};
        } else if (move == 'v') {
            dir = (struct Pos){0, 1};
        } else if (move == '<') {
            dir = (struct Pos){-1, 0};
        } else {
            assert(false);
        }

        struct Pos next_pos = add_pos(robot, dir);
        char next_tile = map.data[next_pos.y * width + next_pos.x];
        if (next_tile == '.') {
            robot = next_pos;
            continue;
        }
        int movables = 0;
        while (next_tile == 'O') {
            movables++;
            next_pos = add_pos(next_pos, dir);
            next_tile = map.data[next_pos.y * width + next_pos.x];
        }
        if (next_tile != '.') {
            continue;
        }

        next_pos = add_pos(robot, dir);
        robot = next_pos;
        next_tile = map.data[next_pos.y * width + next_pos.x];

        for (int j = 0; j < movables; j++) {
            next_pos = add_pos(next_pos, dir);
            char tmp = map.data[next_pos.y * width + next_pos.x];
            map.data[next_pos.y * width + next_pos.x] = next_tile;
            next_tile = tmp;
        }

        map.data[robot.y * width + robot.x] = '.';
    }

    int gps = 0;

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            char tile = map.data[y * width + x];
            if (tile == 'O') {
                gps += 100 * y + x;
            }
        }
    }

    // print_map(&map, robot, width, height);

    printf("Result: %d\n", gps);

    fclose(file);
}
