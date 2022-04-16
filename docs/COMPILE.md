# How to compile and install GreatSPN on your PC

This page describes how to compile the GreatSPN framework.
GreatSPN can be compiled and installed on most POSIX-compliant systems.
The framework requires several dependencies to be installed on your computer
in order to compile it. 

## Compile GreatSPN on Linux/Unix/macOS/WSL

### Step 1: install dependencies

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
 

#### Fedora 

Install the dependencies with this command:

```
sudo dnf -y install gcc gcc-c++ gmp-devel gmp-c++ gmp boost-devel flex-devel \
 ant glib2-devel patch python glpk-devel autoconf automake ghostscript \
 libtool zip flex git byacc time graphviz suitesparse-devel motif-devel make \
 libxml++-devel glibmm24-devel java-latest-openjdk
```


#### Debian/Ubuntu

Install the dependencies with this command:

```
sudo apt-get install -y gcc g++ libgmp-dev libgmpxx4ldbl libboost-all-dev \
 flexc++ ant libglib2.0-dev patch python3 libglpk-dev liblpsolve55-dev \
 autoconf automake libtool zip flex byacc time graphviz libsuitesparse-dev \
 libmotif-dev make libxml++2.6-dev libglibmm-2.4-dev texlive-font-utils \
 openjdk-17-jdk cmake git ghostscript
```

This list was last tested on Ubuntu 20.04LTS, and it also applies to WSL-2 with Ubuntu.


#### macOS

GreatSPN compiles and runs successfully on macOS. 
You will first need to install the Developer Tools (Xcode) and latest [Java OpenJDK](https://jdk.java.net/).
Then install all the required dependencies using a software like [Homebrew](https://brew.sh/), e.g. 
by running the command:
```
brew install gmp boost flex byacc ant graphviz libxml++ \
 autoconf automake libtool suitesparse lp_solve ghostscript
```


---

### Step 2: compile dependent libraries

#### Meddly (decision diagram library)

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

#### Spot (ω-automata manipulation library)

To compile the CTL* model checker, the [Spot library](https://spot.lrde.epita.fr/) is required. Download and install it from the Spot website. 
A sample installation procedure could be the following:

```
wget http://www.lrde.epita.fr/dload/spot/spot-2.9.6.tar.gz  # The URL could be different
tar xzf spot-2.9.6.tar.gz
cd spot-2.9.6
./configure --disable-python --disable-debug
make
sudo make install
```

#### lpsolve (mixed integer linear programming solver)

lpsolve can be downloaded from the [official repository](https://sourceforge.net/projects/lpsolve/), or from our mirror using:

```
wget https://datacloud.di.unito.it/index.php/s/cx9oJ5wjsgEjKcs/download/lp_solve_5.5.2.11_source.tar.gz
```

and then extract and compile it either following the documentation, or by running these commands:

```
tar xzf lp_solve_5.5.2.11_source.tar.gz
wget https://raw.githubusercontent.com/greatspn/SOURCES/master/contrib/build_lpsolve.sh
sh ./build_lpsolve.sh
```

#### OGDF (open graph algorithms and data structures framework)

[OGDF](https://ogdf.uos.de/) is used for the automatic Petri net relayout feature of the graphical editor. It can be installed using these commands:
```
git clone https://github.com/ogdf/ogdf.git
cd ogdf
mkdir build && cd build && cmake .. && make
sudo make install
```
or, alternatively, follow the instructions directly from [the OGDF repository.](https://github.com/ogdf/ogdf)

---

### Step 3: compile the GreatSPN sources

Once all dependencies have been compiled and installed, create the main `GreatSPN` source directory. 
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
If not, on Linux modify the `/etc/ld.so.conf` and/or the environmental variable `LD_LIBRARY_PATH` accordingly.

**Missing GraphMDP**: Only in the case you are interested in solution of MDP systems, you can find the optional dependency GraphMDP [here](http://www.di.unito.it/~greatspn/graphMDP-0.5.tar.gz). Note that this library is currently unmantained, and it could be hard to compile.


---

#### Notes on Windows/WSL

GreatSPN is known to compile and run successfully using the *Windows Subsystem for Linux ver.2* (WSL-2, Windows 10 64bit minimum) using Ubuntu 20.04LTS.
WSL-2 installation is slightly different, since the GUI will be a standard Windows application, while the command line tools will reside and run inside the WSL-2 subsystem.

Follow the Linux steps to compile all the command line tools inside the WSL-2 subsystem.
If the WSL-2 component is newly installed, remember to update the apt-cache before installing the dependencies, 
i.e. run the command `sudo apt update`.
The you can:
 * (EASIEST) run the precompiled Windows GUI (see the [INSTALL](./INSTALL.md) page).
   In that case, the GUI by default will not run the solvers from WSL, but will run the pre-compiled
   versions that are shipped with GreatSPN package. To use the WSL package, go to Options > Solvers > and uncheck the "Use the portable App-Image" option.
 * run the GUI as a Linux application inside WSL (Windows 11 only)
 * copy the GUI from the WSL volume back into the Windows volume, and run it as a standard Windows application

The GUI for Windows is configured to use the tools inside the WSL-2 environment, so no additional step is needed.


GreatSPN command line tools can also build and run using [Cygwin](https://www.cygwin.com/).



### Compile GreatSPN in a Docker container

A two-stage self-building Dockerfile, based on Fedora, is provided in the `docker` folder.






