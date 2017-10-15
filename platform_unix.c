#include <dirent.h>
#include <errno.h>
#include <stdio.h>

struct bigdir_iterator {
    char bd_eof;
    char *bd_name;

    DIR* _dir;
};

int bigdir_iterator_open(struct bigdir_iterator* it, const char* path) {
    it->_dir = opendir(path);
    if(!it->_dir) {
        return -1;
    }
    return 0;
}

void bigdir_iterator_dealloc(struct bigdir_iterator* it) {
    if(!it->_dir) {
        return;
    }
    (void)closedir(it->_dir);
    it->_dir = 0;
}

int bigdir_iterator_next(struct bigdir_iterator* it) {
    errno = 0;
    struct dirent *ent = readdir(it->_dir);

    if(errno) {
        return 1;
    }

    if(!ent) {
        it->bd_eof = 1;
        bigdir_iterator_dealloc(it); 
        return 0;
    }

    it->bd_name = ent->d_name;
    return 0;
}
