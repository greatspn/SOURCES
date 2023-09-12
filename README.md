
# The GreatSPN Framework version 3.1

**GreatSPN** (**GR**aphical **E**ditor and **A**nalyzer for **T**imed and **S**tochastic **P**etri **N**ets) 
is a software package for the modeling, validation, and performance evaluation of distributed systems 
using Generalized Stochastic Petri Nets and their colored extension, Stochastic Well-formed Nets. 
The tool provides a friendly framework to experiment with (stochastic) Petri net based modeling techniques. 
It implements efficient analysis algorithms to allow its use on rather complex applications.


## Install GreatSPN

Follow the [instructions](docs/INSTALL.md) on how to install GreatSPN on Linux, macOS or Windows. The provided binaries install a working subset of GreatSPN, which should be enough for most users. The binary installation requires *Graphviz* to be installed. Follow the instuctions on how to install it on your platform.


## Compile GreatSPN from sources

GreatSPN full functionalities (i.e. scripting) are available when it is installed from sources.
Follow the [instructions](docs/COMPILE.md) on how to compile the framework from sources.


## How to run the Graphical User Interface.

If the installation of GreatSPN is successful, the GreatSPN Editor should appear
in the system application menu. 
In case the Editor is built from sources, you can launch it from the command line
with this command:
```
greatspn_editor
```

## How to run the command-line solvers.

When GreatSPN is installed on your PC from sources on Unix-like systems, it will (by default) place itself in the: `/usr/local/GreatSPN` directory. 
The following sub-directories will be created:

 * `${GREATSPN_HOME}/scripts`:  the scripts that invoke the solution pipelines.
 * `${GREATSPN_HOME}/bin`:  the individual solver binaries.
 * `${GREATSPN_HOME}/models`:  the default models available from the GUI.

Typically, most tools are accessed through a script in the first directory.

**TBD**: description of the core tools.



## Citing GreatSPN:

To cite the GreatSPN framework, please use this reference: [(PDF)](https://iris.unito.it/retrieve/handle/2318/1624717/295450/Amparore-trivedi-chapter.pdf)

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

To cite the decision-diagram based model checker RGMEDD, please use this reference: [(PDF)](https://aperto.unito.it/retrieve/handle/2318/1764225/685722/Variable%20order%20metrics%20for%20decision%20diagrams%20in%20system%20verification%20-%20STTT.pdf)

```
@article{bib:STTTvarordmetrics,
  title={Variable order metrics for decision diagrams in system verification},
  author={Amparore, Elvio Gilberto and Donatelli, Susanna and Ciardo, Gianfranco},
  journal={International Journal on Software Tools for Technology Transfer},
  pages={541--562},
  year={2019},
  publisher={Springer}
}
```

Other recent papers referencing GreatSPN:
[(link)](https://peerj.com/articles/cs-823/).

```
@article{bib:greatspn:starMC,
  author    = {Amparore, Elvio Gilberto and 
               Donatelli, Susanna and 
               Gall{\`a}, Francesco},
  title     = {starMC: an automata based CTL* model checker},
  journal   = {PeerJ Comput. Sci.},
  volume    = {8},
  pages     = {e823},
  year      = {2022},
  url       = {https://doi.org/10.7717/peerj-cs.823},
  doi       = {10.7717/peerj-cs.823},
}
```


## Recent Awards (Model Checking Context):

GreatSPN partecipated to several Model Checking Context (MCC) editions:

 * MCC 2022 Edition [(link)](https://mcc.lip6.fr/2022/results.php):
    - 🥈 Silver medal in StateSpace category;
    - 🥉 Bronze medal in GlobalProperties, UpperBounds and CTL categories;
 * MCC 2021 Edition [(link)](https://mcc.lip6.fr/2021/results.php):
    - 🥈 Silver medal in StateSpace category;
 * MCC 2020 Edition [(link)](https://mcc.lip6.fr/2020/results.php):
    - 🥈 Silver medal in StateSpace category;
    - 🥈 Silver medal in GlobalProperties category;
    - 🥉 Bronze medal in UpperBounds category;
 * MCC 2019 Edition [(link)](https://mcc.lip6.fr/2019/results.php):
    - 🥈 Silver medal in StateSpace category;
    - ✔️ 100% accuracy of the results.
 * MCC 2018 Edition [(link)](https://mcc.lip6.fr/2018/results.php):
    - 🥇 Gold medal in StateSpace category;
    - 🥉 Bronze medal in UpperBounds category;
    - ✔️ 100% accuracy of the results.

See the [Model Checking Context site](https://mcc.lip6.fr) for further details.



## Copyright and License

The GreatSPN Framework is licensed under the GPLv2.0 license, 
which is available within the repository in the [LICENSE](LICENSE) file. 
The source code owner is the University of Torino, Italy.
If you are interested in a different licensing scheme, please [contact us](mailto:greatspn@di.unito.it).
Any contributing code is not owned by the contributor.
Contributors are required to sign a [Contributor License Agreement](CONTRIBUTING) (CLA) 
when contributing code to the GreatSPN Framework.




