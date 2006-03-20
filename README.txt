Intructions for Building NeoOffice
----------------------------------

Mac OS X 10.3 (Panther)
-----------------------

1. Make sure that you have installed the following package from CD 3 of the Mac OS X 10.3 installation CD set:

   Packages/X11User.pkg

2. Make sure that you have installed the following package installed from the Mac OS X Xcode Tools CD:

   Developer.mpkg
   Packages/X11SDK.pkg

3. Make sure that you have downloaded and installed the following dependencies from http://connect.apple.com/ website:

   Xcode Tools v1.5
   November 2004 GCC 3.3 Updater
   Java 1.4.1 Developer Tools Update

4. Make sure that you have set the compiler version to 3.3 by executing the following command:

   sudo gcc_select 3.3

5. Make sure that you have downloaded and installed the following Perl module from the http://www.cpan.org/modules/index.html website. Note that you will need to follow the instructions on the website to download and install the Archive::Zip module:

   Archive::Zip

6. Make sure that you have installed the "gcp" and "pkg-config" commands. You can download, compile, and install these commands by downloading, compiling, and installing the following packages from the http://www.darwinports.org/ website. Note that you will need to follow the instructions on the website for downloading, compiling, and installing the DarwinPorts "port" command. The "port" commis then used to do the downloading, compiling, and installation of the following packages:

   coreutils
   pkgconfig

7. Make sure that the /Applications/Utilities/X11 application is running.

8. Start the build by invoking the following commands. Note that you should replace $NEO_HOME with absolute path of your workspace's "neojava" directory:

   cd $NEO_HOME
   make GNUCP=</absolute/path/of/your/gcp/command> PKG_CONFIG=</absolute/path/of/your/pkg-config/command>

TODO: Currently, the build will only build through the build.neo_sysui_patch. Updating of the NeoOffice vcl custom code and installer still needs to be done.


Mac OS X 10.4 (Tiger)
-----------------------

1. Make sure that you have installed the following optional component from the Mac OS X 10.4 installation CD:

   X11

2. Make sure that you have installed the following package installed from the Mac OS X 10.4 installation CD:

   Xcode Tools/XcodeTools.mpkg
   Xcode Tools/Packages/X11SDK.pkg

3. Make sure that you have downloaded and installed the following dependencies from http://connect.apple.com/ website:

   Xcode Tools v2.2.1

4. Make sure that you have set the compiler version to 4.0.1 by executing the following command:

   sudo gcc_select 4.0

5. Make sure that you have downloaded and installed the following Perl module from the http://www.cpan.org/modules/index.html website. Note that you will need to follow the instructions on the website to download and install the Archive::Zip module:

   Archive::Zip

6. Make sure that you have installed the "gcp" and "pkg-config" commands. You can download, compile, and install these commands by downloading, compiling, and installing the following packages from the http://www.darwinports.org/ website. Note that you will need to follow the instructions on the website for downloading, compiling, and installing the DarwinPorts "port" command. The "port" commis then used to do the downloading, compiling, and installation of the following packages:

   coreutils
   pkgconfig

7. Make sure that the /Applications/Utilities/X11 application is running.

8. Start the build by invoking the following commands. Note that you should replace $NEO_HOME with absolute path of your workspace's "neojava" directory:

   cd $NEO_HOME
   make GNUCP=</absolute/path/of/your/gcp/command> PKG_CONFIG=</absolute/path/of/your/pkg-config/command>

