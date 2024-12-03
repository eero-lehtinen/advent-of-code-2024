#include <string.h>
#include <stddef.h>
#include <stdbool.h>
#include <assert.h>
#include <stdlib.h>

struct Match {
	int start;
	int end;
	char* value;
};

char* substring(const char* src, size_t start, size_t end) {
    if (src == NULL || start > end || start < 0) {
        return NULL;
    }
    size_t length = end - start + 1;
    char* dest = malloc(length + 1);
    if (dest == NULL) {
        return NULL;
    }
    strncpy(dest, src + start, length);
    dest[length] = '\0';

    return dest;
}

bool match(char* pattern, char* input, struct Match* matches, size_t matches_count) {
	for (int i = 0; i < matches_count; i++) {
		matches[i].start = -1;
		matches[i].end = -1;
	}

	size_t ip = 0;
	size_t ii = 0;
	size_t im = 0;

	const char ESCAPE = '\\';

	while (true) {
		struct Match* mat = &matches[im];
		char pat = pattern[ip];
		char inp = input[ii];
		if (pat == '\0') {
			break;
		}
		if (inp == '\0') {
			if (pat != '\0') {
				return false;
			}
			break;
		}
		if (pat == ESCAPE) {
			ip++;
			if (pat == 'd' && inp >= '0' && inp <= '9') {

			} else {
				return false;
			}
		}
		else if (pat == '(') {
			if (mat->start != -1 || mat->end != -1) {
				return false;
			}
			mat->start = ii;
			ip++;
		}
		else if (pat == ')') {
			if (mat->start == -1 || mat->end != -1) {
				return false;
			}
			mat->end = ii;
			ip++;
		}
		else if (pat != inp) {
			return false;
			ip++;
			ii++;
		}
		else {
			return false;
		}
	}

	return true;
}

int main() {
	const size_t MATCHES_COUNT = 0;
	struct Match matches[MATCHES_COUNT];

	assert(match("asd", "asd", matches, MATCHES_COUNT));
	assert(!match("bcd", "asd", matches, MATCHES_COUNT));
	assert(match("\\d\\d\\d", "123", matches, MATCHES_COUNT));
	assert(!match("\\dx", "12", matches, MATCHES_COUNT));

	assert(match("(\\d) (\\d)", "1 2", matches, MATCHES_COUNT));
	assert(matches[0].start == 0);
	assert(matches[0].end == 1);
	assert(matches[1].start == 2);
	assert(matches[1].end == 3);
	assert(matches[2].start == -1);
}
