import pytest
import bigdir
import errno


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
