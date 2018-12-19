import os

basepath = os.getcwd()
Export("basepath")

cflags = ["-O2"]
Export("cflags")

boostpath = "/usr/lib64"
Export("boostpath")

SConscript('./algebra/SConstruct')
SConscript('./NSRC/SConstruct')

