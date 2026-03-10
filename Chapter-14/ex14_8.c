#include <aio.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define BSZ 4096
#define NBUF 2

enum bufstate {
    B_FREE = 0,
    B_READING,
    B_FULL,
    B_WRITING
};

struct buffer {
    enum bufstate state;
    struct aiocb rdcb;
    struct aiocb wrcb;
    char data[BSZ];
    size_t len;          /* valid bytes in data[] */
    size_t written;      /* bytes already written */
    unsigned long seq;   /* input order */
};

static void rot13(char *p, size_t n)
{
    for (size_t i = 0; i < n; i++) {
        unsigned char c = (unsigned char)p[i];
        if (c >= 'a' && c <= 'z')
            p[i] = (char)('a' + (c - 'a' + 13) % 26);
        else if (c >= 'A' && c <= 'Z')
            p[i] = (char)('A' + (c - 'A' + 13) % 26);
    }
}

static void die(const char *msg)
{
    perror(msg);
    exit(1);
}

static int is_seekable(int fd)
{
    off_t r = lseek(fd, 0, SEEK_CUR);
    if (r >= 0)
        return 1;
    if (errno == ESPIPE)
        return 0;
    return 0;
}

static void start_read(struct buffer *b, int fd, int seekable, off_t off, unsigned long seq)
{
    memset(&b->rdcb, 0, sizeof(b->rdcb));
    b->rdcb.aio_fildes = fd;
    b->rdcb.aio_buf    = b->data;
    b->rdcb.aio_nbytes = BSZ;
    b->rdcb.aio_offset = seekable ? off : 0;
    b->state = B_READING;
    b->seq = seq;

    if (aio_read(&b->rdcb) < 0)
        die("aio_read");
}

static void start_write(struct buffer *b, int fd, int seekable, off_t off)
{
    memset(&b->wrcb, 0, sizeof(b->wrcb));
    b->wrcb.aio_fildes = fd;
    b->wrcb.aio_buf    = b->data + b->written;
    b->wrcb.aio_nbytes = b->len - b->written;
    b->wrcb.aio_offset = seekable ? off : 0;
    b->state = B_WRITING;

    if (aio_write(&b->wrcb) < 0)
        die("aio_write");
}

int main(void)
{
    struct buffer bufs[NBUF];
    memset(bufs, 0, sizeof(bufs));

    int in_seekable  = is_seekable(STDIN_FILENO);
    int out_seekable = is_seekable(STDOUT_FILENO);

    off_t in_off = 0;
    off_t out_off = 0;

    unsigned long next_read_seq = 0;
    unsigned long next_write_seq = 0;

    int eof_seen = 0;
    int read_inflight = 0;
    int write_inflight = 0;

    /* 先发起一个异步读 */
    start_read(&bufs[0], STDIN_FILENO, in_seekable, in_off, next_read_seq++);
    read_inflight = 1;

    for (;;) {
        const struct aiocb *list[2 * NBUF];
        int nlist = 0;

        for (int i = 0; i < NBUF; i++) {
            if (bufs[i].state == B_READING)
                list[nlist++] = &bufs[i].rdcb;
            else if (bufs[i].state == B_WRITING)
                list[nlist++] = &bufs[i].wrcb;
        }

        if (nlist == 0) {
            if (eof_seen)
                break;
            /* 理论上不该到这 */
            fprintf(stderr, "internal error: no outstanding AIO\n");
            exit(1);
        }

        if (aio_suspend(list, nlist, NULL) < 0) {
            if (errno == EINTR)
                continue;
            die("aio_suspend");
        }

        /* 处理已完成的读 */
        for (int i = 0; i < NBUF; i++) {
            struct buffer *b = &bufs[i];
            if (b->state != B_READING)
                continue;

            int err = aio_error(&b->rdcb);
            if (err == EINPROGRESS)
                continue;
            if (err != 0) {
                errno = err;
                die("aio_read completion");
            }

            ssize_t nr = aio_return(&b->rdcb);
            if (nr < 0)
                die("aio_return(read)");

            read_inflight = 0;

            if (nr == 0) {
                /* EOF */
                eof_seen = 1;
                b->state = B_FREE;
            } else {
                b->len = (size_t)nr;
                b->written = 0;
                rot13(b->data, b->len);
                if (in_seekable)
                    in_off += nr;
                b->state = B_FULL;
            }
        }

        /* 如果没有 EOF，且当前没有读在进行，找空缓冲继续读 */
        if (!eof_seen && !read_inflight) {
            for (int i = 0; i < NBUF; i++) {
                if (bufs[i].state == B_FREE) {
                    start_read(&bufs[i], STDIN_FILENO, in_seekable, in_off, next_read_seq++);
                    read_inflight = 1;
                    break;
                }
            }
        }

        /* 处理已完成的写 */
        for (int i = 0; i < NBUF; i++) {
            struct buffer *b = &bufs[i];
            if (b->state != B_WRITING)
                continue;

            int err = aio_error(&b->wrcb);
            if (err == EINPROGRESS)
                continue;
            if (err != 0) {
                errno = err;
                die("aio_write completion");
            }

            ssize_t nw = aio_return(&b->wrcb);
            if (nw < 0)
                die("aio_return(write)");

            write_inflight = 0;
            b->written += (size_t)nw;
            if (out_seekable)
                out_off += nw;

            if (b->written < b->len) {
                /* 短写：继续写剩余部分 */
                start_write(b, STDOUT_FILENO, out_seekable, out_off);
                write_inflight = 1;
            } else {
                b->state = B_FREE;
                next_write_seq++;
            }
        }

        /* 如果当前没有写在进行，则按顺序挑选最早的一块写出去 */
        if (!write_inflight) {
            int chosen = -1;
            for (int i = 0; i < NBUF; i++) {
                if (bufs[i].state == B_FULL && bufs[i].seq == next_write_seq) {
                    chosen = i;
                    break;
                }
            }
            if (chosen >= 0) {
                start_write(&bufs[chosen], STDOUT_FILENO, out_seekable, out_off);
                write_inflight = 1;
            }
        }
    }

    return 0;
}
