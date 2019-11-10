#!/bin/bash
set -x
set +o history
groupadd -g $U_GID $U_USER
useradd -u $U_UID -g $U_USER -d /SOURCES/usermodels $U_USER
su $U_USER -c "java -Duser.home=/SOURCES/usermodels -cp /SOURCES/JavaGUI/Editor/dist/lib/swing-layout-1.0.4.jar:/SOURCES/JavaGUI/Editor/dist/lib/AbsoluteLayout.jar:/SOURCES/JavaGUI/Editor/dist/lib/antlr-runtime-4.2.1.jar:/SOURCES/JavaGUI/Editor/dist/lib/jlatexmath-1.0.4.jar:/SOURCES/JavaGUI/Editor/dist/Editor.jar editor.Main"
