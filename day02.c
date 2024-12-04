#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include <stdbool.h>
#include <assert.h>
#include <stdlib.h>


FILE* open_file(const char *filename) {
	FILE *file = NULL;

	file = fopen(filename, "r");
	if (file == NULL) {
		fprintf(stderr, "Error opening file %s: %s\n", filename, strerror(errno));
		assert(false);
	}

	return file;
}


const size_t MAX_LINE_LENGTH = 1024;


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

	int safe_count = 0;

	while (fgets(line, MAX_LINE_LENGTH, file) != NULL) {
		char* cursor = (char*)line;
		int num = 0;
		int nums[10000];
		int num_count = 0;
		while(parseint(&cursor, &num)) {
			nums[num_count] = num;
			num_count++;
		}

		for (int skip = -1; skip < num_count; skip++) {
			int diff_sign = 0;
			int last = 0;
			bool safe = true;
			for (int i = 0; i < num_count; i++) {
				if (i == skip) continue;
				if (last == 0) {
					last = nums[i];
					continue;
				}

				int diff = nums[i] - last;
				if (diff_sign == 0) {
					diff_sign = sign(diff);
				}

				if (diff_sign != sign(diff) || abs(diff) < 1 || abs(diff) > 3) {
					safe = false;
					break;
				}

				last = nums[i];
			}

			if (safe) {
				safe_count++;
				break;
			}
		}
	}


	printf("%d", safe_count);

	fclose(file);
}

