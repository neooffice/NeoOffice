Intructions for Building NeoOffice
----------------------------------


Steps for building on 10.5 Leopard for Intel
----------------------------------------------

At this time, the NeoOffice build only supports building on Mac OS X 10.5 Leopard for Intel only. Building on Mac OS X 10.6 and higher will not work, and as a result isn't supported.

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


Steps for building on Windows XP or Vista
-----------------------------------------

Although building on Windows will not work, in theory the Go-oo portion of the build supports building on Windows. Although we have not successfully build the Go-oo code using the NeoOffice makefile, we have listed the dependencies that we know of at the end of this file for anyone who may be interested in porting NeoOffice to Windows.

1. Download and install Cygwin from http://www.cygwin.com/. When installing, be sure to enable installation of the following tools that Cygwin does not install by default:

   Archive :: cabextract
   Devel :: cvs
   Devel :: flex
   Devel :: gcc-g++
   Devel :: make
   Devel :: pkg-config
   Gnome :: libIDL_0
   Editors :: vim or Emacs
   Net :: openssh
   Shells :: tcsh
   Utils :: patch
   Web :: wget

2. Download and install the "vcsetup.exe" installer for Visual C++ 9.0 Express Edition from the following URL. Note: newer versions of this software will not work. Also you will need to install this software using the "Run as Administrator" option:

   http://www.microsoft.com/downloads/info.aspx?na=41&srcfamilyid=a22341ee-21db-43aa-8431-40be78461ee0&srcdisplaylang=en&u=http%3a%2f%2fdownload.microsoft.com%2fdownload%2fd%2fc%2f3%2fdc3439e7-5533-4f4c-9ba0-8577685b6e7e%2fvcsetup.exe

3. Download and install the DirectX 9 SDK from the following URL. Note: newer versions of this SDK will not work. Also you will need to install this software using the "Run as Administrator" option:

   http://download.microsoft.com/download/5/8/2/58223f79-689d-47ae-bdd0-056116ee8d16/DXSDK_Nov08.exe

4. Download and install the latest Java Development Kit (JDK) from the following URL:

   http://www.oracle.com/technetwork/java/javase/downloads

5. Download and install the Visual C++ 2003 Runtime and the .NET 1.1 SDK from the following URL. These are needed to use the pre-built Mozilla libraries provided by OpenOffice.org:

   http://www.microsoft.com/downloads/en/details.aspx?familyid=262d25e3-f589-4842-8157-034d1e7cf3a3&displaylang=en
   http://www.microsoft.com/downloads/en/details.aspx?FamilyID=9b3a2ca6-3647-4070-9f41-a333c6b9181d

6. Download and run the installer to unpack the GDI+ gdiplus.dll file. Note the folder that you had the installer it to as you will need to copy the asms\10\msft\windows\gdiplus\gdiplus.dll file if the build fails because it cannot find gdiplus.dll:

   http://www.microsoft.com/download/en/details.aspx?id=18909

7. Make the /usr/bin/awk symlink a real file by executing the following command:

   rm /usr/bin/awk ; cp /usr/bin/gawk.exe /usr/bin/awk.exe
