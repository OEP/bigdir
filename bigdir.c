#include <Python.h>

/**
 * Platform-specific bigdir_iterator implementations. They are expected to
 * provide the following structure:
 *
 *     struct bigdir_iterator {
 *         char bd_eof;   // True if EOF is reached
 *         char *bd_name; // Relative path of current directory entry.
 *     };
 *
 * and the following functions for opening, closing, and advancing the
 * iterator:
 *
 *     int bigdir_iterator_open(struct bigdir_iterator*, const char*);
 *     void bigdir_iterator_dealloc(struct bigdir_iterator*);
 *     int bigdir_iterator_next(struct bigdir_iterator*);
 */
#ifdef __linux__
#include "platform_linux.c"
#elif defined(__APPLE__) || defined(unix) || defined(__unix__) || defined(__unix)
#include "platform_unix.c"
#else
#error Unsupported platform
#endif

const char* bigdir_doc =
    "Read very large directories easily\n"
    "\n"
    "bigdir is a drop in replacement for os.listdir() which handles large\n"
    "directories more gracefully. It differs from scandir() in that it does\n"
    "not return file attribute information and can be fast for very large\n"
    "directories on Linux systems which rely on glibc's readdir()\n"
    "implementation.\n"
    "\n"
    "Synopsis\n"
    "--------\n"
    "\n"
    "Use bigdir.scan() in place of os.listdir() on directories which may be\n"
    "very large:\n"
    "\n"
    "    import bigdir\n"
    "    for path in bigdir.scan('/tmp'):\n"
    "        print(path)\n"
    "\n"
    "Use bigdir.IMPLEMENTATION to determine which implementation of bigdir you\n"
    "are using. It is either \"linux\" or \"unix\". The \"linux\" implementation\n"
    "avoids calling readdir(), opting for the lower-level getdents() system call\n"
    "which is will not buffer the entire directory into memory before returning\n"
    "the first result. This usually means more responsive scripts when the\n"
    "number of files are in the millions. The \"unix\" implementation does not\n"
    "provide any special benefit, and is only present so you can write somewhat\n"
    "portable scripts with bigdir:\n"
    "\n"
    "    >>> import bigdir\n"
    "    >>> bigdir.IMPLEMENTATION\n"
    "    \"linux\"\n"
    "\n"
;


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
    sizeof(bigdir_PyIterator), /*tp_basicsize*/
    0,                         /*tp_itemsize*/
    bigdir_PyIterator_dealloc, /*tp_dealloc*/
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
    PyObject* module = Py_InitModule3("bigdir", bigdir_methods, bigdir_doc);
    (void)PyObject_SetAttr(module,
        PyString_FromString("IMPLEMENTATION"),
        PyString_FromString(BIGDIR_IMPLEMENTATION));
}
