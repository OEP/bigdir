#include <sys/types.h>
#include <dirent.h>
#include <Python.h>

typedef struct {
    PyObject_HEAD
    int count;
    int current;
    struct dirent **namelist;
} bigdir_Iterator;

PyObject* bigdir_Iterator_iter(PyObject *self)
{
    Py_INCREF(self);
    return self;
}

PyObject* bigdir_Iterator_next(PyObject *self)
{
    bigdir_Iterator *p = (bigdir_Iterator*)self;
    if (p->current < p->count) {
        return PyString_FromString(p->namelist[p->current++]->d_name);
    } else {
	PyErr_SetNone(PyExc_StopIteration);
	return NULL;
    }
}

static PyTypeObject bigdir_IteratorType = {
    PyObject_HEAD_INIT(NULL)
    0,                         /*ob_size*/
    "bigdir._Iterator",        /*tp_name*/
    sizeof(bigdir_Iterator),   /*tp_basicsize*/
    0,                         /*tp_itemsize*/
    0,                         /*tp_dealloc*/
    0,                         /*tp_print*/
    0,                         /*tp_getattr*/
    0,                         /*tp_setattr*/
    0,                         /*tp_compare*/
    0,                         /*tp_repr*/
    0,                         /*tp_as_number*/
    0,                         /*tp_as_sequence*/
    0,                         /*tp_as_mapping*/
    0,                         /*tp_hash */
    0,                         /*tp_call*/
    0,                         /*tp_str*/
    0,                         /*tp_getattro*/
    0,                         /*tp_setattro*/
    0,                         /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_HAVE_ITER,
      /* tp_flags: Py_TPFLAGS_HAVE_ITER tells python to
         use tp_iter and tp_iternext fields. */
    "Internal iterator object",           /* tp_doc */
    0,  /* tp_traverse */
    0,  /* tp_clear */
    0,  /* tp_richcompare */
    0,  /* tp_weaklistoffset */
    bigdir_Iterator_iter,  /* tp_iter: __iter__() method */
    bigdir_Iterator_next  /* tp_iternext: next() method */
};

static PyObject *
bigdir_scan(PyObject *self, PyObject *args)
{
    const char *path;
    int count;
    bigdir_Iterator *p;

    if (!PyArg_ParseTuple(args, "s", &path)) {
        return NULL;
    }

    struct dirent **namelist;
    count = scandir(path, &namelist, NULL, NULL);

    p = PyObject_New(bigdir_Iterator, &bigdir_IteratorType);
    if (!p) return NULL;
    if (!PyObject_Init((PyObject *)p, &bigdir_IteratorType)) {
	Py_DECREF(p);
	return NULL;
    }
    p->current = 0;
    p->count = count;
    p->namelist = namelist;

    return (PyObject*)p;
}

static PyMethodDef bigdir_methods[] = {
    {"scan", bigdir_scan, METH_VARARGS, "Scan a directory for files"},
    {NULL, NULL, 0, NULL},
};

PyMODINIT_FUNC
initbigdir(void)
{
    (void) Py_InitModule("bigdir", bigdir_methods);
}
