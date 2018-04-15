import bigdir
import errno
import pytest
import six
import sys


USE_LINUX = sys.platform in ('linux2', 'linux')
USE_UNIX = not USE_LINUX


def test_scan_noarg():
    with pytest.raises(TypeError):
        bigdir.scan()


def test_scan_too_many_args():
    with pytest.raises(TypeError):
        bigdir.scan(None, None)


def test_scan_empty(tmpdir):
    result = bigdir.scan(str(tmpdir))
    assert list(result) == []


def test_single_file(tmpdir):
    f = tmpdir.join("test.txt")
    f.write("Hello.")

    result = bigdir.scan(str(tmpdir))
    assert list(result) == ["test.txt"]


def test_scan_error_open(tmpdir):
    f = tmpdir.join("test.txt")
    f.write("Hello.")

    # open() fails while initializing iterator
    with pytest.raises(IOError) as e:
        bigdir.scan(str(f))
    assert e.value.errno == errno.ENOTDIR


def test_scan_error_readdir(tmpdir):
    # readdir() file is removed
    root = tmpdir.join('foo').mkdir()
    iterator = bigdir.scan(str(root))
    root.remove()
    # NOTE POSIX.1 says that this case should be treated like an EOF
    # glibc implements this, and that's what we're following
    assert list(iterator) == []


@pytest.mark.skipif(not USE_UNIX, reason='Unix implementation only')
def test_implementation_unix():
    assert bigdir.IMPLEMENTATION == "unix"


@pytest.mark.skipif(not USE_LINUX, reason='Linux implementation only')
def test_implementation_linux():
    assert bigdir.IMPLEMENTATION == "linux"


def test_doc():
    assert bigdir.__doc__


def test_unicode1(tmpdir):
    # Technical note: path1 and path2 are both legal ways of representing the
    # same perceptual data. If you read the data off of HFS it makes this
    # conversion. Kind of strange...
    path1 = u'\u0108.txt'
    path2 = u'C\u0302.txt'
    tmpdir.join(path1).ensure()
    result = list(bigdir.scan(str(tmpdir)))
    assert len(result) == 1
    assert result[0] in (path1, path2)


def test_unicode2(tmpdir):
    root = tmpdir.join(u'\u0108')
    root.mkdir()
    root.join('foo.txt').ensure()
    arg = six.text_type(root)
    result = list(bigdir.scan(arg))
    assert result == ['foo.txt']
