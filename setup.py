from setuptools import setup, Extension

ext_modules = [
    Extension('bigdir', sources=['bigdir.c']),
]

setup(
    name='bigdir',
    version='0.1.0',
    description='bigdir lets you read really big directories',
    ext_modules=ext_modules,
)
