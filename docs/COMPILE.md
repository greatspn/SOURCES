## How to compile and install GreatSPN on your PC

This page describes how to compile the GreatSPN framework.
GreatSPN can be compiled and installed on most POSIX-compliant systems.
The framework requires several dependencies to be installed on your computer
in order to compile it. 

### Compile GreatSPN on Linux/Unix

In order to compile and install GreatSPN from sources, you need to do the following steps.
First, install the following dependencies (using the developer packages with the header files):
 * gcc and g++ (with support for C++14)
 * autotools (autoconf, automake, libtool, ...)
 * libgmp (GNU multiprecision library) (devel)
 * boost-c++ version 1.64+
 * flex (devel) and byacc
 * graphviz
 * libxml++ (devel), glibmm24 (devel) and glib2 (devel)
 * libcolamd2, on fedora platform provided by suitesparse (devel)
 * glpk (devel) and lpsolve (devel)
 * ant and Java 11+

Verify that java is properly installed following these two steps: (1) type `javac -version` and check that it is the right version; (2) type `echo ${JAVA_HOME}` and verify that the JAVA_HOME environment variable correctly points to the JDK directory. If the JAVA_HOME is not pointing to the JDK directory, or if it is not defined, it is important to define it.
 
For Fedora 32, you can install the dependencies with this command:

```
sudo dnf -y install gcc gcc-c++ gmp-devel gmp-c++ gmp boost-devel flex-devel \
 ant glib2-devel patch python glpk-devel lpsolve-devel autoconf automake \
 libtool zip flex git byacc time graphviz suitesparse-devel motif-devel make 
 libxml++-devel glibmm24-devel lpsolve ghostscript
```

For Debian/Ubuntu, the list of dependencies is installed with the command:
```
sudo apt-get install -y gcc g++ libgmp-dev libgmpxx4ldbl libboost-all-dev \
 flexc++ ant libglib2.0-dev patch python3 libglpk-dev liblpsolve55-dev \
 autoconf automake libtool zip flex byacc time graphviz libsuitesparse-dev \
 libmotif-dev make libxml++2.6-dev libglibmm-2.4-dev texlive-font-utils \
 openjdk-16-jdk cmake git ghostscript
```
This list was last tested on Ubuntu 20.04LTS, and it also applies to WSL-2 with Ubuntu.

Parts of GreatSPN use the [Meddly library](https://github.com/asminer/meddly). 
To download and install the Meddly library, type:

```
git clone  https://github.com/asminer/meddly.git meddly
cd meddly
./autogen.sh
./configure --prefix=/usr/local
make
sudo make install
```

Please, refer to the Meddly site for further information on how to download, compile and install it.


To compile the CTL* model checker, the [Spot library](https://spot.lrde.epita.fr/) is required. Download and install it from the Spot website. **NOTE**: ensure that it is installed in a system directory, like `/usr/local/lib`.
A sample installation procedure could be the following:

```
wget http://www.lrde.epita.fr/dload/spot/spot-2.9.6.tar.gz  # The URL could be different
tar xzf spot-2.9.6.tar.gz
cd spot-2.9.6
./configure --disable-python --disable-debug
make
sudo make install
```

Once Meddly and Spot are both compiled and installed, create the main `GreatSPN` source directory. 
Download, compile and install the sources with these commands:

```
mkdir ~/GreatSPN
cd ~/GreatSPN
git clone https://github.com/greatspn/SOURCES.git SOURCES
cd ~/GreatSPN/SOURCES
make
sudo make install
```
 
Upon completion, the framework will be installed by default in the `/usr/local/GreatSPN/` directory.

Verify that all installed library are in the search path of the dynamic linker, 
otherwise you will get an ld error when running the command line tools. 
If not, modify the `/etc/ld.so.conf` and/or the environmental variable `LD_LIBRARY_PATH` accordingly.

**Missing GraphMDP**: Only in the case you are interested in solution of MDP systems, you can find the optional dependency GraphMDP [here](http://www.di.unito.it/~greatspn/graphMDP-0.5.tar.gz). Note that this library is currently unmantained, and it could be hard to compile.

**Optional dependency: OGDF**
[OGDF](https://ogdf.uos.de/) is used for the automatic Petri net relayout feature of the graphical editor. 
It can be installed using these commands:
```
git clone https://github.com/ogdf/ogdf.git
cd ogdf
mkdir build && cd build && cmake .. && make
sudo make install
```
or, alternatively, follow the instructions directly from [the OGDF repository.](https://github.com/ogdf/ogdf)


### Compile GreatSPN on macOS

GreatSPN compiles and runs successfully on macOS. 
Before running the Linux/Unix installation steps, you will first need to install the Developer Tools (Xcode)
and Java OpenJDK (version 11 or greater).
Then install all the required dependencies using a software like [Homebrew](https://brew.sh/), e.g. 
by running the command:
```
brew install gmp boost flex byacc ant graphviz libxml++ \
 autoconf automake libtool suitesparse lp_solve ghostscript
```
In the last Homebrew version, the lp_solve package could require to be built from sources.
In that case, download the lp_solve sources from Sourceforge and follow the instructions
to compile the package.
After you have installed Xcode, brew, and the required dependecies, follow
the Linux/Unix instructions to compile Meddly and GreatSPN.



### Compile GreatSPN on Windows using WSL

GreatSPN is known to compile and run successfully using the *Windows Subsystem for Linux ver.2* (WSL-2, Windows 10 64bit minimum) using Ubuntu 20.04LTS.
WSL-2 installation is slightly different, since the GUI will be a standard Windows application, while the command line tools will reside and run inside the WSL-2 subsystem.

Follow the Linux steps to compile all the command line tools inside the WSL-2 subsystem.
If the WSL-2 component is newly installed, remember to update the apt-cache before installing the dependencies, 
i.e. run the command `sudo apt update`.
Then download a precompiled GUI from [here](http://www.di.unito.it/~amparore/mc4cslta/editor.html). 
You can use either the Windows version, or the JAR version (in case the Windows version raises problems).
The GUI for Windows is automatically configured to use the tools inside the WSL-2 environment, so no additional step is needed.


GreatSPN command line tools can also build and run using [Cygwin](https://www.cygwin.com/).



### Compile GreatSPN in a Docker container

A two-stage self-building Dockerfile, based on Fedora, is provided in the `docker` folder.






