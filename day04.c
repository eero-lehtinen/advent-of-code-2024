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

const size_t MAX_LINE_LENGTH = 50000;

struct Offset {
    int x;
    int y;
};

struct Offset offsets[4][3] = {{{-1, -1}, {0, 0}, {1, 1}},
                               {{1, 1}, {0, 0}, {-1, -1}},
                               {{1, -1}, {0, 0}, {-1, 1}},
                               {{-1, 1}, {0, 0}, {1, -1}}};

int main() {
    FILE *file = open_file("input");

    char line[MAX_LINE_LENGTH];

    char *chars = malloc(MAX_LINE_LENGTH * 1000);

    int width = 0;
    int height = 0;

    while (fgets(line, sizeof(line), file) != NULL) {
        int len = 0;
        while (line[len] != '\0' && line[len] != '\n') {
            len++;
        }
        if (width == 0) {
            width = len;
        }

        strncpy_s(chars + width * height, len + 1, line, len);
        height++;
    }
    chars[width * height] = '\0';

    // Print all chars
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            printf("%c", chars[y * width + x]);
        }
        printf("\n");
    }

    char *search_word = "MAS";
    int word_len = strlen(search_word);

    int result = 0;

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int found = 0;
            for (int i = 0; i < 4; i++) {
                for (int j = 0; j < word_len; j++) {
                    int x2 = x + offsets[i][j].x;
                    int y2 = y + offsets[i][j].y;
                    if (x2 < 0 || x2 >= width || y2 < 0 || y2 >= height) {
                        break;
                    }
                    if (chars[y2 * width + x2] != search_word[j]) {
                        break;
                    }
                    if (j == word_len - 1) {
                        found++;
                    }
                }
            }
            if (found == 2) {
                result++;
            }
        }
    }

    printf("%d", result);

    fclose(file);
}
