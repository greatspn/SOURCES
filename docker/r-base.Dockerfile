#================================================================================
# Stage 1: build all the GreatSPN modules and their dependencies
#================================================================================
FROM r-base AS builder

RUN sudo apt-get update

RUN sudo apt-get install -y gcc g++ libgmp-dev libgmpxx4ldbl libboost-all-dev \
     flexc++ ant libglib2.0-dev patch python3 libglpk-dev liblpsolve55-dev \
     autoconf automake libtool zip flex byacc time graphviz libsuitesparse-dev \
     libmotif-dev make libxml++2.6-dev libglibmm-2.4-dev texlive-font-utils \
     openjdk-17-jdk cmake git ghostscript

ENV MAKE_ARGS=-j4

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
    ./configure --disable-python --disable-debug ;\
    make ${MAKE_ARGS} ;\
    sudo make install

# Install ogdf
RUN git clone https://github.com/ogdf/ogdf ;\
    cd ogdf ;\
    mkdir build && cd build && cmake .. && make ${MAKE_ARGS} ;\
    sudo make install

# Install lpsolve (latest)
RUN wget https://datacloud.di.unito.it/index.php/s/JFsJwyHfJ9FNWZJ/download/lp_solve_5.5.2.11_source.tar.gz ;\
    wget https://raw.githubusercontent.com/greatspn/SOURCES/master/contrib/build_lpsolve.sh ;\
    tar xzf lp_solve_5.5.2.11_source.tar.gz ;\
    sh ./build_lpsolve.sh 


# Install GreatSPN
RUN mkdir ~/GreatSPN ;\
    cd ~/GreatSPN ;\
    git clone https://github.com/greatspn/SOURCES.git SOURCES ;\
    cd ~/GreatSPN/SOURCES ;\
	  git pull ;\
    make ${MAKE_ARGS} -k derived_objects ;\
    make ${MAKE_ARGS} ;\
    make ;\
    sudo make install

#================================================================================
# Stage 2: prepare the deployment image with the non-devel packages
#================================================================================
FROM r-base AS deploy_base

RUN sudo apt-get update

RUN sudo apt-get install -y gcc g++ libgmp libgmpxx4ldbl libboost-all \
     flexc++ ant libglib2.0 patch python3 libglpk liblpsolve55 \
     flex time graphviz libsuitesparse byson \
     libmotif make libxml++2.6 libglibmm-2.4 \
     openjdk-17-jre git ghostscript

# RUN sudo dnf -y install gmp gmp-c++ gmp boost flex \
#     glib2 patch glpk lpsolve  \
#     time graphviz suitesparse motif make \
#     libxml++ glibmm24 ghostscript java-latest-openjdk \
#     gcc gcc-c++ make perl

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
RUN sudo dnf -y install glpk-devel

# add library & binary paths
RUN echo '/usr/local/lib' > /etc/ld.so.conf.d/greatspn.conf ; ldconfig
ENV PATH="$PATH:/usr/local/GreatSPN/scripts"










