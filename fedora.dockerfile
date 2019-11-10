FROM fedora

RUN dnf -y install git

RUN dnf -y install \
    gcc \
    gcc-c++ \
    gmp-devel \
    gmp-c++ \
    gmp \
    boost-devel \
    flex-devel \
    ant \
    glib2-devel \
    patch \
    python \
    libxml-devel \
    glpk-devel \
    lpsolve-devel \
    autoconf \
    automake \
    libtool \
    zip \
    flex git \
    byacc \
    time \
    graphviz \
    suitesparse-devel \
    motif-devel \
    make \
    libxml++-devel \
    glibmm24-devel \
    lpsolve-devel 

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

COPY / /SOURCES
RUN cd /SOURCES; rm -r `find . -name .antlr` || true  
RUN cd /SOURCES; make print_binaries
RUN cd /SOURCES; make JavaGUI-antlr
RUN cd /SOURCES; make java-jars
# RUN cd /SOURCES; make print
# RUN cd /SOURCES; make derived_objects
# RUN cd /SOURCES; make libraries
# RUN cd /SOURCES; make binaries
RUN cd /SOURCES; make scripts

# # RUN cd /SOURCES; CFLAGS="-O2" CPPFLAGS="-O2" LDFLAGS="-O2" \
# #     make -k -j 4 derived_objects
# # RUN cd /SOURCES; CFLAGS="-O2" CPPFLAGS="-O2" LDFLAGS="-O2" \
# #     make -k -j 4

# RUN cd /SOURCES; make install

RUN echo 'PATH=$PATH:/SOURCES/scripts' >> /etc/profile.d/greatspn.path.sh
# RUN echo 'unset HISTFILE' >> /etc/profile.d/disable.history.sh

ENV DISPLAY :0
RUN set +o history

# CMD [ "/SOURCE/greatspn_in_docker.sh" ]
CMD [ "/SOURCES/launch_in_docker.sh" ]


# XSOCK=/tmp/.X11-unix
# XAUTH=/tmp/.docker.xauth
# xauth nlist :0 | sed -e 's/^..../ffff/' | xauth -f $XAUTH nmerge -
# docker run -ti -v $XSOCK:$XSOCK -v $XAUTH:$XAUTH -e XAUTHORITY=$XAUTH xeyes