#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "harness.h"

extern const TestSuite opcode_suite;
extern const TestSuite cpu_suite;
extern const TestSuite keypad_suite;
extern const TestSuite display_suite;

static const TestSuite *const suites[] = {
    &opcode_suite,
    &cpu_suite,
    &keypad_suite,
    &display_suite,
};
static const size_t suite_count = sizeof suites / sizeof suites[0];

static bool contains_ci(const char *text, const char *needle) {
    size_t n = strlen(needle);

    for (const char *p = text; *p != '\0'; p++) {
        size_t i = 0;
        while (i < n && p[i] != '\0' && tolower((unsigned char)p[i]) == tolower((unsigned char)needle[i])) {
            i++;
        }
        if (i == n) {
            return true;
        }
    }

    return n == 0;
}

static bool selected(const TestSuite *suite, const TestCase *test, const char *filter) {
    if (filter == NULL) {
        return true;
    }

    return contains_ci(suite->area, filter) || contains_ci(test->name, filter);
}

int main(int argc, char *argv[]) {
    const char *filter = NULL;
    bool list = false;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--list") == 0) {
            list = true;
        } else {
            filter = argv[i];
        }
    }

    int run = 0;
    int failed = 0;

    for (size_t s = 0; s < suite_count; s++) {
        const TestSuite *suite = suites[s];
        bool header = false;

        for (const TestCase *test = suite->tests; test->name != NULL; test++) {
            if (!selected(suite, test, filter)) {
                continue;
            }

            if (list) {
                printf("%s/%s\n", suite->area, test->name);
                continue;
            }

            if (!header) {
                printf("%s\n", suite->area);
                header = true;
            }

            int before = test_failures;
            test->fn();
            run++;

            if (test_failures == before) {
                printf("  ok    %s\n", test->name);
            } else {
                printf("  FAIL  %s\n", test->name);
                failed++;
            }
        }
    }

    if (list) {
        return 0;
    }

    if (run == 0) {
        printf("no tests matched '%s'\n", filter);
        return 1;
    }

    printf("\n%d run, %d failed\n", run, failed);
    return failed ? 1 : 0;
}
