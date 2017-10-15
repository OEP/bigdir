#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <fcntl.h>

#define BIGDIR_BUF_SIZE (5 * 1024 * 1024)
#define BIGDIR_NREAD_INIT -2
#define BIGDIR_NREAD_EOF -3

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
    it->_nread = BIGDIR_NREAD_INIT;
    return 0;
}

void bigdir_iterator_dealloc(struct bigdir_iterator* it) {
    (void)close(it->_fd);
}

int bigdir_iterator_next(struct bigdir_iterator* it) {
    if (it->_nread == BIGDIR_NREAD_INIT || (it->_nread > 0 && it->_pos >= it->_nread)) {
        it->_pos = 0;
        it->_nread = syscall(SYS_getdents, it->_fd, it->_buf, sizeof(it->_buf));

        /* This follows what glibc does in the case of error or in EOF
            * case, which is a POSIX.1 requirement */
        if (it->_nread <= 0) {
            it->bd_eof = 1;
            bigdir_iterator_dealloc(it);
            return 0;
        }
    }

    it->_ent = (struct linux_dirent *)(it->_buf + it->_pos);
    it->_pos += it->_ent->d_reclen;
    it->bd_name = it->_ent->d_name;
}
