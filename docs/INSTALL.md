
## How to install the GreatSPN binary distribution on your PC

GreatSPN is available as a pre-built binary distribution for the following platforms:

 * Windows (x86-64)
 * Linux (deb and rpm packages)
 * MacOS (arm64, x86-64 incoming)

To install GreatSPN, follow your platform's instructions.

### Install on Windows

1. Download and install Graphviz from the [official repository](https://graphviz.org/download/).
   When asked, select **"Add Graphviz to the system PATH for all users"**, so that GreatSPN can find and use it to generate the diagrams.

2. Download and install Ghostscript from the [official repository](https://www.ghostscript.com/releases/gsdnld.html). You have to install the **Postscript and PDF interpreter/renderer**.
Once installed, check that Ghostscript is properly added to the PATH Environment Variable.

3. Download and install the `.msi` installer from the [GreatSPN binary repository](https://datacloud.di.unito.it/index.php/s/E3kzX9ntCTnpRNi).

4. GreatSPN is added to the Windows Start menu.


### Install on Linux

You need first to install the external packages.

On **Ubuntu** or other Debian-based distributions, install the following packages:
```
sudo apt-get install -y graphviz ghostscript
```
Then download the `.deb` package from the [GreatSPN binary repository](https://datacloud.di.unito.it/index.php/s/E3kzX9ntCTnpRNi), and install it using:
```
sudo apt install -y greatspn_3.2_amd64.deb
```

On **Fedora** or other RedHat-based distributions, install the following packages:
```
sudo dnf -y install graphviz ghostscript
```
Then download the `.rpm` package from the [GreatSPN binary repository](https://datacloud.di.unito.it/index.php/s/E3kzX9ntCTnpRNi), and install it using:
```
sudo dnf install -y greatspn-3.2.x86_64.rpm
```

### Install on macOS

You first need to install a minimal set of dependencies using a software like [Homebrew](https://brew.sh/), e.g. by running the command:
```
brew install graphviz ghostscript 
```
After that, download the `.dmg` package from the [GreatSPN binary repository](https://datacloud.di.unito.it/index.php/s/E3kzX9ntCTnpRNi), open it and drag the GreatSPN application bundle in the Application directory.
GreatSPN will then run directly as a standard macOS application.



### Pre-installed VirtualBox machine (OLD)

We offer a pre-installed VirtualBox machine image, based on Ubuntu, that ships with
the GreatSPN framework already installed, with all the required dependencies.
This is the easiest way to get GreatSPN together with the pre-compiled source, since you just have to download the image, but of course the VirtualBox machine will be slower than a native build.
The VirtualBox image can be downloaded from this [repository](http://www.di.unito.it/~greatspn/VBox/).
In case you need it, the root password of the VirtualBox image is: `user`

Sometimes the virtual box machine will require the re-installation of
the so-called "Guest Additions", which allows to share directories with the host pc.
Follow the instruction on the VirtualBox site on how to install the Guest Additions.

TBD. Instructions on how to update the shared directory.

