import os

basepath = os.getcwd()
Export("basepath")

colorgcc = basepath + "/contrib/colorgcc"
Export("colorgcc")

cflags = ["-O2"]
Export("cflags")

boostpath = "/usr/lib64"
Export("boostpath")

installdir = "/usr/local/GreatSPN"
Export("installdir")

SConscript('./algebra/SConstruct')
SConscript('./NSRC/SConstruct')
SConscript('./JavaGUI/SConstruct')

