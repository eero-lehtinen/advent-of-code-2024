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

void pos_array_copy(struct PosArray *dst, struct PosArray *src) {
    dst->size = src->size;
    dst->capacity = src->capacity;
    dst->data = malloc(sizeof(struct Pos) * src->capacity);
    memcpy(dst->data, src->data, sizeof(struct Pos) * src->size);
}

void pos_array_free(struct PosArray *arr) { free(arr->data); }

int mod(int a, int b) {
    int r = a % b;
    return r < 0 ? r + b : r;
}

void print_map(struct Array *map, struct PosArray *boxes, struct Pos robot, int width, int height) {
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            if (robot.x == x && robot.y == y) {
                printf("@");
                continue;
            }
            bool box = false;
            for (int i = 0; i < boxes->size; i++) {
                if (boxes->data[i].x == x && boxes->data[i].y == y) {
                    printf("[");
                    box = true;
                    break;
                }
                if (boxes->data[i].x == x - 1 && boxes->data[i].y == y) {
                    printf("]");
                    box = true;
                    break;
                }
            }
            if (box) {
                continue;
            }

            printf("%c", map->data[y * width + x]);
        }
        printf("\n");
    }
    printf("\n");
}

struct Pos add_pos(struct Pos a, struct Pos b) { return (struct Pos){a.x + b.x, a.y + b.y}; }
struct Pos sub_pos(struct Pos a, struct Pos b) { return (struct Pos){a.x - b.x, a.y - b.y}; }

bool move_box(struct PosArray *boxes, int i, struct Pos dir, struct Array *map, int width,
              int height) {
    struct Pos cur_pos = boxes->data[i];
    struct Pos new_pos = add_pos(cur_pos, dir);
    if (map->data[new_pos.y * width + new_pos.x] == '#' ||
        map->data[new_pos.y * width + new_pos.x + 1] == '#') {
        return false;
    }
    bool can_move = true;
    for (int j = 0; j < boxes->size; j++) {
        if (i == j) {
            continue;
        }
        struct Pos other_box = boxes->data[j];
        if (abs(new_pos.x - other_box.x) <= 1 && new_pos.y == other_box.y) {
            if (!move_box(boxes, j, dir, map, width, height)) {
                can_move = false;
            }
        }
    }
    if (!can_move) {
        return false;
    }
    boxes->data[i] = new_pos;
    return true;
}

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

    struct PosArray boxes;
    pos_array_new(&boxes, 8);

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
                    array_push(&map, '.');
                } else if (*cursor == 'O') {
                    pos_array_push(&boxes, (struct Pos){w, height});
                    array_push(&map, '.');
                    array_push(&map, '.');
                } else {
                    array_push(&map, *cursor);
                    array_push(&map, *cursor);
                }
                w += 2;
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

    // print_map(&map, &boxes, robot, width, height);
    //
    // for (int i = 0; i < moves.size; i++) {
    //     printf("%c", moves.data[i]);
    // }
    // printf("\n");
    //

    for (int i = 0; i < moves.size; i++) {
        struct PosArray boxes_old;
        pos_array_copy(&boxes_old, &boxes);
        // print_map(&map, &boxes, robot, width, height);
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
            return 1;
        }

        struct Pos next_pos = add_pos(robot, dir);
        char next_tile = map.data[next_pos.y * width + next_pos.x];
        if (next_tile == '#') {
            continue;
        }

        bool moved = true;
        for (int j = 0; j < boxes.size; j++) {
            struct Pos box = boxes.data[j];
            if ((box.x == next_pos.x || box.x + 1 == next_pos.x) && box.y == next_pos.y) {
                moved = move_box(&boxes, j, dir, &map, width, height);
                break;
            }
        }

        if (moved) {
            robot = next_pos;
        } else {
            pos_array_copy(&boxes, &boxes_old);
        }
        pos_array_free(&boxes_old);
    }

    int gps = 0;
    for (int i = 0; i < boxes.size; i++) {
        gps += 100 * boxes.data[i].y + boxes.data[i].x;
    }

    // print_map(&map, &boxes, robot, width, height);

    printf("Result: %d\n", gps);

    fclose(file);
}
