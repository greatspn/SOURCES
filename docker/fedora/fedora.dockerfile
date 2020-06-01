# contributed by millergarym
FROM fedora

RUN dnf -y install git

RUN dnf -y install \
    ant \
    autoconf \
    automake \
    boost-devel \
    byacc \
    flex-devel \
    flex git \
    gcc \
    gcc-c++ \
    glib2-devel \
    glibmm24-devel \
    glpk-devel \
    gmp \
    gmp-c++ \
    gmp-devel \
    graphviz \
    libtool \
    libxml++-devel \
    libxml-devel \
    lpsolve-devel \
    make \
    motif-devel \
    patch \
    python \
    suitesparse-devel \
    time \
    zip

RUN git clone git://git.code.sf.net/p/meddly/code-git meddly
RUN cd meddly; \
    aclocal && autoconf && libtoolize && autoheader && automake --add-missing \
    && ./autogen.sh \
    &&  LDFLAGS="-O2" ./configure --prefix=/usr/local \
    && make -j 4 \
    && make install

# # Makefile:145: "The GraphMDP library is not installed. Some packages will not be compiled."
# # Makefile:158: "The libXML++-2.6 library is not installed. Some packages will not be compiled."
# # Makefile:168: "The glibmm-2.4 library is not installed. Some packages will not be compiled."
# # Makefile:178: "The GLPJ library is not installed. Some packages will not be compiled."

WORKDIR /SOURCES

COPY / /SOURCES
RUN rm -r `find . -name .antlr` || true  
RUN make print_binaries
RUN make JavaGUI-antlr
RUN make java-jars
# RUN make print
# RUN make derived_objects
# RUN make libraries
# RUN make binaries
RUN make scripts

# # RUN CFLAGS="-O2" CPPFLAGS="-O2" LDFLAGS="-O2" \
# #     make -k -j 4 derived_objects
# # RUN CFLAGS="-O2" CPPFLAGS="-O2" LDFLAGS="-O2" \
# #     make -k -j 4

RUN make install

RUN dnf -y install terminus-fonts evince cups-pdf

RUN echo 'PATH=$PATH:/SOURCES/scripts' >> /etc/profile.d/greatspn.path.sh
# RUN echo 'unset HISTFILE' >> /etc/profile.d/disable.history.sh

ENV DISPLAY :0
RUN set +o history

# CMD [ "/SOURCE/greatspn_in_docker.sh" ]
CMD [ "/SOURCES/docker/fedora/launch_in_docker.sh" ]


# XSOCK=/tmp/.X11-unix
# XAUTH=/tmp/.docker.xauth
# xauth nlist :0 | sed -e 's/^..../ffff/' | xauth -f $XAUTH nmerge -
# docker run -ti -v $XSOCK:$XSOCK -v $XAUTH:$XAUTH -e XAUTHORITY=$XAUTH xeyes