from setuptools import setup, Extension

ext_modules = [
    Extension('_bigdir', sources=['_bigdir.c']),
]

setup(
    name='bigdir',
    version='0.1.0',
    description='bigdir lets you read really big directories',
    py_modules=['bigdir'],
    ext_modules=ext_modules,
)
