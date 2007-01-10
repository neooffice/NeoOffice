Intructions for Building NeoOffice
----------------------------------

At this time, the NeoOffice build only supports building on Mac OS X 10.4 (Tiger). When compiled on Tiger, PowerPC builds should install and run on machines running Mac OS X 10.3.9 or later and Intel builds should install and run on machines running Mac OS X 10.4 or later.


Steps for building on Mac OS X 10.4 (Tiger) for both PowerPC and Intel
----------------------------------------------------------------------

1. Make sure that you have installed the following optional component from the Mac OS X 10.4 installation CD:

   X11

2. Make sure that you have downloaded and installed the following dependencies from http://connect.apple.com/ website:

   Xcode Tools v2.4.1

3. Make sure that you have set the compiler version to the correct version by executing the following command. Note that PowerPC machines require that gcc 3.3 be used. gcc 3.3 is required for the build to be runnable on Mac OS X 10.3.x machines:

   On PowerPC machines: sudo gcc_select 3.3
   On Intel machines: sudo gcc_select 4.0

4. Make sure that you have downloaded and installed the following Perl module from the http://www.cpan.org/modules/index.html website. Note that you will need to follow the instructions on the website to download and install the Archive::Zip module:

   Archive::Zip

5. Make sure that you have installed the "gcp" and "pkg-config" commands. You can download, compile, and install these commands by downloading, compiling, and installing the following packages from the http://www.darwinports.org/ website. Note that you will need to follow the instructions on the website for downloading, compiling, and installing the DarwinPorts "port" command. The "port" command is then used to do the downloading, compiling, and installation of the following packages:

   sudo /path/to/port/command install coreutils
   sudo /path/to/port/command install pkgconfig
   sudo /path/to/port/command install libIDL

6. Make sure that you have downloaded and installed the Mono Mac OS X framework:

   ftp://www.go-mono.com/archive/1.2.2.1/macos-10-universal/0/MonoFramework-1.2.2.1_0.macos10.novell.universal.dmg

7. Make sure that you have downloaded and installed the Subversion client and have the "svn" command in your PATH. Subversion binaries can be downloaded from here:

   http://subversion.tigris.org/project_packages.html

8. Start the build by invoking the following commands. Note that you should replace $NEO_HOME with absolute path of your workspace's "neojava" directory:

   cd $NEO_HOME
   make GNUCP=</absolute/path/of/your/gcp/command> LIBIDL_CONFIG=</absolute/path/of/your/libIDL-config-2/command> PKG_CONFIG=</absolute/path/of/your/pkg-config/command>

