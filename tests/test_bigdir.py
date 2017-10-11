import pytest
import bigdir


def test_scan(tmpdir):
    with pytest.raises(TypeError):
        result = bigdir.scan()
    with pytest.raises(TypeError):
        result = bigdir.scan(None, None)
    result = bigdir.scan(str(tmpdir))
    for item in result:
        print item
    assert False
