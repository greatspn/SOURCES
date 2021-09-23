#================================================================================
# Stage 1: build all the GreatSPN modules and their dependencies
#================================================================================
FROM fedora AS builder

RUN dnf -y install git wget

RUN sudo dnf -y install gcc gcc-c++ gmp-devel gmp-c++ gmp boost-devel flex-devel \
      ant glib2-devel patch python glpk-devel lpsolve-devel autoconf automake \
      libtool zip flex byacc time graphviz suitesparse-devel motif-devel make \
      libxml++-devel glibmm24-devel lpsolve texlive-epstopdf cmake diff

ENV MAKE_ARGS=-j16

# Install Meddly
RUN git clone  https://github.com/asminer/meddly.git meddly ;\
	cd meddly ;\
	./autogen.sh ;\
	./configure --prefix=/usr/local ;\
	make ${MAKE_ARGS} ;\
	sudo make install

# Install SPOT
# NOTE: The URL could change
RUN wget http://www.lrde.epita.fr/dload/spot/spot-2.9.6.tar.gz ;\
	tar xzf spot-2.9.6.tar.gz ;\
	cd spot-2.9.6 ;\
	./configure --disable-python ;\
	make ${MAKE_ARGS} ;\
	sudo make install

# Install ogdf
RUN git clone https://github.com/ogdf/ogdf ;\
	cd ogdf ;\
	./makeMakefile.sh ;\
	mkdir build && cd build && cmake .. && make ${MAKE_ARGS} ;\
	sudo make install

# Install GreatSPN
RUN mkdir ~/GreatSPN ;\
	cd ~/GreatSPN ;\
	git clone https://github.com/greatspn/SOURCES.git SOURCES ;\
	cd ~/GreatSPN/SOURCES ;\
	make ${MAKE_ARGS} -k derived_objects ;\
	make ${MAKE_ARGS} ;\
	make ;\
	sudo make install

#================================================================================
# Stage 2: prepare the deployment image with the non-devel packages
#================================================================================
FROM fedora AS deploy_base

RUN sudo dnf -y install gmp gmp-c++ gmp boost flex \
      glib2 patch glpk lpsolve  \
      time graphviz suitesparse motif make \
      libxml++ glibmm24 
# texlive-epstopdf python ant autoconf automake libtool zip byacc

RUN sudo dnf -y install java-11-openjdk gcc gcc-c++ make

#================================================================================
# Stage 3: Update the GreatSPN framework by pulling/remaking the latest changes
#================================================================================
FROM builder AS updater

ARG update_version
RUN cd ~/GreatSPN/SOURCES ;\
    git pull ;\
    make ${MAKE_ARGS} -k derived_objects ;\
    make ${MAKE_ARGS} ;\
    make ;\
    sudo make install

#================================================================================
# Stage 4: Copy the compiled binaries into the deployment image
#================================================================================
FROM deploy_base

# copy the compiled binaries
COPY --from=updater /usr/local/GreatSPN /usr/local/GreatSPN 
COPY --from=updater /usr/local/lib /usr/local/lib
RUN rm -rf /usr/local/lib/*.a /usr/local/lib/*.la

# add library & binary paths
RUN echo '/usr/local/lib' > /etc/ld.so.conf.d/greatspn.conf ; ldconfig
RUN echo 'export PATH=$PATH:/usr/local/GreatSPN/scripts' > /etc/bashrc












