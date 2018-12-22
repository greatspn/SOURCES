#!/bin/bash

if [ ! -e /home/user/.greatspn-on-vbox ]; then
	echo 'This script should be run only on the VirtualBox image of GreatSPN.'
	exit 1
fi

B1=$(tput bold)
B0=$(tput sgr0)
C1=$(tput setaf 4)
C0=$(tput setaf 9)
LN="${B1}===============================================================================${B0}"
RUN=/home/user/GreatSPN/SOURCES/NSRC/VirtualMachineSupport/run_as_root.sh

#===================================================================
# Compile and install GreatSPN
#===================================================================
if [ "$1" != 'skip_make' ]
then
	echo && echo && echo
	echo $LN
	echo "${B1}Compiling GreatSPN from sources...${B0}"
	echo $LN && echo


	# For now, always clear the editor sources, to avoid having ant
	# not rebuilding everything and leaving some class uncompiled/stale
	${RUN} chown -R user:user /home/user/GreatSPN/SOURCES
	(rm -rf /home/user/GreatSPN/SOURCES/JavaGUI/Editor/build/classes/* )
	(touch /home/user/GreatSPN/SOURCES/JavaGUI/Editor/src/editor/domain/grammar/ExprLang.g4)
	(cd /home/user/GreatSPN/SOURCES && make JavaGUI-antlr)
	# Make everything else
	(cd /home/user/GreatSPN/SOURCES && make)
	echo
	if [ $? -eq 0 ]; then
		echo "${B1}GreatSPN has been recompiled. ${B0}"
	else
		echo "${B1}Error: Could not recompile GreatSPN. ${B0}"
		read -p "Press any key to quit..." -n1 -s
		exit 1
	fi

	echo && echo && echo
	echo $LN
	echo "${B1}Installing GreatSPN...${B0}"
	echo $LN && echo

	(cd /home/user/GreatSPN/SOURCES && ${RUN} make install)
	echo
	if [ $? -eq 0 ]; then
		echo "${B1}GreatSPN has been installed. ${B0}"
	else
		echo "${B1}Error: Could not install GreatSPN. ${B0}"
		read -p "Press any key to quit..." -n1 -s
		exit 1
	fi

	echo $LN
fi

#===================================================================
# Post-installation
#===================================================================
echo && echo && echo
echo $LN
echo "${B1}Running post-installation script...${B0}"
echo $LN && echo

# Update Deskop link to mount script
cat <<EOF > "/home/user/Desktop/Mount Shared Folder"
[Desktop Entry]
Encoding=UTF-8
Type=Application
Name=Mount Shared Folder
Name[en_US]=Mount Shared Folder
Icon=utilities-terminal-symbolic
Exec=/home/user/GreatSPN/SOURCES/NSRC/VirtualMachineSupport/mount-shared-folder.sh
Comment[en_US]=
Terminal=true
EOF
${RUN} chown user:user "/home/user/Desktop/Mount Shared Folder"

# Update Deskop link to upgrade GreatSPN
cat <<EOF > "/home/user/Desktop/Upgrade GreatSPN"
[Desktop Entry]
Encoding=UTF-8
Type=Application
Name=Upgrade GreatSPN
Name[en_US]=Upgrade GreatSPN
Icon=utilities-terminal-symbolic
Exec=/home/user/GreatSPN/SOURCES/NSRC/VirtualMachineSupport/upgrade-greatspn-from-svn.sh
Comment[en_US]=
Terminal=true
EOF
${RUN} chown user:user "/home/user/Desktop/Upgrade GreatSPN"

# Link to the old GreatSPN interface
cat <<EOF > "/home/user/launch-old-greatspn"
#! /bin/sh
export PATH=$PATH:/usr/local/GreatSPN/scripts
greatspn
EOF
${RUN} chown user:user "/home/user/launch-old-greatspn"
chmod 755 "/home/user/launch-old-greatspn"

cat <<EOF > "/home/user/Desktop/GreatSPN-OLD"
[Desktop Entry]
Encoding=UTF-8
Type=Application
Name=GreatSPN-OLD
Name[en_US]=GreatSPN-OLD
Icon=utilities-terminal-symbolic
Exec=/home/user/launch-old-greatspn
Comment[en_US]=
Terminal=true
EOF
${RUN} chown user:user "/home/user/Desktop/GreatSPN-OLD"

# Link to the new GreatSPN interface
# cat <<EOF > "/home/user/Desktop/New GreatSPN Editor"
# EOF
# ${RUN} chown user:user "/home/user/Desktop/New GreatSPN Editor"


#-----------------------------------------------------------------
# Documents
cat <<EOF > "/home/user/Desktop/GreatSPN VM image README"
=======================================================================================
This virtual machine contains a pre-installed version of the GreatSPN framework.
It is designed to be run under VirtualBox or VMWare, to ease the software portability.
=======================================================================================

UPGRADE GREATSPN FROM SOURCES USING THE GITHUB REPOSITORY:

Just double-click on the Upgrade-GreatSPN script on the Desktop.
The script downloads the latest sources of GreatSPN and builds it.

---------------------------------------------------------------------------------------
INTEGRATION WITH VIRTUALBOX:

The VirtualBox software integration is used to share directories between
the VM and the external environment (Windows, Linux, MacOSX, ...).

In order to work properly, the VM *must* install the latest version of the
"Guest Additions" software. If you experience problems in the VirtualBox integration
(shared directory does not work, copy/paste between the VM and the external environment
is not working, etc...), try reinstalling the Guest Additions.

To reinstall the "Guest Additions", do:
 1. Go to the VirtualBox menu and select Devices->Insert Guest Additions CD image...
 2. Open a terminal and type:
       su
    Write the password 'fedora' when asked. Then type:
       /run/media/user/VBOXADDITIONS_<version of VirtualBox>/VBoxLinuxAdditions.run

Installation may take several minutes. Check that everything ends correctly.

---------------------------------------------------------------------------------------
EOF
${RUN} chown user:user "/home/user/Desktop/GreatSPN VM image README"

#-----------------------------------------------------------------
cat <<EOF > "/home/user/Desktop/HowTo mount shared folder"
These are the instructions needed to add a shared folder using the VirtualBox
or VMWare integration preferences.

FOR VIRTUALBOX
-----------------------
Configure VirtualBox shared folders by adding your shared folder in:
   Machine > Settings > Shared Folders > Machine Folders

Share name *MUST* be:  /media/shared

Auto-mount feature of VirtualBox does not work properly and it is
disabled. To mount the shared folder, double click on the
'Mount Shared Folder' icon on the desktop.

FOR VMWARE
-----------------------
You can drag&drop files using the vm-ware integration.
EOF
${RUN} chown user:user "/home/user/Desktop/HowTo mount shared folder"

#-----------------------------------------------------------------
cat <<EOF > "/home/user/Desktop/HowTo get root access"
The root password is: fedora
EOF
${RUN} chown user:user "/home/user/Desktop/HowTo get root access"

#-----------------------------------------------------------------
if [[ ! -e /home/user/Desktop/SharedFolder ]] ; then
	ln -s /media/vbox_Dati /home/user/Desktop/SharedFolder
fi

# Remove old upgrade script
# ${RUN} rm -f "/home/user/upgrade-greatspn-from-svn"
# ${RUN} rm -f "/home/user/Desktop/HowTo update GreatSPN"

# Update YUM cache
# ${RUN} yum makecache fast

# Install CUPS/PDF packages
# rpm -q cups-pdf
# if [ $? == 1 ]; then
# 	${RUN} yum install cups-pdf.i686
# fi



#===================================================================
# Install script end.
#===================================================================
echo && echo && echo
echo "${B1}Upgrade procedure completed successfully...${B0}"
echo
read -p "Press any key to quit..." -n1 -s


