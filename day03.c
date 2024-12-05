#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include <stdbool.h>
#include <assert.h>
#include <stdlib.h>



char* substring(const char* start, const char* end) {
	assert(start != NULL || end != NULL || start < end);

	size_t length = end - start;

	char* dest = malloc(length + 1);
	assert(dest != NULL);

	strncpy(dest, start, length);
	dest[length] = '\0';

	return dest;
}

const char ESCAPE = '\\';

struct Match {
	const char* start;
	const char* value;
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
	if (pattern == '(' || pattern == ')' || pattern == '[' || pattern == ']' || pattern == '+') {
		return input == pattern;
	}
	return false;
}

enum MatchResult match_inner(const char* pattern, const char* input, struct Match* matches, size_t matches_count, size_t im, char** end) {
	if (*pattern == '\0') {
		return YES_MATCH;
	}
	const char* pat = pattern;
	if (*pat == ESCAPE) {
		pat++;
		if (!test_escaped(*pat, *input)) {
			return NO_MATCH;
		}
		input++;
		*end = (char*)input;
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
			return NO_MATCH;
		}
		input++;
	} else if (*pat == '(') {
		struct Match* mat = &matches[im];
		if (mat->start != NULL) {
			return ERR_NESTED_GROUP;
		}
		mat->start = input;
	} else if (*pat == ')') {
		if (im >= matches_count) {
			return ERR_TOO_MANY_MATCHES;
		}
		struct Match* mat = &matches[im];
		if (mat->start == NULL) {
			return ERR_UNMATCHED_GROUP_CLOSE;
		}
		mat->value = substring(mat->start, input);
		im++;
	} else if (*pat == *input) {
		input++;
		*end = (char*)input;
	} else {
		return NO_MATCH;
	}

	if (*(pat + 1) == '+') {
		// First try to greedily match more
		enum MatchResult res = match_inner(pattern, input, matches, matches_count, im, end);
		if (res == NO_MATCH) {
			// If didn't work, stop greeding
			res  = match_inner(pat + 2, input, matches, matches_count, im, end);
		}
		return res;

	} else {
		return match_inner(pat + 1, input, matches, matches_count, im, end);
	}
}



enum MatchResult match(const char* pattern, char** input, struct Match* matches, size_t matches_count) {
	for (int i = 0; i < matches_count; i++) {
		matches[i].start = NULL;
		matches[i].value = NULL;
	}

	enum MatchResult res = NO_MATCH;
	while (**input != '\0' && res == NO_MATCH) {
		for (int i = 0; i < matches_count; i++) {
			matches[i].start = NULL;
			if (matches[i].value != NULL) {
				free((void*)matches[i].value);
			}
			matches[i].value = NULL;
		}
		char* end = *input;
		res = match_inner(pattern, *input, matches, matches_count, 0, &end);
		if (res == YES_MATCH) {
			*input = end;
			return YES_MATCH;
		}
		(*input)++;
	}
	return res;
}

void match_assert(struct Match* m1, const char** m2, size_t count) {
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
	char* input = "asd";
	my_assert(0, match("asd", &input, matches, matches_count), YES_MATCH);
	input = "asd";
	my_assert(1, match("bcd", &input, matches, matches_count), NO_MATCH);
	input = "123";
	my_assert(2, match("\\d\\d\\d", &input, matches, matches_count), YES_MATCH);
	input = "12";
	my_assert(3, match("\\dx", &input, matches, matches_count), NO_MATCH);

	input = "1 2";
	my_assert(4, match("(\\d) (\\d)", &input, matches, matches_count), YES_MATCH);
	match_assert(matches, (const char*[3]){ "1", "2", NULL },  3);

	input = "12   34\n";
	my_assert(5, match("(\\d+) +(\\d+)", &input, matches, matches_count), YES_MATCH);
	match_assert(matches, (const char*[3]){ "12", "34", NULL }, 3);

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
	match_assert(matches, (const char*[1]){ "()" }, 1);


	// input = "abc";
	// my_assert(9, match("mul()", &input, matches, matches_count), NO_MATCH);
}

FILE* open_file(const char *filename) {
	FILE *file = NULL;

	file = fopen(filename, "r");
	if (file == NULL) {
		fprintf(stderr, "Error opening file %s: %s\n", filename, strerror(errno));
		assert(false);
	}

	return file;
}


const size_t MAX_LINE_LENGTH = 50000;


int compare(const void* a, const void* b) {
   return (*(int*)a - *(int*)b);
}

bool parseint(char** in, int* out) {
	bool found = false;
	*out = 0;
	while (true) {
		if (!found && **in == ' ') {
			(*in)++;
			printf("'%s'\n", *in);
			continue;
		} 
		else if (**in >= '0' && (**in) <= '9') {
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

int sign(int n) {
	if (n > 0) return 1;
	if (n < 0) return -1;
	return 0;
}

int main() {
	FILE* file = open_file("input");

	char line[MAX_LINE_LENGTH];

	const size_t matches_count = 2;
	struct Match matches[matches_count];

	int result = 0;

	bool doit = true;
	while (fgets(line, sizeof(line), file) != NULL) {
		const char* do_positions[10000];
		int do_count = 0;
		char* cursor = line;
		while(match("(do\\(\\))", &cursor, matches, 1) == YES_MATCH) {
			do_positions[do_count] = matches[0].start;
			do_count++;
		}
		const char* dont_positions[10000];
		int dont_count = 0;
		cursor = line;
		while(match("(don't\\(\\))", &cursor, matches, 1) == YES_MATCH) {
			dont_positions[dont_count] = matches[0].start;
			dont_count++;
		}

		cursor = line;
		while(match("mul\\((\\d+),(\\d+)\\)", &cursor, matches, matches_count) == YES_MATCH) {
			char* do_pos = NULL;
			for (int i = do_count - 1; i >= 0; i--) {
				if (do_positions[i] < matches[0].start) {
					do_pos = (char*)do_positions[i];
					doit = true;
					break;
				}
			}
			for (int i = dont_count - 1; i >= 0; i--) {
				if (dont_positions[i] < do_pos) {
					break;
				}
				if (dont_positions[i] < matches[0].start) {
					doit = false;
					break;
				}
			}

			if (doit) {
				int n0 = atoi(matches[0].value);
				int n1 = atoi(matches[1].value);
				result += n0 * n1;
			}
		}
		printf("\n");
	}


	printf("%d", result);

	fclose(file);
}

