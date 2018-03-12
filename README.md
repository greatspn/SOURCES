
# The GreatSPN Framework version 3.0

**GreatSPN** (**GR**aphical **E**ditor and **A**nalyzer for **T**imed and **S**tochastic **P**etri **N**ets) 
is a software package for the modeling, validation, and performance evaluation of distributed systems 
using Generalized Stochastic Petri Nets and their colored extension, Stochastic Well-formed Nets. 
The tool provides a friendly framework to experiment with timed Petri net based modeling techniques. 
It implements efficient analysis algorithms to allow its use on rather complex applications, not only toy examples.









### How to compile and install GreatSPN on your PC.

GreatSPN can be compiled and installed on several POSIX-compliant systems.
The framework requires several dependencies. We provide an installation script that should simplify 
the installation. As of now, the only supported platform is Fedora. We will add other platforms later on.

#### Linux installation instructions:

On Fedora (version 18 or greater), type the following command at the prompt:
```
	curl http://github.com/XXXXXXXXXX/download.sh | bash -Fedora
```
This will create a GreatSPN/ directory, which will contain these subdirectories:
 * SOURCES: the main source tree of the GreatSPN framework
 * meddly: the meddly library, a required dependency for tools using Decision Diagrams.

Enter in each directory, compile and install. To install meddly, type:
```
	cd GreatSPN/meddly
	./autogen.sh
	./configure --prefix=/usr/local
	make
	sudo make install
```
To install the GreatSPN framework, type:
```
	cd GreatSPN/SOURCES
	make
	sudo make install
```

On other Linux distributions, you will first need to install the following dependencies 
(using the developer packages with the header files):
 * gcc and g++ (with support for C++14)
 * autotools (autoconf, automake, libtool, ...)
 * libgmp (GNU multiprecision library) (devel)
 * boost-c++ version 1.64+
 * flex (devel) and byacc
 * ant and Java 1.8+ (Oracle JDK)
 * graphviz
 * libxml++ (devel), glibmm24 (devel) and glib2 (devel)
 * glpk (devel) and lpsolve (devel)

After that, type the command:
```
	curl http://github.com/XXXXXXXXXX/install.sh | bash
```
and follow the above instruction to compile meddly and GreatSPN.


#### MacOSX installation instructions:

GreatSPN is known to be working on macOS X after having installed all the required dependencies 
using Homebrew.

TBD.


#### Windows installation instructions:

GreatSPN compiles and runs successfully using the WSL (Windows 10 64bit minimum).

TBD.

#### Pre-installed VirtualBox machine:

We offer a pre-installed VirtualBox machine image, based on Fedora Core, that ships with
the GreatSPN framework already installed, with all the rewuired dependencies.
This is the easiest way of getting GreatSPN, since you just have to download the image.
The VirtualBox image can be downloaded from this [repository](www.di.unito.it/~greatspn/VBox/).
In case you need it, the root password of the VirtualBox image is: fedora

Sometimes the virtual box machine will require the re-installation of
the so-called "Guest Additions", which allows to share directories with the host pc.
Follow the instruction on the VirtualBox site on how to install the Guest Additions.



TBD. Instructions on how to update the shared directory.








### How to run the Graphical User Interface.

TBD.










### How to run the command-line solvers.

When GreatSPN is installed on your PC, it will (by default) place itself 
in the: /usr/local/GreatSPN directory. The following sub-directory will be created:
 * ${GREATSPN_HOME}/scripts:  the scripts that invoke the solver pipelines.
 * ${GREATSPN_HOME}/bin:  the individual solver binaries.
 * ${GREATSPN_HOME}/models:  the default model library available from the GUI.










### Copyright and License

The GreatSPN Framework is licensed under the GPLv2.0 license, 
which is available within the repository in the [LICENSE](LICENSE) file. 
The source code owner is the University of Torino, Italy.
If you are interested in a different licensing scheme, please [contact us](mailto:greatspn@di.unito.it).
Any contributing code is not owned by the contributor.
Contributors are required to sign a [Contributor License Agreement](CLA.txt) (CLA) 
when contributing code to the GreatSPN Framework.




