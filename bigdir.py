"""Read very large directories easily

bigdir is a drop in replacement for os.listdir() which handles large
directories more gracefully. It differs from scandir() in that it does not
return file attribute information and can be fast for very large directories on
Linux systems which rely on glibc's readdir() implementation.

Synopsis
--------

Use bigdir.scan() in place of os.listdir() on directories which may be very
large:

    import bigdir
    for path in bigdir.scan('/tmp'):
        print(path)

Use bigdir.IMPLEMENTATION to determine which implementation of bigdir you are
using. It is either "linux" or "unix". The "linux" implementation avoids
calling readdir(), opting for the lower-level getdents() system call which is
will not buffer the entire directory into memory before returning the first
result. This usually means more responsive scripts when the number of files are
in the millions. The "unix" implementation does not provide any special
benefit, and is only present so you can write somewhat portable scripts with
bigdir:

    >>> import bigdir
    >>> bigdir.IMPLEMENTATION
    "linux"

"""

import _bigdir

scan = _bigdir.scan
IMPLEMENTATION = _bigdir.IMPLEMENTATION
