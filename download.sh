#!/bin/bash

# This script downloads the GreatSPN repository from GitHub, along with
# the required dependent repositories (meddly) from their respective sources.
# It can also compile the repositories and install them, and install
# the required packages on some supported platforms.


echo "TERM=${TERM}"
if [[ ! -z ${TERM} ]] ; then
    B1=$(tput bold)
    B0=$(tput sgr0)
    C1=$(tput setaf 1)
    C0=$(tput setaf 9)
fi
LN="==============================================================================="

function print_header {
	echo "${B1}${LN}"
	echo "$1" && echo "${LN}${B0}" && echo
}
function fail {
	echo && echo "${C1}${B1} Error:${B0} $1" && echo
	exit 1
}

#==============================================================================
# Parse command line arguments
#==============================================================================
PLATFORM="-no-deps"
DEVMODE="N"
ONLY_DEPS="N"
BUILD_MEDDLY="Y"
INSTALL_MEDDLY="Y"
BUILD_GREATSPN="Y"
INSTALL_GREATSPN="Y"
SUDO=sudo

for i in "$@"
do
	case $i in
		"-Fedora"*|"-FedoraVBox"|"-Debian"*|"-WSL"|"-macOS"|"-no-deps")
			PLATFORM=$i
			shift
			;;
		"-dev")
			DEVMODE="Y"
			shift
			;;
		"-only-deps")
			ONLY_DEPS="Y"
			shift
			;;
		"-docker")
			BUILD_GREATSPN="Y"
			INSTALL_GREATSPN="Y"
			BUILD_MEDDLY="Y"
			INSTALL_MEDDLY="Y"
			SUDO="" # On Docker we are alreay root, and sudo is not available
			;;
		"-dont-install")
			INSTALL_GREATSPN="N" ; shift
			;;
		"-dont-build")
			BUILD_GREATSPN="N" ; shift
			;;
		"-dont-build-meddly")
			BUILD_MEDDLY="N" ; shift
			;;
		"-dont-install-meddly")
			INSTALL_MEDDLY="N" ; shift
			;;
	    # -e=*|--extension=*)
	    # EXTENSION="${i#*=}"
	    # shift # past argument=value
	    # ;;
	    *)
	    fail "Unknown option: ${i}."
	    ;;
	esac
done

#==============================================================================
# Start of installation script
#==============================================================================
print_header "GreatSPN Framework v3.0 installation script"

#==============================================================================
# Install dependencies
#==============================================================================
print_header "Installing required GreatSPN dependencies.."

case ${PLATFORM} in

	"-Fedora"*|"-FedoraVBox")
		# Install base packages (Fedora system)
		dnf -y install gcc gcc-c++ gmp-devel gmp-c++ gmp boost-devel flex-devel \
					   ant glib2-devel patch python libxml-devel glpk-devel lpsolve-devel \
					   autoconf automake libtool zip flex git \
					   byacc time graphviz  motif-devel \
					   libxml++-devel glibmm24-devel lpsolve-devel \
			|| fail "Could not install the required dependencies using dnf."
		# Dependencies for the Virtual Box image.
		if [[ $PLATFORM == "FedoraVBox" ]] ; then
			dnf -y install terminus-fonts subversion evince cups-pdf
		fi
		
		;;

	"-Debian"*|"-WSL")
		fail "TBD: Dependencies using apt."
		exit 1
		;;

	"-macOS")
		fail "TBD: Dependencies using brew."
		exit 1
		# Make libgmp visible to the meddly configure script
		export CXXFLAGS=-I/usr/local/opt/gmp/include/ 
		export LIBS=-L/usr/local/opt/gmp/lib/
		;;

	"-no-deps")
		echo "Skipping dependency installation."
		;;

	*)
		fail "You must specify a platform. Cannot install the required dependencies."
		exit -1
		;;
esac
#==============================================================================

if [[ $ONLY_DEPS == "Y" ]] ; then
	echo "All dependencies have been installed. Quit."
    exit
fi



#==============================================================================
# Patching bug in boost
#==============================================================================
# if [[ -e /usr/include/boost/numeric/ublas/matrix.hpp ]] ; then
# 	sudo sed -i 's/fixed_matrix \&operator = (matrix m) {/fixed_matrix \&operator = (fixed_matrix m) {/g' \
# 		/usr/include/boost/numeric/ublas/matrix.hpp
# fi
# if [[ -e /usr/local/include/boost/numeric/ublas/matrix.hpp ]] ; then
# 	sudo sed -i 's/fixed_matrix \&operator = (matrix m) {/fixed_matrix \&operator = (fixed_matrix m) {/g' \
# 		/usr/local/include/boost/numeric/ublas/matrix.hpp
# fi


#==============================================================================
# Prepare the source directory
#==============================================================================

mkdir -p GreatSPN
cd GreatSPN

# Setup public repository URLs
GREATSPN_SOURCES=https://github.com/greatspn/SOURCES.git

if [[ $DEVMODE == "Y" ]] ; then
	if [[ ! -e ~/.ssh/id_rsa.pub ]] ; then
		fail "Cannot find the SSH key in ~/.ssh/id_rsa.pub . Cannot clone the repositories in developer mode."
	fi
	# Use SSH repositories (require developer access and a valid ~/.ssh/id_rsa key)
	# GREATSPN_SOURCES=git@gitlab.di.unito.it:amparore/GreatSPN-public.git
	GREATSPN_PRIVATE=git@gitlab.di.unito.it:amparore/GreatSPN-private.git
