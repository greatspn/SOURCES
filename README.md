
# The GreatSPN Framework version 3.0

**GreatSPN** (**GR**aphical **E**ditor and **A**nalyzer for **T**imed and **S**tochastic **P**etri **N**ets) 
is a software package for the modeling, validation, and performance evaluation of distributed systems 
using Generalized Stochastic Petri Nets and their colored extension, Stochastic Well-formed Nets. 
The tool provides a friendly framework to experiment with timed Petri net based modeling techniques. 
It implements efficient analysis algorithms to allow its use on rather complex applications.


### How to compile and install GreatSPN on your PC.

GreatSPN can be compiled and installed on most POSIX-compliant systems.
The framework requires several dependencies to be installed on your computer
in order to compile it. 

#### Linux/Unix installation instructions:

In order to compile and install GreatSPN from sources, you need to do the following steps.
First, install the following dependencies (using the developer packages with the header files):
 * gcc and g++ (with support for C++14)
 * autotools (autoconf, automake, libtool, ...)
 * libgmp (GNU multiprecision library) (devel)
 * boost-c++ version 1.64+
 * flex (devel) and byacc
 * ant and Java 1.8+ (Oracle JDK)
 * graphviz
 * libxml++ (devel), glibmm24 (devel) and glib2 (devel)
 * libcolamd2, on fedora platform provided by suitesparse (devel)
 * glpk (devel) and lpsolve (devel)
 
For Fedora 32, you can install the dependencies with this command:

```
sudo dnf -y install gcc gcc-c++ gmp-devel gmp-c++ gmp boost-devel flex-devel ant glib2-devel \
 patch python glpk-devel lpsolve-devel autoconf automake libtool zip flex git byacc time \
 graphviz suitesparse-devel motif-devel make libxml++-devel glibmm24-devel lpsolve
```

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


To compile the CTL* model checker, the [Spot library](https://spot.lrde.epita.fr/) is required. Download and install it from the Spot website.

Once Meddly and Spot are both compiled and installed, create the main `GreatSPN` source directory with this command:

```
mkdir ~/GreatSPN
cd ~/GreatSPN
git clone https://github.com/greatspn/SOURCES.git SOURCES
cd ~/GreatSPN/SOURCES
make
sudo make install
```

to download, compile and install the sources. 


#### macOS installation instructions:

GreatSPN compiles and runs successfully on macOS. 
Before running the Linux/Unix installation steps, you will first need to install the Developer Tools (Xcode)
and Oracle Java (version 8 or greater).
Then install all the required dependencies using a software like [Homebrew](https://brew.sh/), e.g. 
by running the command:
```
brew install gmp boost flex byacc ant graphviz libxml++ autoconf automake libtool suitesparse lp_solve
```
In the last Homebrew version, the lp_solve package could require to be built from sources.
In that case, download the lp_solve sources from Sourceforge and follow the instructions
to compile the package.
After you have installed Xcode, brew, and the required dependecies, follow
the Linux/Unix instructions to compile Meddly and GreatSPN.


#### Windows installation instructions:

GreatSPN is known to compile and run successfully using the WSL (Windows 10 64bit minimum).

TBD.

#### Pre-installed VirtualBox machine:

We offer a pre-installed VirtualBox machine image, based on Fedora Core, that ships with
the GreatSPN framework already installed, with all the required dependencies.
This is the easiest way to get GreatSPN, since you just have to download the image.
The VirtualBox image can be downloaded from this [repository](http://www.di.unito.it/~greatspn/VBox/).
In case you need it, the root password of the VirtualBox image is: `fedora`

Sometimes the virtual box machine will require the re-installation of
the so-called "Guest Additions", which allows to share directories with the host pc.
Follow the instruction on the VirtualBox site on how to install the Guest Additions.



TBD. Instructions on how to update the shared directory.





### How to run the Graphical User Interface.

If the installation of GreatSPN is successful, the GreatSPN Editor should appear
in the Linux application menu. Sometimes, a logout is required.
In case the Editor does not appear in the system menu, you can launch it from the command line
with this command:
```
greatspn_editor
```
For macOS users, a compiled app is generated in the `SOURCES/JavaGUI/Editor/dist` directory.




### Citing GreatSPN:

To cite the GreatSPN framework, please use this reference [PDF](https://iris.unito.it/retrieve/handle/2318/1624717/295450/Amparore-trivedi-chapter.pdf):

```
@incollection{bib:30YearsOfGreatSPN,
  title={30 years of {GreatSPN}},
  author={Amparore, Elvio Gilberto and Balbo, Gianfranco and Beccuti, Marco 
  	      and Donatelli, Susanna and Franceschinis, Giuliana},
  booktitle={Principles of Performance and Reliability Modeling and Evaluation},
  pages={227--254},
  year={2016},
  publisher={Springer}
}
```


### Recent Awards (Model Checking Context 2018 edition):

GreatSPN partecipated to several Model Checking Context (MCC) editions:

 * 2018 Edition [MCC2018](https://mcc.lip6.fr/2018/results.php):
    - 🥇 Gold medal in StateSpace category;
    - 🥉 Bronze medal in UpperBounds category;
    - ✔️ 100% accuracy of the results.
 * 2019 Edition [MCC2019](https://mcc.lip6.fr/2019/results.php):
    - 🥈 Silver medal in StateSpace category;
    - ✔️ 100% accuracy of the results.

See the [MCC site](https://mcc.lip6.fr) for further details.



### How to run the command-line solvers.

When GreatSPN is installed on your PC, it will (by default) place itself 
in the: `/usr/local/GreatSPN` directory. The following sub-directory will be created:

 * `${GREATSPN_HOME}/scripts`:  the scripts that invoke the solver pipelines.
 * `${GREATSPN_HOME}/bin`:  the individual solver binaries.
 * `${GREATSPN_HOME}/models`:  the default model library available from the GUI.

Typically, most tools are accessed through a script in the first directory.

**TBD**: description of the core tools.



### Copyright and License

The GreatSPN Framework is licensed under the GPLv2.0 license, 
which is available within the repository in the [LICENSE](LICENSE) file. 
The source code owner is the University of Torino, Italy.
If you are interested in a different licensing scheme, please [contact us](mailto:greatspn@di.unito.it).
Any contributing code is not owned by the contributor.
Contributors are required to sign a [Contributor License Agreement](CONTRIBUTING) (CLA) 
when contributing code to the GreatSPN Framework.




