@echo off

echo --------------------------------------------------------------
echo GreatSPN jpackage procedure for Windows.
echo --------------------------------------------------------------

set GUI_VERSION=1.6
echo Version %GUI_VERSION%

set JAVAGUI_MODULES=java.base,java.compiler,java.desktop,java.naming,java.prefs,java.sql
echo Modules %JAVAGUI_MODULES%

rem Builds the native application using the jpackage tool
echo "  [JPACKAGE MSI] " $@
jpackage --input JavaGUI/Editor/dist --name "GreatSPN Editor" ^
	--main-jar Editor.jar --main-class editor.Main  ^
	--add-modules %JAVAGUI_MODULES% ^
	--java-options "-enableassertions" ^
	--java-options "-splash:Contents/Java/splash.png" ^
	--app-version %GUI_VERSION% ^
	--copyright "University of Torino, Italy" ^
	--description "The GUI of the GreatSPN framework. Visit https://github.com/greatspn/SOURCES for more informations." ^
	--dest objects/JavaGUI/ ^
	--icon "JavaGUI/Additional/greatspn-win.ico" ^
	--file-associations JavaGUI/DISTRIB/PNPRO-win-FileAssoc.txt ^
	--win-dir-chooser --win-shortcut ^
	--win-menu --win-menu-group "GreatSPN" ^
	--type msi

rem --win-console 

echo "  [JPACKAGE APP-IMAGE] " $@
rmdir /s /q "objects/JavaGUI/GreatSPN Editor"
jpackage --input JavaGUI/Editor/dist --name "GreatSPN Editor" ^
	--main-jar Editor.jar --main-class editor.Main  ^
	--add-modules %JAVAGUI_MODULES% ^
	--java-options "-enableassertions" ^
	--java-options "-splash:Contents/Java/splash.png" ^
	--app-version %GUI_VERSION% ^
	--copyright "University of Torino, Italy" ^
	--description "The GUI of the GreatSPN framework. Visit https://github.com/greatspn/SOURCES for more informations." ^
	--dest objects/JavaGUI/ ^
	--icon "JavaGUI/Additional/greatspn-win.ico" ^
	--type app-image
