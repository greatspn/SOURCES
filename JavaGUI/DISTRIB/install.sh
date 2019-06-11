#!/bin/bash
APP="pnpro-editor"
LONGNAME="New GreatSPN Editor"
EXT="pnpro"
COMMENT="Petri Net Project"

# is running as root?
if [[ $EUID -eq 0 ]]; then
    PREFIX=${INSTALLDIR}
    if [ -z $PREFIX ]; then
        PREFIX=/usr/local/GreatSPN/
    fi
	if [ -z $XDG_DIR ]; then
    	XDG_DIR=/usr/local/share
	fi
else
    PREFIX=${HOME}
	if [ -z $XDG_DIR ]; then
		XDG_DIR=~/.local/share
	fi
fi

# idiomatic parameter and option handling in sh
while test $# -gt 0
do
    case "$1" in
        -h) 
			echo "This is the Linux installer of the new GreatSPN Java GUI."
			exit
            ;;
        -silent) 
            SILENT=1
            ;;
        -*) 
            echo "Unknown argument $1"
            exit
            ;;
        *) 
			echo "Unknown argument $1"
		    exit
            ;;
    esac
    shift
done

# Where will be installed
#echo PREFIX = $PREFIX
APP_PATH=${PREFIX}/bin

# Test proper installation invokation
if [ ! -f bin/Editor.jar ]; then
	echo "Could not find installation files."
	echo "Ensure that the current working directory is the one where this script is located."
	echo "Quit without installing."
	exit
fi

# Ask the user
if [ -z "${SILENT}" ]; then
	echo "The GreatSPN Editor will be installed in ${APP_PATH}"
	echo
	read -p "Press [Enter] key to start."
fi

# Create directories if missing
mkdir -p ${XDG_DIR}/mime/packages
mkdir -p ${XDG_DIR}/applications
mkdir -p ${XDG_DIR}/pixmaps
mkdir -p $APP_PATH

# Install the application
# echo "Copying application data..."
# cp -R bin/*  ${APP_PATH}/

# Install the icons
xdg-icon-resource install --novendor --size 48 $APP_PATH/$APP.png $APP
xdg-icon-resource install --novendor --size 48 $APP_PATH/application-x-$APP.png application-x-$APP

# Create mime xml 
echo "<?xml version=\"1.0\" encoding=\"UTF-8\"?>
<mime-info xmlns=\"http://www.freedesktop.org/standards/shared-mime-info\">
    <mime-type type=\"application/x-$APP\">
        <comment>$COMMENT</comment>
        <icon name=\"application-x-$APP\"/>
        <glob pattern=\"*.$EXT\"/>
    </mime-type>
</mime-info>" > ${XDG_DIR}/mime/packages/application-x-$APP.xml

echo "Creating desktop entry..."

# Create application desktop
echo "[Desktop Entry]
Name=$LONGNAME
Exec=java -Dswing.defaultlaf=com.sun.java.swing.plaf.gtk.GTKLookAndFeel -splash:$APP_PATH/lib/splash.png -jar $APP_PATH/Editor.jar %F
MimeType=application/x-$APP
Icon=$APP
Path=$APP_PATH
Terminal=false
Type=Application
Categories=
Comment=The New GreatSPN Editor.
"> ${XDG_DIR}/applications/$APP.desktop

echo "Installing menu entry..."
xdg-desktop-menu install --novendor --mode system ${XDG_DIR}/applications/$APP.desktop

#rm -f ~/Desktop/New\ GreatSPN\ Editor.desktop
#ln ${XDG_DIR}/applications/$APP.desktop ~/Desktop/New\ GreatSPN\ Editor.desktop

echo "Updating application/mime databases..."

# update databases for both application and mime
update-desktop-database ${XDG_DIR}/applications
update-mime-database    ${XDG_DIR}/mime

# copy associated icons to pixmaps
# cp $APP_PATH/$APP.png                ${XDG_DIR}/pixmaps
# cp $APP_PATH/application-x-$APP.png  ${XDG_DIR}/pixmaps


