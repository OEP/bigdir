/**
 * Linux bigdir iterator.
 *
 * glibc's implementation of readdir() on Linux can end up reading the entire
 * directory into memory. This can lead to a problem with very large
 * directories with millions of files in them. Instead of using readdir(), this
 * uses the underlying system call that glibc uses, getdents(), which does not
 * have this problem.
 */
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <fcntl.h>

#define BIGDIR_BUF_SIZE (5 * 1024 * 1024)

struct linux_dirent {
    unsigned long d_ino;
    unsigned long d_off;
    unsigned short d_reclen;
    char d_name[];
};

struct bigdir_iterator {
    char bd_eof;
    char *bd_name;

    int _fd;
    int _nread;
    int _pos;
    char _buf[BIGDIR_BUF_SIZE];
    struct linux_dirent *_ent;
};

int bigdir_iterator_open(struct bigdir_iterator* it, const char* path) {
    it->_fd = open(path, O_RDONLY | O_DIRECTORY);
    if(it->_fd == -1) {
        return -1;
    }
    /* Setting these magic values tricks next() into performing a read */
    it->_pos = 1;
    it->_nread = 1;
    return 0;
}

void bigdir_iterator_dealloc(struct bigdir_iterator* it) {
    if(it->_fd < 0) {
        return;
    }
    (void)close(it->_fd);
}

int bigdir_iterator_next(struct bigdir_iterator* it) {
    if (it->_nread > 0 && it->_pos >= it->_nread) {
        it->_pos = 0;
        it->_nread = syscall(SYS_getdents, it->_fd, it->_buf, sizeof(it->_buf));

        /* This follows what glibc does in the case of error or in EOF case,
         * which is a POSIX.1 requirement */
        if (it->_nread <= 0) {
            it->bd_eof = 1;
            bigdir_iterator_dealloc(it);
            return 0;
        }
    }

    it->_ent = (struct linux_dirent *)(it->_buf + it->_pos);
    it->_pos += it->_ent->d_reclen;
    it->bd_name = it->_ent->d_name;
    return 0;
}
