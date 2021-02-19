
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
 * ant and Java 11+
 * graphviz
 * libxml++ (devel), glibmm24 (devel) and glib2 (devel)
 * libcolamd2, on fedora platform provided by suitesparse (devel)
 * glpk (devel) and lpsolve (devel)
 
For Fedora 32, you can install the dependencies with this command:

```
sudo dnf -y install gcc gcc-c++ gmp-devel gmp-c++ gmp boost-devel flex-devel \
 ant glib2-devel patch python glpk-devel lpsolve-devel autoconf automake \
 libtool zip flex git byacc time graphviz suitesparse-devel motif-devel make 
 libxml++-devel glibmm24-devel lpsolve texlive-epstopdf
```

For Debian/Ubuntu, the list of dependencies is installed with the command:
```
sudo apt-get install -y gcc g++ libgmp-dev libgmpxx4ldbl libboost-all-dev \
 flexc++ ant libglib2.0-dev patch python3 libglpk-dev liblpsolve55-dev \
 autoconf automake libtool zip flex byacc time graphviz libsuitesparse-dev \
 libmotif-dev make libxml++2.6-dev libglibmm-2.4-dev texlive-font-utils
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
./configure
make
make install
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
You could also need to install MacTex to have the (optional) `epstopdf` command, 
needed by some visualization functionalities.


#### Windows installation instructions:

GreatSPN is known to compile and run successfully using the WSL-2 (Windows 10 64bit minimum) using Ubuntu 20.04LTS.
WSL installation is slightly different, since the GUI will be a standard Windows application, while the command line tools will reside and run inside the WSL subsystem.

Follow the Linux steps to compile all the command line tools inside the WSL subsystem, then download a precompiled GUI from [here](http://www.di.unito.it/~amparore/mc4cslta/editor.html). You can use either the Windows version, or the JAR version in case the Windows version raises problems.
The GUI for Windows is automatically configured to use the tools inside the WSL environment, so no additional step is needed.


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

To cite the decision-diagram based model checker RGMEDD, please use this reference [PDF](https://aperto.unito.it/retrieve/handle/2318/1764225/685722/Variable%20order%20metrics%20for%20decision%20diagrams%20in%20system%20verification%20-%20STTT.pdf):

```
@article{bib:STTTvarordmetrics,
  title={Variable order metrics for decision diagrams in system verification},
  author={Amparore, Elvio Gilberto and Donatelli, Susanna and Ciardo, Gianfranco},
  journal={International Journal on Software Tools for Technology Transfer},
  pages={1--22},
  year={2019},
  publisher={Springer}
}
```

Other recent papers referencing GreatSPN:

```
@inproceedings{bib:ctlstar:greatspn,
  author="Amparore, Elvio Gilberto and Donatelli, Susanna and Gall{\`a}, Francesco",
  title="A CTL* Model Checker for Petri Nets",
  booktitle="Application and Theory of Petri Nets and Concurrency",
  year="2020",
  publisher="Springer International Publishing",
  address="Cham",
  pages="403--413",
  isbn="978-3-030-51831-8"
}
```
PDF [here](https://iris.unito.it/retrieve/handle/2318/1764228/685729/A%20CTLstar%20model%20checker%20for%20Petri%20nets.pdf).


### Recent Awards (Model Checking Context 2018 edition):

GreatSPN partecipated to several Model Checking Context (MCC) editions:

 * MCC 2018 Edition [(link)](https://mcc.lip6.fr/2018/results.php):
    - 🥇 Gold medal in StateSpace category;
    - 🥉 Bronze medal in UpperBounds category;
    - ✔️ 100% accuracy of the results.
 * MCC 2019 Edition [(link)](https://mcc.lip6.fr/2019/results.php):
    - 🥈 Silver medal in StateSpace category;
    - ✔️ 100% accuracy of the results.
 * MCC 2020 Edition [(link)](https://mcc.lip6.fr/2020/results.php):
    - 🥈 Silver medal in StateSpace category;
    - 🥈 Silver medal in GlobalProperties category;
    - 🥉 Bronze medal in UpperBounds category;

See the [Model Checking Context site](https://mcc.lip6.fr) for further details.



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




