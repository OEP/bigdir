bigdir
======

A Python extension which lets you read really big directories.

Example
-------

```
import bigdir
for path in bigdir.scan('/tmp'):
    print(path)
```

Description
-----------

This module's primary target is Linux, but can be used on any Unix-like
platform. Particularly on Linux, glibc has issues in its `readdir()` call where
the entire directory is read into a buffer before the first result is returned.
On very large directories, this cause your program to hang, or worse exhaust
the system memory.

In Python, you might experience this in a call to `os.listdir()` or
`scandir.scan()`. Note this module can be a drop-in replacement for
`os.listdir()`, but not for `scandir.scan()` since it has a rich structure it
returns.

This extension is based on the idea from
[lowmem-tools](https://github.com/ScottDuckworth/lowmem-tools) which provides
alternative implementations of `rm` and `ls` Unix commands.

Support
-------

Platform | Notes
-------- | -----
Linux | Alternative `readdir()` implementation based on `getdents()`.
Unix | Tested on Mac.

The primary focus of this project is Linux support, other platforms are a
smaller priority since they do not seem to have the same problem.
Implementations for other platforms are mainly to provide the same interface
without aiming to increase the performance in any way.

Python 2 is currently what is tested, but Python 3 support is probably a couple
of lines of code away.

Testing
-------

You should be able to use [`tox`](https://tox.readthedocs.io/) to run the unit tests.

Benchmarking
------------

A benchmark script is provided in this repository to compare the performance of
`bigdir` against similar functionality like `os.listdir()` and
`scandir.scan()`. Note that the script generates a *really big* flat directory
which may be a nuisance depending on your system.

```
tox -e benchmark
```
