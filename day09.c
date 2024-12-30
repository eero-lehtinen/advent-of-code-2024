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
    int64_t *data;
};

void array_new(struct Array *arr, int capacity) {
    arr->capacity = capacity;
    arr->size = 0;
    arr->data = malloc(sizeof(int64_t) * capacity);
}

void array_push(struct Array *arr, int64_t value) {
    if (arr->size == arr->capacity) {
        arr->capacity *= 2;
        arr->data = realloc(arr->data, sizeof(int64_t) * arr->capacity);
    }
    arr->data[arr->size] = value;
    arr->size++;
}

void array_free(struct Array *arr) { free(arr->data); }

const size_t MAX_LINE_LENGTH = 50000;

int main() {
    FILE *file = open_file("input");

    char line[MAX_LINE_LENGTH];

    struct Array filesystem;
    array_new(&filesystem, 8);

    int id = 0;
    while (fgets(line, sizeof(line), file) != NULL) {
        char *cursor = line;
        bool file = true;
        while (*cursor != '\n' && *cursor != '\0') {
            int num = *cursor - '0';
            if (file) {
                for (int i = 0; i < num; i++) {
                    array_push(&filesystem, id);
                }
                id++;
            } else {
                for (int i = 0; i < num; i++) {
                    array_push(&filesystem, -1);
                }
            }
            cursor++;
            file = !file;
        }
    }

    // for (int i = 0; i < filesystem.size; i++) {
    //     if (filesystem.data[i] == -1) {
    //         printf(".");
    //     } else {
    //         printf("%lld", filesystem.data[i]);
    //     }
    // }
    // printf("\n");

    int ir = filesystem.size - 1;
    while (--id >= 0) {
        int size = 0;
        while (ir >= 0 && filesystem.data[ir] != id) {
            ir--;
        }
        while (ir >= 0 && filesystem.data[ir] == id) {
            ir--;
            size++;
        }

        int il = 0;
        int space = 0;
        while (il <= ir) {
            if (filesystem.data[il] == -1) {
                space++;
                if (space == size) {
                    break;
                }
            } else {
                space = 0;
            }
            il++;
        }
        if (space == size) {
            for (int i = 0; i < size; i++) {
                int il2 = il - size + i + 1;
                int ir2 = ir + i + 1;
                int tmp = filesystem.data[il2];
                filesystem.data[il2] = filesystem.data[ir2];
                filesystem.data[ir2] = tmp;
            }
        }
    }

    // for (int i = 0; i < filesystem.size; i++) {
    //     if (filesystem.data[i] == -1) {
    //         printf(".");
    //     } else {
    //         printf("%lld", filesystem.data[i]);
    //     }
    // }
    // printf("\n");

    uint64_t checksum = 0;

    for (uint64_t i = 0; i < filesystem.size; i++) {
        uint64_t data = filesystem.data[i];
        if (data != -1) {
            checksum += data * i;
        }
    }

    printf("Result: %llu\n", checksum);

    fclose(file);
}
