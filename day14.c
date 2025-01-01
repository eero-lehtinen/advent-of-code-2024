#define __STDC_WANT_LIB_EXT1__ 1

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

const char ESCAPE = '\\';

struct Match {
    char *start;
    char *value;
};

enum MatchResult {
    YES_MATCH,
    NO_MATCH,
    ERR_NESTED_GROUP,
    ERR_UNMATCHED_GROUP_CLOSE,
    ERR_TOO_MANY_MATCHES,
    ERR_INCORRECT_CHAR_CLASS,
};

bool test_escaped(char pattern, char input) {
    if (pattern == 'd') {
        return input >= '0' && input <= '9';
    }
    if (pattern == 'w') {
        return (input >= 'a' && input <= 'z') || (input >= 'A' && input <= 'Z') || input == '_';
    }
    if (pattern == '(' || pattern == ')' || pattern == '[' || pattern == ']' || pattern == '+' ||
        pattern == '?') {
        return input == pattern;
    }
    return false;
}

enum MatchResult match_inner(const char *pattern, char *input, struct Match *matches,
                             size_t matches_count, size_t im, char **end) {
    if (*pattern == '\0') {
        return YES_MATCH;
    }
    bool match = true;
    const char *pat = pattern;
    if (*pat == ESCAPE) {
        pat++;
        if (!test_escaped(*pat, *input)) {
            match = false;
        }
        input++;
        *end = (char *)input;
    } else if (*pat == '[') {
        pat++;
        if (*pat != '^') {
            return ERR_INCORRECT_CHAR_CLASS;
        }
        bool matched = false;
        pat++;
        while (*pat != ']') {
            if (*pat != *input) {
                matched = true;
            }
            pat++;
            if (*pat == '\0') {
                return ERR_INCORRECT_CHAR_CLASS;
            }
        }
        if (!matched) {
            match = false;
        }
        input++;
    } else if (*pat == '(') {
        struct Match *mat = &matches[im];
        if (mat->start != NULL) {
            return ERR_NESTED_GROUP;
        }
        mat->start = input;
    } else if (*pat == ')') {
        if (im >= matches_count) {
            return ERR_TOO_MANY_MATCHES;
        }
        struct Match *mat = &matches[im];
        if (mat->start == NULL) {
            return ERR_UNMATCHED_GROUP_CLOSE;
        }
        mat->value = substring(mat->start, input);
        im++;
    } else if (*pat == *input) {
        input++;
        *end = (char *)input;
    } else {
        match = false;
    }

    if (*(pat + 1) == '?') {
        return match_inner(pat + 2, input, matches, matches_count, im, end);
    }

    if (!match) {
        return NO_MATCH;
    }

    if (*(pat + 1) == '+') {
        // First try to greedily match more
        enum MatchResult res = match_inner(pattern, input, matches, matches_count, im, end);
        if (res == NO_MATCH) {
            // If didn't work, stop greeding
            res = match_inner(pat + 2, input, matches, matches_count, im, end);
        }
        return res;

    } else {
        return match_inner(pat + 1, input, matches, matches_count, im, end);
    }
}

enum MatchResult match(const char *pattern, char **input, struct Match *matches,
                       size_t matches_count) {
    for (int i = 0; i < matches_count; i++) {
        matches[i].start = NULL;
        matches[i].value = NULL;
    }

    enum MatchResult res = NO_MATCH;
    while (**input != '\0' && res == NO_MATCH) {
        for (int i = 0; i < matches_count; i++) {
            matches[i].start = NULL;
            if (matches[i].value != NULL) {
                free((void *)matches[i].value);
            }
            matches[i].value = NULL;
        }
        char *end = *input;
        res = match_inner(pattern, *input, matches, matches_count, 0, &end);
        if (res == YES_MATCH) {
            *input = end;
            return YES_MATCH;
        }
        (*input)++;
    }
    return res;
}

void match_assert(struct Match *m1, const char **m2, size_t count) {
    for (size_t i = 0; i < count; i++) {
        if (m1[i].value == NULL || m2[i] == NULL) {
            assert(m1[i].value == NULL);
        } else {
            assert(strcmp(m1[i].value, m2[i]) == 0);
        }
    }
}

void my_assert(int n, enum MatchResult res, enum MatchResult expected) {
    if (res != expected) {
        printf("%d, Expected: %d, got: %d\n", n, expected, res);
    }
}

