#!/usr/bin/env python

"""
setup.py file for SWIG jackaudio
"""

from distutils.core import setup, Extension
import os
os.system("swig -c++ -python jackcpp.i")

jackaudio_module = Extension('_jackaudio', 
        sources=['jackcpp_wrap.cxx'],
        libraries=['jackcpp', 'jack'],
        )

setup (name = 'jackaudio',
       version = '0.3',
       author      = "Alex Norman",
       description = """A simple wrapper for Jack Audio IO""",
       ext_modules = [jackaudio_module],
       py_modules = ["jackaudio"],
       )
