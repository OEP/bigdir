import pytest
import bigdir
import errno


def test_scan_noarg():
    with pytest.raises(TypeError):
        bigdir.scan()


def test_scan_too_many_args():
    with pytest.raises(TypeError):
        bigdir.scan(None, None)


def test_scan_tmpdir(tmpdir):
    result = bigdir.scan(str(tmpdir))
    for item in result:
        assert item in ('.', '..')


def test_scan_error_open(tmpdir):
    f = tmpdir.join("test.txt")
    f.write("Hello.")

    # open() fails while initializing iterator
    with pytest.raises(IOError) as e:
        bigdir.scan(str(f))
    assert e.value.errno == errno.ENOTDIR


def test_scan_error_getdents(tmpdir):
    # getdents() call fails in the middle of iteration
    root = tmpdir.join('foo').mkdir()
    iterator = bigdir.scan(str(root))
    root.remove()
    with pytest.raises(IOError) as e:
        list(iterator)
    assert e.value.errno == errno.ENOENT
