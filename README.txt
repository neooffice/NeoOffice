Intructions for Building NeoOffice
----------------------------------

At this time, the NeoOffice build only supports building on Mac OS X 10.5 (Leopard).

WARNING: Building under Mac OS X 10.6 and higher will not work, and as a result isn't supported.


Steps for building on 10.5 (Leopard) for Intel
----------------------------------------------

1. Make sure that you have downloaded and installed the following dependencies from http://connect.apple.com/ website:

   Xcode Tools v3.1.4

   Important: you will need to select the X11SDK package during installation as this package is required to build OpenOffice.org moz module.
   
2. Make sure that you have downloaded and installed the following Perl module from the http://www.cpan.org/modules/index.html website. Note that you will need to follow the instructions on the website to download and install the Archive::Zip module:

   Archive::Zip

3. Make sure that you have installed the "gcp" and "pkg-config" commands. You can download, compile, and install these commands by downloading, compiling, and installing the following packages from the http://www.macports.org/ website. Note that you will need to follow the instructions on the website for downloading, compiling, and installing the DarwinPorts "port" command. The "port" command is then used to do the downloading, compiling, and installation of the following packages:

   sudo /path/to/port/command install coreutils
   sudo /path/to/port/command install pkgconfig
   sudo /path/to/port/command install libIDL
   sudo /path/to/port/command install gperf
   sudo /path/to/port/command install flex
   sudo /path/to/port/command install wget

4. Make sure that you have downloaded and installed the Subversion client and have the "svn" command in your PATH. Subversion binaries can be downloaded from here:

   http://subversion.tigris.org/project_packages.html

5. Start the build by invoking the following commands. Note that you should replace $NEO_HOME with absolute path of your workspace's "neojava" directory:

   cd $NEO_HOME
   make GNUCP=</absolute/path/of/your/gcp/command> LIBIDL_CONFIG=</absolute/path/of/your/libIDL-config-2/command> PKG_CONFIG=</absolute/path/of/your/pkg-config/command>

