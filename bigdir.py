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
    for filename in bigdir.scan('/tmp'):
        print(filename)

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

import sys
import _bigdir

IMPLEMENTATION = _bigdir.IMPLEMENTATION


def scan(path):
    """Scan a directory for its contents.

    Returns an iterator which yields the filenames contained within the
    directory. You should use os.path.join() to construct absolute paths to the
    file. For example, this loop removes files in the directory:

    >>> import bigdir
    >>> for filename in bigdir.scan(path):
    ...     os.remove(os.path.join(path, filename))

    """
    # The PyArg_ParseTuple methods aren't very tolerant of unicode characters
    # in Python 2.7, so this is just a quick fix so those paths will work.
    if sys.version_info[0] < 3:
        path = path.encode('utf-8')
    return _bigdir.scan(path)
