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

When to use
-----------

Probably, you don't need to use bigdir, and should use
[scandir](https://github.com/benhoyt/scandir) instead.

bigdir is meant for the pathological case where a misbehaving program has
written millions of files to a flat directory and you're probably only
interested in cleaning up the mess. It turns out scandir does pretty well in
this case. Here is the timing I get on a million-file directory after dropping
the file system cache (`echo 3 > /proc/sys/vm/drop_caches`).

Benchmark Name | Time (sec)
-------------- | ----------
`listdir_all`  | 30.2
`listdir_1`    | 32.4
`bigdir_all`   | 27.4
`bigdir_1`     | 2.58
`bigdir_0`     | 0.0371
`scandir_all`  | 32.1
`scandir_1`    | 0.0472
`scandir_0`    | 0.00155

There is a small difference between reading the entire directory between bigdir
and scandir, but it is probably because scandir returns file attribute
information as well.

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
