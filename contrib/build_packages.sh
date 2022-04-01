#!/bin/bash

if [ ! -e bin/WNRG ]; then
	echo "Call from the main SOURCE directory, with GreatSPN fully built"
	exit 1
fi

GREATSPN_APP_VERSION_MAJOR=3
GREATSPN_APP_VERSION_MINOR=1
GREATSPN_APP_VERSION=v${GREATSPN_APP_VERSION_MAJOR}r${GREATSPN_APP_VERSION_MINOR}
GREATSPN_APP_VERSION_FULLNUMBER=`expr ${GREATSPN_APP_VERSION_MAJOR}00 + ${GREATSPN_APP_VERSION_MINOR}`
GREATSPN_APPNAME=GreatSPN
GREATSPN_APPNAME_ID=greatspn-${GREATSPN_APP_VERSION}

APPIMAGE_ROOTDIR=objects/AppImage

echo "Building ${GREATSPN_APPNAME} version ${GREATSPN_APP_VERSION_FULLNUMBER}"

# jdeps --ignore-missing-deps --print-module-deps \
# 			--class-path ${OBJDIR}/JavaGUI/bin/lib/*.jar \
# 			${OBJDIR}/JavaGUI/bin/Editor.jar
JAVAGUI_MODULES=java.base,java.compiler,java.desktop,java.naming,java.prefs,java.sql

# Determine OS-dependent parameters.
case "$OSTYPE" in
#----------------------------------------------------------
darwin*)
echo "OSX"
JPACKAGE_OPTIONS="
	--java-options \"-Djava.library.path=Contents/Java/\" 
	--java-options \"-Dapple.laf.useScreenMenuBar=true\" 
	--java-options \"-Dcom.apple.macos.useScreenMenuBar=true\" 
	--icon JavaGUI/AdditionalV3/GreatSPN.icns
	--mac-package-identifier \"${GREATSPN_APPNAME_ID}\"
	"
APPIMAGE_DIR=${APPIMAGE_ROOTDIR}/${GREATSPN_APPNAME}.app
;;

#----------------------------------------------------------
linux*)   
echo "LINUX"
JPACKAGE_OPTIONS="
	--icon JavaGUI/AdditionalV3/GreatSPN.png
	"
APPIMAGE_DIR=${APPIMAGE_ROOTDIR}/${GREATSPN_APPNAME}
;;

#----------------------------------------------------------
*)        
echo "unknown: $OSTYPE"
exit 1 
;;
esac

echo ${JPACKAGE_OPTIONS}


###########################################################
echo 'FIRST STAGE: Building the app-image'

# rm -rf ${APPIMAGE_ROOTDIR}
# mkdir -p ${APPIMAGE_ROOTDIR}
# jpackage \
# 	--type app-image \
# 	--input JavaGUI/Editor/dist \
# 	--name ${GREATSPN_APPNAME} \
# 	--main-jar Editor.jar \
# 	--main-class editor.Main \
# 	--add-modules ${JAVAGUI_MODULES} \
# 	--java-options -enableassertions \
# 	--java-options '"-splash:\$APPDIR/GreatSPN Editor.app/Contents/Java/splash.png"' \
# 	--app-version "${GREATSPN_APP_VERSION_FULLNUMBER}" \
# 	--copyright "University of Torino, Italy" \
# 	--description "The GreatSPN framework. Visit https://github.com/greatspn/SOURCES for more informations." \
# 	--dest ${APPIMAGE_ROOTDIR} \
# 	${JPACKAGE_OPTIONS}
# RET=$? ; if [ ${RET} -eq 1 ]; then exit ${RET} ; fi

###########################################################
echo 'SECOND STAGE: Creating the Portable GreatSPN distribution'

mkdir -p ${APPIMAGE_DIR}/lib/app/portable_greatspn
mkdir -p ${APPIMAGE_DIR}/lib/app/portable_greatspn/bin
mkdir -p ${APPIMAGE_DIR}/lib/app/portable_greatspn/lib

cp bin/* ${APPIMAGE_DIR}/lib/app/portable_greatspn/bin/
rm -f ${APPIMAGE_DIR}/lib/app/portable_greatspn/bin/DSPN-Tool-Debug # not needed

case "$OSTYPE" in
#----------------------------------------------------------
darwin*)
cp  ../meddly/src/.libs/libmeddly.dylib \
    ../spot-2.9.6/spot/.libs/libspot.dylib \
    ../spot-2.9.6/buddy/src/.libs/libbddx.dylib \
    ${APPIMAGE_DIR}/lib/app/portable_greatspn/lib/
;;

#----------------------------------------------------------
linux*)   
cp  ../meddly/src/.libs/libmeddly.so.0 \
    ../spot-2.9.6/spot/.libs/libspot.so.0 \
    ../spot-2.9.6/buddy/src/.libs/libbddx.so.0 \
    ${APPIMAGE_DIR}/lib/app/portable_greatspn/lib/
;;

#----------------------------------------------------------
*)        
echo "unknown: $OSTYPE"
exit 1 
;;
esac

###########################################################
echo 'THIRD STAGE: Creating installable packages'


case "$OSTYPE" in
#----------------------------------------------------------
darwin*)
echo "OSX"
# Make the DMG package
jpackage \
	--type dmg \
	--app-image ${APPIMAGE_DIR} \
	--name ${GREATSPN_APPNAME} \
	--app-version ${GREATSPN_APP_VERSION_FULLNUMBER} \
	--file-associations JavaGUI/AdditionalV3/PNPRO-macos-FileAssoc.txt \
	--resource-dir JavaGUI/AdditionalV3 \
	--mac-package-name "${GREATSPN_APPNAME_VER}" \
	--mac-package-identifier "${GREATSPN_APPNAME_ID}"
;;

#----------------------------------------------------------
linux*)   
echo "LINUX"
# Make installable DEB package
# Application icon is taken from the resource-dir directory
jpackage \
	--type deb \
	--app-image ${APPIMAGE_DIR} \
	--name ${GREATSPN_APPNAME} \
	--app-version ${GREATSPN_APP_VERSION_FULLNUMBER} \
	--file-associations JavaGUI/AdditionalV3/PNPRO-linux-FileAssoc.txt \
	--resource-dir JavaGUI/AdditionalV3 \
	--linux-deb-maintainer "Elvio Amparore <amparore@di.unito.it>" \
	--linux-menu-group "Science" \
	--linux-app-release "full" \
	--linux-package-deps "libsuitesparse-dev, libgmpxx4ldbl, libgmp-dev, graphviz"

# Make installable RPM package
jpackage \
	--type rpm \
	--app-image ${APPIMAGE_DIR} \
	--name ${GREATSPN_APPNAME} \
	--app-version ${GREATSPN_APP_VERSION_FULLNUMBER} \
	--file-associations JavaGUI/AdditionalV3/PNPRO-linux-FileAssoc.txt \
	--resource-dir JavaGUI/AdditionalV3 \
	--linux-menu-group "Science" \
	--linux-app-release "full" \
	--linux-package-deps "gmp-c++, gmp, suitesparse, graphviz, lpsolve"

;;

#----------------------------------------------------------
*)        
echo "unknown: $OSTYPE"
exit 1 
;;
esac





###########################################################
echo 'FOURTH STAGE: Cleaning temporary files'

# rm -rf ${APPIMAGE_ROOTDIR}

echo "Done."




















