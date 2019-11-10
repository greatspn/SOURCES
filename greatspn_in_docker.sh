
XSOCK=/tmp/.X11-unix
XAUTH=/tmp/.docker.xauth
xauth nlist :0 | sed -e 's/^..../ffff/' | xauth -f $XAUTH nmerge -
docker run -ti --rm \
    -v $XSOCK:$XSOCK \
    -v $XAUTH:$XAUTH \
    -e XAUTHORITY=$XAUTH \
    -e HOME=/SOURCES/usermodels \
    -e U_USER=$USER \
    -e U_UID=`id -u` \
    -e U_GID=`id -g` \
    -v `pwd`:/SOURCES/usermodels \
    -w /SOURCES/usermodels \
    gspn
