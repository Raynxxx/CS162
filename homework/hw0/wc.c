#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <ctype.h>

typedef unsigned long long wc_count_t;

static wc_count_t   tot_line_cnt, tot_word_cnt, tot_char_cnt;
static int          rval = 0;

static void count(char*);
static void print_counts(wc_count_t, wc_count_t, wc_count_t, char*);


int main(int argc, char *argv[]) {
    if (argc == 1) {
        count(NULL);
    } else {
        argv++;
        argc--;
        int dototal = (argc > 1);
        do {
            count(*argv);
        } while(*++argv);

        if (dototal) {
            print_counts(tot_line_cnt, tot_word_cnt, tot_char_cnt, "total");
        }
    }
    exit(rval);
}


static void count(char* file) {
    wc_count_t line_cnt, word_cnt, char_cnt;
    char* name;
    int fd, len = 0, has_space;
    char ch;
    
    line_cnt = word_cnt = char_cnt = 0;
    if (file) {
        if ((fd = open(file, O_RDONLY, 0)) < 0) {
            fprintf(stderr, "wc: %s: No such file or directory\n", file);
            rval = 1;
            return;
        }
        name = file;
    } else {
        fd = STDIN_FILENO;
        name = "<stdin>";
    }

    has_space = 1;
    while ((len = read(fd, &ch, 1)) == 1) {
        ++char_cnt;
        if (isspace(ch)) {
            has_space = 1;
            if (ch == '\n') {
                ++line_cnt;
            }
        } else {
            if (has_space) {
                has_space = 0;
                ++word_cnt;
            }
        }
    }

    print_counts(line_cnt, word_cnt, char_cnt, name);
    tot_line_cnt += line_cnt;
    tot_word_cnt += word_cnt;
    tot_char_cnt += char_cnt;

    if (close(fd)) {
        fprintf(stderr, "wc: error in file %s", name);
        rval = 1;
    }
}

static void print_counts(wc_count_t line_cnt, wc_count_t word_cnt, wc_count_t char_cnt, char* name) {
    printf("%7llu %7llu %7llu", line_cnt, word_cnt, char_cnt);
    if (name) {
        printf(" %s\n", name);
    } else {
        printf("\n");
    }
}
