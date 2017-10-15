#include <Python.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <fcntl.h>

#define BIGDIR_BUF_SIZE 32768
#define BIGDIR_NREAD_INIT -2
#define BIGDIR_NREAD_EOF -3

struct linux_dirent {
    unsigned long d_ino;
    unsigned long d_off;
    unsigned short d_reclen;
    char d_name[];
};

typedef struct {
    PyObject_HEAD
    int fd;
    int pos;
    int nread;
    char buf[BIGDIR_BUF_SIZE];
    struct linux_dirent *ent;
} bigdir_Iterator;

static void
xclose(int* fd) {
    if(*fd == -1) {
        return;
    }
    if(close(*fd) == -1) {
        PyErr_SetFromErrno(PyExc_IOError);
    } else {
        *fd = -1;
    }
}

static PyObject*
bigdir_Iterator_iter(PyObject *self)
{
    Py_INCREF(self);
    return self;
}

void
bigdir_Iterator_dealloc(PyObject *self)
{
    bigdir_Iterator *p = (bigdir_Iterator*)self;
    xclose(&p->fd);
}

static PyObject*
bigdir_Iterator_next(PyObject *self)
{
    bigdir_Iterator *p = (bigdir_Iterator*)self;

    /* Check if we are in an EOF state */
    if (p->nread == BIGDIR_NREAD_EOF) {
        PyErr_SetNone(PyExc_StopIteration);
        return NULL;
    }

    do {
        /* Check if it is time to read some more */
        if (p->nread == BIGDIR_NREAD_INIT || (p->nread > 0 && p->pos >= p->nread)) {
            p->pos = 0;
            p->nread = syscall(SYS_getdents, p->fd, p->buf, sizeof(p->buf));

            /* This follows what glibc does in the case of error or in EOF
             * case, which is a POSIX.1 requirement */
            if (p->nread <= 0) {
                p->nread = BIGDIR_NREAD_EOF;
                PyErr_SetNone(PyExc_StopIteration);
                xclose(&p->fd);
                return NULL;
            }
        }

        p->ent = (struct linux_dirent *)(p->buf + p->pos);
        p->pos += p->ent->d_reclen;
    }
    while (
        !strncmp(p->ent->d_name, ".", 1) ||
        !strncmp(p->ent->d_name, "..", 2)
    );

    return PyString_FromString(p->ent->d_name);
}

static PyTypeObject bigdir_IteratorType = {
    PyObject_HEAD_INIT(NULL)
    0,                         /*ob_size*/
    "bigdir._Iterator",        /*tp_name*/
    sizeof(bigdir_Iterator),   /*tp_basicsize*/
    0,                         /*tp_itemsize*/
    bigdir_Iterator_dealloc,   /*tp_dealloc*/
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
    bigdir_Iterator *p;

    if (!PyArg_ParseTuple(args, "s", &path)) {
        return NULL;
    }

    p = PyObject_New(bigdir_Iterator, &bigdir_IteratorType);
    if (!p) return NULL;
    if (!PyObject_Init((PyObject *)p, &bigdir_IteratorType)) {
        Py_DECREF(p);
        return NULL;
    }
    p->fd = open(path, O_RDONLY | O_DIRECTORY);
    if(p->fd == -1) {
        PyErr_SetFromErrno(PyExc_IOError);
        return NULL;
    }
    p->nread = BIGDIR_NREAD_INIT;
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
