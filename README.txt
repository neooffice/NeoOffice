Intructions for Building NeoOffice
----------------------------------

At this time, the NeoOffice build only supports building on Mac OS X 10.4 (Tiger). When compiled on Tiger, PowerPC builds should install and run on machines running Mac OS X 10.3.9 or later and Intel builds should install and run on machines running Mac OS X 10.4 or later.


Steps for building on Mac OS X 10.4 (Tiger) for both PowerPC and Intel
----------------------------------------------------------------------

1. Make sure that you have downloaded and installed the following dependencies from http://connect.apple.com/ website:

   Xcode Tools v2.4.1

2. Make sure that you have set the compiler version to the correct version by executing the following command:

   sudo gcc_select 4.0

3. Make sure that you have downloaded and installed the following Perl module from the http://www.cpan.org/modules/index.html website. Note that you will need to follow the instructions on the website to download and install the Archive::Zip module:

   Archive::Zip

4. Make sure that you have installed the "gcp" and "pkg-config" commands. You can download, compile, and install these commands by downloading, compiling, and installing the following packages from the http://www.darwinports.org/ website. Note that you will need to follow the instructions on the website for downloading, compiling, and installing the DarwinPorts "port" command. The "port" command is then used to do the downloading, compiling, and installation of the following packages:

   sudo /path/to/port/command install coreutils
   sudo /path/to/port/command install pkgconfig
   sudo /path/to/port/command install libIDL
   sudo /path/to/port/command install gperf
   sudo /path/to/port/command install flex

5. Make sure that you have downloaded and installed the Mono Mac OS X framework:

   ftp://www.go-mono.com/archive/1.2.4/macos-10-universal/4/MonoFramework-1.9.1_3.macos10.novell.universal.dmg

6. Make sure that you have downloaded and installed the Subversion client and have the "svn" command in your PATH. Subversion binaries can be downloaded from here:

   http://subversion.tigris.org/project_packages.html

7. Start the build by invoking the following commands. Note that you should replace $NEO_HOME with absolute path of your workspace's "neojava" directory:

   cd $NEO_HOME
   make GNUCP=</absolute/path/of/your/gcp/command> LIBIDL_CONFIG=</absolute/path/of/your/libIDL-config-2/command> PKG_CONFIG=</absolute/path/of/your/pkg-config/command>

