#define __STDC_WANT_LIB_EXT1__ 1

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
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

bool parseint(char **in, int *out) {
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

const size_t MAX_LINE_LENGTH = 50000;

struct Rule {

    int a;
    int b;
};

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

int main() {
    FILE *file = open_file("input");

    char line[MAX_LINE_LENGTH];

    const size_t matches_count = 2;
    struct Match matches[matches_count];

    struct Rule rules[10000];
    int rules_count = 0;

    struct Array page_lists[10000];
    int page_lists_count = 0;

    bool first_part = true;

    while (fgets(line, sizeof(line), file) != NULL) {
        char *cursor = line;
        if (*line == '\n') {
            first_part = false;
            continue;
        }

        if (first_part) {
            if (match("(\\d+)|(\\d+)", &cursor, matches, matches_count) ==
                YES_MATCH) {
                assert(parseint(&matches[0].start, &rules[rules_count].a));
                assert(parseint(&matches[1].start, &rules[rules_count].b));
                rules_count++;
            }
        } else {
            struct Array *list = &page_lists[page_lists_count++];
            array_new(list, 8);
            while (match("(\\d+),?", &cursor, matches, matches_count) ==
                   YES_MATCH) {
                int res;
                assert(parseint(&matches[0].start, &res));
                array_push(list, res);
            }
        }
    }

    for (int i = 0; i < rules_count; i++) {
        printf("%d, %d\n", rules[i].a, rules[i].b);
    }

    for (int i = 0; i < page_lists_count; i++) {
        struct Array *list = &page_lists[i];
        printf("%d: ", i);
        for (int j = 0; j < list->size; j++) {
            printf("%d, ", list->data[j]);
        }
        printf("\n");
    }

    int result = 0;

    for (int i = 0; i < page_lists_count; i++) {
        struct Array *list = &page_lists[i];
        bool correct = true;
        for (int j = 0; j < rules_count; j++) {
            int a_pos = -1;
            int b_pos = -1;

            for (int k = 0; k < list->size; k++) {
                int cur = list->data[k];

                if (cur == rules[j].a) {
                    a_pos = k;
                }

                if (cur == rules[j].b) {
                    b_pos = k;
                }
            }
            if (a_pos != -1 && b_pos != -1 && a_pos >= b_pos) {
                correct = false;
                break;
            }
        }

        if (correct) {
            int middle = list->data[list->size / 2];
            result += middle;
        }

        array_free(list);
    }

    printf("%d", result);

    fclose(file);
}
