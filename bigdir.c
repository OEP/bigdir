#include <Python.h>

#ifdef __linux__
#include "platform_linux.c"
#elif defined(__APPLE__) || defined(unix) || defined(__unix__) || defined(__unix)
#include "platform_unix.c"
#else
#error Unsupported platform
#endif

typedef struct {
    PyObject_HEAD
    struct bigdir_iterator it;
} bigdir_PyIterator;

static PyObject*
bigdir_PyIterator_iter(PyObject *self)
{
    Py_INCREF(self);
    return self;
}

void
bigdir_PyIterator_dealloc(PyObject *self)
{
    bigdir_PyIterator *p = (bigdir_PyIterator*)self;
    bigdir_iterator_dealloc(&p->it);
}

static PyObject*
bigdir_PyIterator_next(PyObject *self)
{
    bigdir_PyIterator *p = (bigdir_PyIterator*)self;

    /* Check that we didn't start on en EOF */
    if (p->it.bd_eof) {
        PyErr_SetNone(PyExc_StopIteration);
        return NULL;
    }

    do {
        if(bigdir_iterator_next(&p->it)) {
            PyErr_SetFromErrno(PyExc_IOError);
            return NULL;
        }
    }
    while (
        !p->it.bd_eof &&
        (!strncmp(p->it.bd_name, ".", 1) ||
         !strncmp(p->it.bd_name, "..", 2))
    );

    /* Check that we didn't end on en EOF */
    if (p->it.bd_eof) {
        PyErr_SetNone(PyExc_StopIteration);
        return NULL;
    }

    return PyString_FromString(p->it.bd_name);
}

static PyTypeObject bigdir_PyIteratorType = {
    PyObject_HEAD_INIT(NULL)
    0,                         /*ob_size*/
    "bigdir._Iterator",        /*tp_name*/
    sizeof(bigdir_PyIterator),   /*tp_basicsize*/
    0,                         /*tp_itemsize*/
    bigdir_PyIterator_dealloc,   /*tp_dealloc*/
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
    bigdir_PyIterator_iter,  /* tp_iter: __iter__() method */
    bigdir_PyIterator_next  /* tp_iternext: next() method */
};

static PyObject *
bigdir_scan(PyObject *self, PyObject *args)
{
    const char *path;
    bigdir_PyIterator *p;

    if (!PyArg_ParseTuple(args, "s", &path)) {
        return NULL;
    }

    p = PyObject_New(bigdir_PyIterator, &bigdir_PyIteratorType);
    if (!p) return NULL;
    if (!PyObject_Init((PyObject *)p, &bigdir_PyIteratorType)) {
        Py_DECREF(p);
        return NULL;
    }
    p->it.bd_eof = 0;
    if(bigdir_iterator_open(&p->it, path)) {
        Py_DECREF(p);
        PyErr_SetFromErrno(PyExc_IOError);
        return NULL;
    }
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
