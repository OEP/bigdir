import os
import functools
import time
import uuid

import click


try:
    range = xrange
except NameError:
    pass


try:
    import scandir
except ImportError:
    scandir = None


try:
    import bigdir
except ImportError:
    bigdir = None


DEFAULT_COUNT = 100


def generate_fixture(path, count):
    os.mkdir(path)
    it = range(count)
    with click.progressbar(it, length=count) as bar:
        for _ in bar:
            open(os.path.join(path, uuid.uuid1().hex), 'w').close()


all_benchmarks = []


class Benchmark(object):

    def __init__(self, fn, skip):
        self.fn = fn
        self.skip = skip

    @property
    def name(self):
        return self.fn.__name__


def benchmark(fn=None, skip=False):
    if fn is None:
        return functools.partial(benchmark, skip=skip)
    all_benchmarks.append(Benchmark(fn, skip))
    return fn


@benchmark
def listdir_all(path):
    for p in os.listdir(path):
        pass


@benchmark
def listdir_1(path):
    for p in os.listdir(path):
        break


@benchmark(skip=not bigdir)
def bigdir_all(path):
    for p in bigdir.scan(path):
        pass


@benchmark(skip=not bigdir)
def bigdir_1(path):
    for p in bigdir.scan(path):
        break


@benchmark(skip=not bigdir)
def bigdir_0(path):
    bigdir.scan(path)


@benchmark(skip=not scandir)
def scandir_all(path):
    for p in scandir.scandir(path):
        pass


@benchmark(skip=not scandir)
def scandir_1(path):
    for p in scandir.scandir(path):
        break


@benchmark(skip=not scandir)
def scandir_0(path):
    scandir.scandir(path)


@click.command()
@click.argument('path')
@click.option('--count', '-c', default=DEFAULT_COUNT, type=int)
def main(path, count):
    if os.path.exists(path):
        click.echo('Fixture exists -- skipping.')
    else:
        generate_fixture(path, count)

    for b in all_benchmarks:
        click.secho("{}... ".format(b.name), bold=True, nl=False)
        if b.skip:
            click.secho("skipped", bold=True)
        else:
            t0 = time.time()
            b.fn(path)
            t1 = time.time()
            click.secho("{} sec".format(t1 - t0), bold=True)


if __name__ == '__main__':
    main()
