#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

static char *progname;
static char makefile[] = "TARGET  := solve.out\n"
"CC      := cc\n"
"CFLAGS  := -Wall -Werror -Wpedantic -Wextra -O2\n"
"OBJECTS := $(wildcard ../../includes/*.o)\n\n"
"TARGET  := solve.out\n"
"all: $(TARGET)\n\n"
"$(TARGET): ./main.c\n"
"	$(CC) $(OBJECTS) $(CFLAGS) -o $@ $^ -lm\n\n"
"clean:\n"
"	rm $(TARGET)\0";

static char cprog[] = "#include <stdio.h>\n\n"
"int main(void) {\n"
"    printf(\"Hello world!\\n\");\n"
"}\n\0";

/* a bit fragile but gets the job done */
int recursiveMkdir(char *path, int perms, int *depth) {
    int err = 0, max = PATH_MAX;
    char tmp[PATH_MAX], *tmpptr;
    int _depth;

    tmpptr = tmp;
    _depth = 0;

    for (char *ptr = path; *ptr != '\0'; ptr++, --max) {
        if (max == 0) {
            fprintf(stderr, "recursiveMkdir ERROR: Max path reached\n");
            exit(EXIT_FAILURE);
        }
        if (*ptr == '/') {
            *tmpptr = '\0';
            tmpptr = tmp;
            /* create dir */
            if ((err = mkdir(tmp, perms)) == -1) return err;
            /* enter newley created dir */
            if ((err = chdir(tmp)) == -1) return err;
            max = PATH_MAX;
            _depth++;
            continue;
        }
        *tmpptr++ = *ptr;
    }
    *tmpptr = '\0'; 
    _depth++;
    err = mkdir(tmp, perms);

    if (depth) *depth = _depth;

    return err;
}

void createAndWrite(char *filename, char *contents, unsigned long contentlen) {
    int fd;
    if ((fd = open(filename, O_RDWR | O_CREAT, 0666)) <= 0) {
        fprintf(stderr, "Failed to open(2) %s %s\n", filename, strerror(errno));
        exit(EXIT_FAILURE);
    }

    if (write(fd, contents, contentlen) <= 0) {
        fprintf(stderr, "Failed to write(2) %s %s\n", filename, strerror(errno));
        close(fd);
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char **argv) {
    progname = argv[0];
    char *dirname;

    dirname = NULL;

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <dirname>\n", progname);
        exit(EXIT_FAILURE);
    }

    dirname = argv[1];

    if (recursiveMkdir(dirname, 0700, NULL) == -1) {
        fprintf(stderr, "Failed to mkdir(2) %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    if (chdir(dirname) == -1) {
        fprintf(stderr, "Failed to chdir(2) %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    createAndWrite("main.c", cprog, strlen(cprog));
    createAndWrite("Makefile", makefile, strlen(makefile));

    exit(EXIT_SUCCESS);
}