fi


#==============================================================================
# Download and install meddly
#==============================================================================

if [[ $BUILD_MEDDLY == "Y" ]] ; then
	(
		if cd meddly ; 
		then 
			print_header "Updating the Meddly repository..." 
			git checkout master || fail "Cannot checkout to master"
			git pull || fail "Cannot pull from the Meddly repository" 
		else 
			print_header "Downloading the Meddly repository..." 
			git clone git://git.code.sf.net/p/meddly/code-git meddly \
						|| fail "Cannot clone the Meddly repository."  
		fi
	) || fail "Could not download the Meddly repository."

	print_header "Compiling the meddly library..."
	(
		cd meddly 
		aclocal && autoconf && libtoolize && autoheader && automake --add-missing
		./autogen.sh 
		CXXFLAGS="-O2 ${CXXFLAGS}" LDFLAGS="-O2 ${LDFLAGS}" ./configure --prefix=/usr/local
		make ${MKFLAGS} 
	) || fail "Could not compile the Meddly library. Check the log for errors." 
fi

if [[ $INSTALL_MEDDLY == "Y" ]] ; then
	print_header "Installing the meddly library..."
	(
		cd meddly 
		${SUDO} make install 
	) || fail "Could not install the Meddly library."
fi



#==============================================================================
# Download and install GreatSPN
#==============================================================================

if [[ $BUILD_GREATSPN == "Y" ]] ; then
	(
		if cd SOURCES ; 
		then 
			print_header "Updating the GreatSPN repository..." 
			git checkout master || fail "Cannot checkout to master"
			git pull || fail "Cannot pull from the GreatSPN repository" 
			# svn update . --username XXXX --password XXXX --non-interactive --force \
			# 			|| fail "Cannot pull from the GreatSPN repository"
		else 
			print_header "Downloading the GreatSPN repository..." 
			git clone ${GREATSPN_SOURCES} SOURCES || fail "Cannot clone the GreatSPN repository."
			# svn co https://svn.di.unito.it/repository/amparore/amparore-svn/GreatSPN \
	  #    		--username XXXX --password XXXX --non-interactive --force \
			# 			|| fail "Cannot clone the GreatSPN repository."
		fi
	) || fail "Could not download the GreatSPN repository."

	if [[ $DEVMODE == "Y" ]] ; then
		(
			if cd PRIVATE ;
			then
				print_header "Updating the internal GreatSPN repository..." 
				git checkout master || fail "Cannot checkout to master"
				git pull || fail "Cannot pull from the internal GreatSPN repository"
			else
				print_header "Downloading the internal GreatSPN repository..." 
				git clone ${GREATSPN_PRIVATE} PRIVATE || fail "Cannot clone the internal GreatSPN repository."
			fi
		) || fail "Could not download the internal GreatSPN repository."
	fi 

	print_header "Compiling and installing GreatSPN from sources..."
	# Compile twice to avoid missing dependencies...
	(
		cd SOURCES
		CFLAGS="-O2 ${CFLAGS}" CPPFLAGS="-O2 ${CPPFLAGS}" LDFLAGS="-O2 ${LDFLAGS}" make -k ${MKFLAGS} derived_objects
		CFLAGS="-O2 ${CFLAGS}" CPPFLAGS="-O2 ${CPPFLAGS}" LDFLAGS="-O2 ${LDFLAGS}" make -k ${MKFLAGS} derived_objects
		CFLAGS="-O2 ${CFLAGS}" CPPFLAGS="-O2 ${CPPFLAGS}" LDFLAGS="-O2 ${LDFLAGS}" make -k ${MKFLAGS} 
		CFLAGS="-O2 ${CFLAGS}" CPPFLAGS="-O2 ${CPPFLAGS}" LDFLAGS="-O2 ${LDFLAGS}" make -k ${MKFLAGS} \
			|| fail "Could not compile GreatSPN. Check the log for errors."
		# make install 
	) || fail "Could not compile and install the GreatSPN framework."
fi


#==============================================================================
if [[ $INSTALL_GREATSPN == "Y" ]] ; 
then
	#==============================================================================
	# Install the compiled tools
	#==============================================================================

	print_header "Installing GreatSPN..."
	(
		cd SOURCES 
		${SUDO} make install 
	) || fail "Could not install GreatSPN."

	#==============================================================================
	# Post-installation script
	#==============================================================================
	print_header "Post-installation script..."

	if [ -e ~/.bashrc ] ; then
		if grep -q '/usr/local/GreatSPN/script' ~/.bashrc ; then
			echo 'PATH=$PATH:/usr/local/GreatSPN/scripts' >> ~/.bashrc
		fi
	fi
	if [ -e ~/.bash_profile ] ; then
		if grep -q '/usr/local/GreatSPN/script' ~/.bash_profile ; then
			echo 'PATH=$PATH:/usr/local/GreatSPN/scripts' >> ~/.bash_profile
		fi
	fi

fi
#==============================================================================

