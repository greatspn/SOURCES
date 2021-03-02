@echo off

echo --------------------------------------------------------------
echo GreatSPN update procedure for Windows WSL.
echo --------------------------------------------------------------
pause

echo.
echo --------------------------------------------------------------
echo Updating sources from repository...
echo --------------------------------------------------------------
wsl -- bash -c 'cd ~/GreatSPN/SOURCES ; git stash ; git pull '

echo.
echo --------------------------------------------------------------
echo Building sources...
echo --------------------------------------------------------------
wsl -- bash -c 'cd ~/GreatSPN/SOURCES ; make all JavaGUI-jar'

echo.
echo --------------------------------------------------------------
echo Installing sources in the WSL subsystem.
echo The WSL password will be requested.
echo --------------------------------------------------------------
wsl -- bash -c 'cd ~/GreatSPN/SOURCES ; sudo make install'

echo.
echo --------------------------------------------------------------
echo Updating Windows GUI Jars...
echo --------------------------------------------------------------
wsl -- bash -c 'cp ~/GreatSPN/SOURCES/GreatSPN-Editor-*-Jar.zip ./'
wsl -- unzip -o GreatSPN-Editor-v*-Jar.zip

del GreatSPN-Editor-v*-Jar.zip

rem move /Y  bin\*  .\
rem mkdir lib
rem move /Y  bin\lib\* .\lib\
rem rmdir bin\lib
rem rmdir bin

echo.
echo --------------------------------------------------------------
echo All done.
echo --------------------------------------------------------------
pause
