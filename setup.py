from setuptools import setup, Extension

ext_modules = [
    Extension('bigdir', sources=['bigdir.c']),
]

setup(
    name='bigdir',
    version='0.1.0',
    description='Demo package!',
    ext_modules=ext_modules,
)
