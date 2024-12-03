#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include <stdbool.h>
#include <assert.h>
#include <stdlib.h>

struct Match {
	const char* start;
	const char* value;
};


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

bool match_inner(const char* pattern, const char* input, struct Match* matches, size_t matches_count, size_t im) {
	if (*pattern == '\0') {
		return true;
	}
	const char* pat = pattern;
	if (*pat == ESCAPE) {
		++pat;
		if (*pat == 'd') {
			if (*input >= '0' && *input <= '9') {
				// matched
			} else {
				return false;
			}
		} else {
			return false;
		}
		++input;
	} else if (*pat == '(') {
		struct Match* mat = &matches[im];
		if (mat->start != NULL) {
			return false;
		}
		mat->start = input;
	} else if (*pat == ')') {
		if (im >= matches_count) {
			return false;
		}
		struct Match* mat = &matches[im];
		if (mat->start == NULL) {
			return false;
		}
		mat->value = substring(mat->start, input);
		im++;
	} else if (*pat == *input) {
		++input;
	} else {
		return false;
	}

	if (*(pat + 1) == '+') {
		// First try to greedily match more
		bool res = match_inner(pattern, input, matches, matches_count, im);
		if (!res) {
			// If didn't work, stop greeding
			res  = match_inner(pat + 2, input, matches, matches_count, im);
		}
		return res;

	} else {
		return match_inner(pat + 1, input, matches, matches_count, im);
	}
}



bool match(const char* pattern, const char* input, struct Match* matches, size_t matches_count) {
	for (int i = 0; i < matches_count; i++) {
		matches[i].start = NULL;
		matches[i].value = NULL;
	}

	return match_inner(pattern, input, matches, matches_count, 0);
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

void test() {
	const size_t matches_count = 3;
	struct Match matches[matches_count];

	assert(match("asd", "asd", matches, matches_count));
	assert(!match("bcd", "asd", matches, matches_count));
	assert(match("\\d\\d\\d", "123", matches, matches_count));
	assert(!match("\\dx", "12", matches, matches_count));
	
	assert(match("(\\d) (\\d)", "1 2", matches, matches_count));
	match_assert(matches, (const char*[3]){ "1", "2", NULL },  3);

	assert(match("(\\d+) +(\\d+)", "12	 34\n", matches, matches_count));
	match_assert(matches, (const char*[3]){ "12", "34", NULL }, 3);
}

FILE* open_file(const char *filename) {
	FILE *file = NULL;

	file = fopen(filename, "r");
	if (file == NULL) {
		fprintf(stderr, "Error opening file %s: %s\n", filename, strerror(errno));
		return NULL;
	}

	return file;
}


const size_t MAX_LINE_LENGTH = 1024;


int compare(const void* a, const void* b) {
   return (*(int*)a - *(int*)b);
}

int main() {
	FILE* file = open_file("input");
	assert(file);

	char line[MAX_LINE_LENGTH];
	const size_t matches_count = 2;
	struct Match matches[matches_count];

	const size_t nums_max = 10000;
	int nums1[nums_max];
	int nums2[nums_max];

	size_t nums_size = 0;
	while (fgets(line, sizeof(line), file) != NULL) {
		match("(\\d+) +(\\d+)", line, matches, matches_count);
		if (matches[0].value == NULL) {
			break;
		}
		nums1[nums_size] = atoi(matches[0].value);
		nums2[nums_size] = atoi(matches[1].value);
		nums_size++;
	}

	qsort(nums1, nums_size, sizeof(int), compare);
	qsort(nums2, nums_size, sizeof(int), compare);

	int result = 0;
	int similarity = 0;
	size_t i = 0;
	size_t j = 0;
	while (i < nums_size && j < nums_size) {
		if (nums1[i] == nums2[j]) {
			similarity++;
			j++;
		} else	{
			result += similarity * nums1[i];
			if (nums1[i] < nums2[j]) {
				int n = nums1[i];
				i++;
				while (i < nums_size && nums1[i] == n) {
					result += similarity * n;
					i++;
				}
			}
			while (nums1[i] > nums2[j]) {
				j++;
			}
			similarity = 0;
		}
	}

	printf("%d", result);

	fclose(file);
}

