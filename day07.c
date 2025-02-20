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
    if (pattern == '(' || pattern == ')' || pattern == '[' || pattern == ']' ||
        pattern == '+' || pattern == '?') {
        return input == pattern;
    }
    return false;
}

enum MatchResult match_inner(const char *pattern, char *input,
                             struct Match *matches, size_t matches_count,
                             size_t im, char **end) {
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
        enum MatchResult res =
            match_inner(pattern, input, matches, matches_count, im, end);
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
    my_assert(4, match("(\\d) (\\d)", &input, matches, matches_count),
              YES_MATCH);
    match_assert(matches, (const char *[3]){"1", "2", NULL}, 3);

    input = "12   34\n";
    my_assert(5, match("(\\d+) +(\\d+)", &input, matches, matches_count),
              YES_MATCH);
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
}

bool parseint(char **in, long long *out) {
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

void array_new(struct Array *arr, int64_t capacity) {
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

enum Op {
    OP_SUM,
    OP_MUL,
    OP_CONCAT,
    OP_COUNT,
};

int count_digits(int64_t value) {
    int count = 0;
    while (value > 0) {
        value /= 10;
        count++;
    }
    return count;
}

int64_t operate(int64_t a, int64_t b, enum Op op) {
    switch (op) {
    case OP_SUM:
        return a + b;
    case OP_MUL:
        return a * b;
    case OP_CONCAT:
        return (int64_t)pow(10, count_digits(b)) * a + b;
    default:
        assert(false);
    }

    return 0;
}

const size_t MAX_LINE_LENGTH = 50000;

int main() {
    FILE *file = open_file("input");

    char line[MAX_LINE_LENGTH];

    struct Match matches[1] = {};

    struct Array map;
    array_new(&map, 8);

    int64_t result = 0;

    while (fgets(line, sizeof(line), file) != NULL) {
        char *cursor = line;

        assert(match("(\\d+):", &cursor, matches,
                     sizeof(matches) / sizeof(matches[0])) == YES_MATCH);

        int64_t value;
        assert(parseint(&matches[0].value, &value));

        struct Array numbers;
        array_new(&numbers, 10);

        int64_t out;
        while (parseint(&cursor, &out)) {
            array_push(&numbers, out);
        }

        struct Array ops;
        array_new(&ops, numbers.size - 1);
        for (int i = 0; i < numbers.size - 1; i++) {
            array_push(&ops, OP_SUM);
        }

        bool stop = false;
        while (!stop) {
            int64_t res = numbers.data[0];
            for (int i = 0; i < ops.size; i++) {
                res = operate(res, numbers.data[i + 1], ops.data[i]);
            }
            if (res == value) {
                result += res;
                break;
            }
            for (int i = 0; i < ops.size; i++) {
                ops.data[i]++;
                if (ops.data[i] == OP_COUNT) {
                    ops.data[i] = 0;
                    if (i == ops.size - 1) {
                        stop = true;
                    }
                } else {
                    break;
                }
            }
        }
    }

    printf("Result: %lld\n", result);

    fclose(file);
}