void test() {
    const size_t matches_count = 3;
    struct Match matches[matches_count];
    char *input = "asd";
    my_assert(0, match("asd", &input, matches, matches_count), YES_MATCH);
    input = "asd";
    my_assert(1, match("bcd", &input, matches, matches_count), NO_MATCH);
    input = "123";
    my_assert(2, match("\\d\\d\\d", &input, matches, matches_count), YES_MATCH);
    input = "12";
    my_assert(3, match("\\dx", &input, matches, matches_count), NO_MATCH);

    input = "1 2";
    my_assert(4, match("(\\d) (\\d)", &input, matches, matches_count), YES_MATCH);
    match_assert(matches, (const char *[3]){"1", "2", NULL}, 3);

    input = "12   34\n";
    my_assert(5, match("(\\d+) +(\\d+)", &input, matches, matches_count), YES_MATCH);
    match_assert(matches, (const char *[3]){"12", "34", NULL}, 3);

    input = "axxx";
    my_assert(6, match("a", &input, matches, matches_count), YES_MATCH);
    assert(strcmp(input, "xxx") == 0);

    input = "abc";
    my_assert(7, match("[^b]bc", &input, matches, matches_count), YES_MATCH);
    input = "abc";
    my_assert(8, match("[^a]bc", &input, matches, matches_count), NO_MATCH);

    input = "xxxasdasd";
    my_assert(9, match("asd", &input, matches, matches_count), YES_MATCH);
    assert(strcmp(input, "asd") == 0);

    input = "()";
    my_assert(10, match("(\\(\\))", &input, matches, matches_count), YES_MATCH);
    match_assert(matches, (const char *[1]){"()"}, 1);

    input = "12";
    my_assert(10, match("(123?)", &input, matches, matches_count), YES_MATCH);
    match_assert(matches, (const char *[1]){"12"}, 1);

    input = "123";
    my_assert(10, match("(123?)", &input, matches, matches_count), YES_MATCH);
    match_assert(matches, (const char *[1]){"123"}, 1);

    input = "aa-123,456";
    my_assert(11, match("aa(-?\\d+),(-?\\d+)", &input, matches, matches_count), YES_MATCH);
    match_assert(matches, (const char *[2]){"-123", "456"}, 2);
}

bool parseint(char **in, int *out) {
    bool found = false;
    bool negative = false;
    *out = 0;
    while (true) {
        if (!found && **in == ' ') {
            (*in)++;
        } else if (!found && !negative && **in == '-') {
            negative = true;
            (*in)++;
        } else if (**in >= '0' && (**in) <= '9') {
            found = true;
            *out *= 10;
            *out += **in - '0';
            (*in)++;
        } else {
            if (negative) {
                *out *= -1;
            }
            return found;
        }
    }
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

void print_positions(int width, int height, struct PosArray *positions) {
    printf("\n");
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int count = 0;
            for (int i = 0; i < positions->size; i++) {
                if (positions->data[i].x == x && positions->data[i].y == y) {
                    count++;
                }
            }

            if (count == 0) {
                printf(".");
            } else {
                printf("%d", count);
            }
        }
        printf("\n");
    }
}

const size_t MAX_LINE_LENGTH = 50000;

int main() {
    FILE *file = open_file("input");
    int width = 101;
    int height = 103;

    char line[MAX_LINE_LENGTH];

    struct Match matches[4];
    const int match_count = sizeof(matches) / sizeof(matches[0]);

    struct PosArray positions;
    pos_array_new(&positions, 8);
    struct PosArray velocities;
    pos_array_new(&velocities, 8);

    while (fgets(line, sizeof(line), file) != NULL) {
        char *cursor = line;
        assert(match("p=(\\d+),(\\d+) v=(-?\\d+),(-?\\d+)", &cursor, matches, match_count) ==
               YES_MATCH);
        struct Pos pos;
        assert(parseint(&matches[0].value, &pos.x));
        assert(parseint(&matches[1].value, &pos.y));
        pos_array_push(&positions, pos);
        struct Pos vel;
        assert(parseint(&matches[2].value, &vel.x));
        assert(parseint(&matches[3].value, &vel.y));
        pos_array_push(&velocities, vel);
    }

    for (int i = 0; i < 100; i++) {
        for (int j = 0; j < positions.size; j++) {
            struct Pos *pos = &positions.data[j];
            struct Pos *vel = &velocities.data[j];

            pos->x += vel->x;
            pos->y += vel->y;

            pos->x = mod(pos->x, width);
            pos->y = mod(pos->y, height);
        }
    }

    // print_positions(width, height, &positions);

    int quadrants[4] = {0, 0, 0, 0};
    int hw = width / 2;
    int hh = height / 2;
    for (int i = 0; i < positions.size; i++) {
        int q = -1;
        struct Pos pos = positions.data[i];
        if (pos.x < hw && pos.y < hh) {
            q = 0;
        } else if (pos.x > hw && pos.y < hh) {
            q = 1;
        } else if (pos.x < hw && pos.y > hh) {
            q = 2;
        } else if (pos.x > hw && pos.y > hh) {
            q = 3;
        }
        if (q != -1) {
            quadrants[q]++;
        }
    }

    int safety_score = quadrants[0];
    for (int i = 1; i < sizeof(quadrants) / sizeof(quadrants[0]); i++) {
        safety_score *= quadrants[i];
    }

    printf("Result: %d\n", safety_score);

    fclose(file);
}
