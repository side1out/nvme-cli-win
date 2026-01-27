#include <stdio.h>
#include <stdlib.h>
#include <io.h>
#include <fcntl.h>


ssize_t getline(char **lineptr, size_t *n, FILE *stream) {
    if (!lineptr || !n || !stream) return -1;
    if (!*lineptr) {
        *n = 128;
        *lineptr = malloc(*n);
        if (!*lineptr) return -1;
    }

    size_t pos = 0;
    int c;

    while ((c = fgetc(stream)) != EOF) {
        if (pos + 1 >= *n) {
            size_t new_size = *n * 2;
            char *new_ptr = realloc(*lineptr, new_size);
            if (!new_ptr) return -1;
            *lineptr = new_ptr;
            *n = new_size;
        }
        (*lineptr)[pos++] = (char)c;
        if (c == '\n') break;
    }

    if (pos == 0 && c == EOF)
        return -1;

    (*lineptr)[pos] = '\0';
    return (ssize_t)pos;
}

static int read_all(FILE *fp, char **out_buf, size_t *out_len) {
    if (!fp || !out_buf || !out_len) { errno = EINVAL; return -1; }

    /* Remember current position so we can restore it */
    long save = ftell(fp);
    if (save < 0) return -1;

    if (fflush(fp) != 0) return -1;

    if (fseek(fp, 0, SEEK_END) != 0) return -1;
    long end = ftell(fp);
    if (end < 0) return -1;
    if (fseek(fp, 0, SEEK_SET) != 0) return -1;

    size_t len = (size_t)end;
    char *buf = (char*)malloc(len + 1);
    if (!buf) { errno = ENOMEM; return -1; }

    size_t got = (len > 0) ? fread(buf, 1, len, fp) : 0;
    if (got != len) { free(buf); return -1; }

    buf[len] = '\0';
    *out_buf = buf;
    *out_len = len;

    /* Restore file position for continued writing */
    if (fseek(fp, save, SEEK_SET) != 0) { /* not fatal, but signal error */
        return -1;
    }
    return 0;
}

FILE* open_memstream(char **bufp, size_t *sizep) {
    if (!bufp || !sizep) { errno = EINVAL; return NULL; }
    *bufp = NULL;
    *sizep = 0;

    /* Use tmpfile() which gives us a binary R/W temp file that's deleted on close. */
    FILE *fp = tmpfile();
    if (!fp) {
        /* Fallback: explicit temp path */
        char tmpname[L_tmpnam];
        if (!tmpnam(tmpname)) return NULL;
        fp = fopen(tmpname, "w+b");
        /* The file will persist; caller still gets correct semantics. */
    }
    return fp;
}

int msync_memstream(FILE *fp, char **bufp, size_t *sizep) {
    /* Re-read entire file into a freshly allocated buffer. */
    return read_all(fp, bufp, sizep);
}

int close_memstream(FILE *fp, char **bufp, size_t *sizep) {
    char *buf = NULL;
    size_t len = 0;
    if (read_all(fp, &buf, &len) != 0) {
        /* best effort: still close */
        int saved = errno;
        fclose(fp);
        errno = saved;
        return -1;
    }
    int rc = fclose(fp);
    if (rc != 0) {
        int saved = errno;
        free(buf);
        errno = saved;
        return -1;
    }
    *bufp = buf;
    *sizep = len;
    return 0;
}