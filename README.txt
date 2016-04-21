Intructions for Building NeoOffice
----------------------------------


Steps for building on 10.9 Mavericks for Intel
--------------------------------------------------

At this time, NeoOffice will only build on Mac OS X 10.9 Mavericks.

1. Make sure that you have downloaded and installed the following dependencies from http://developer.apple.com/ website:

   Xcode Tools v6.2
   Command Line Tools for Xcode v6.2

2. Download and install the Apple's Java 1.6 Development Kit (JDK) from the following URL. Note: the build will ignore Oracle's JDKs:

   http://support.apple.com/kb/DL1572

3. Make sure that you have installed the "gcp" and "pkg-config" commands. You can download, compile, and install these commands by downloading, compiling, and installing the following packages from the http://www.macports.org/ website. Note that you will need download and install the latest MacPorts Mountain Lion package to install MacPorts "port" command. The "port" command is then used to do the downloading, compiling, and installation of the following packages:

   sudo /opt/local/bin/port install coreutils -x11
   sudo /opt/local/bin/port install pkgconfig -x11
   sudo /opt/local/bin/port install libIDL -x11
   sudo /opt/local/bin/port install gperf -x11
   sudo /opt/local/bin/port install flex -x11
   sudo /opt/local/bin/port install wget -x11
   sudo /opt/local/bin/port install gnutar -x11
   sudo /opt/local/bin/port install poppler -x11
   sudo /opt/local/bin/port install libwpd -x11

   After running the above command, add "/opt/local/bin" to the end of your shell's PATH environment variable so that the build can find the "autoconf" and other commands.

4. Make sure that you have downloaded and installed the following Perl module from the http://www.cpan.org/modules/index.html website. Note that you will need to follow the instructions on the website to download and install the Archive::Zip module:

   Archive::Zip

5. Start the build by invoking the following commands. Note that you should replace $NEO_HOME with absolute path of your workspace's "neojava" directory:

   cd $NEO_HOME
   make GNUCP=</absolute/path/of/your/gcp/command> LIBIDL_CONFIG=</absolute/path/of/your/libIDL-config-2/command> PKG_CONFIG=</absolute/path/of/your/pkg-config/command>


Steps for building on Windows XP or Vista
-----------------------------------------

Although building on Windows will not work, in theory the Go-oo portion of the build supports building on Windows. Although we have not successfully built the Go-oo code using the NeoOffice makefile, we have listed the dependencies that we know of at the end of this file for anyone who may be interested in porting NeoOffice to Windows.

1. Download and install Cygwin from http://www.cygwin.com/. When installing, be sure to enable installation of the following tools that Cygwin does not install by default:

   Archive :: cabextract
   Archive :: unzip
   Archive :: zip
   Devel :: cvs
   Devel :: flex
   Devel :: gcc-g++
   Devel :: gperf
   Devel :: make
   Devel :: pkg-config
   Gnome :: libIDL_0
   Editors :: vim or Emacs
   Net :: openssh
   Shells :: tcsh
   Utils :: patch
   Web :: wget

2. Download and install the Visual C++ 2008 Express Edition with SP1 from the following URL. Note: newer versions of this software will not work. Also you will need to install this software using the "Run as Administrator" option:

   http://www.microsoft.com/visualstudio/en-us/products/2008-editions/express

3. Download and install the DirectX 9 SDK from the following URL. Note: newer versions of this SDK will not work. Also you will need to install this software using the "Run as Administrator" option:

   http://download.microsoft.com/download/5/8/2/58223f79-689d-47ae-bdd0-056116ee8d16/DXSDK_Nov08.exe

4. Download and install the Java 1.6 Development Kit (JDK) from the following URL. Note: do not use the Java 1.7 JDK:

   http://www.oracle.com/technetwork/java/javase/downloads

5. Download and install the Visual C++ 2003 Runtime and the .NET 1.1 SDK from the following URL. These are needed to use the pre-built Mozilla libraries provided by OpenOffice.org:

   http://www.microsoft.com/downloads/en/details.aspx?familyid=262d25e3-f589-4842-8157-034d1e7cf3a3&displaylang=en
   http://www.microsoft.com/downloads/en/details.aspx?FamilyID=9b3a2ca6-3647-4070-9f41-a333c6b9181d

6. Download and install the Windows SDK for Windows Server 2008 and .NET Framework 3.5 from the following URL. These are needed to build the native installer:

   http://www.microsoft.com/download/en/details.aspx?displaylang=en&id=11310

7. Download and run the installer to unpack the GDI+ gdiplus.dll file. Note the folder that you had the installer it to as you will need to copy the asms\10\msft\windows\gdiplus\gdiplus.dll file if the build fails because it cannot find gdiplus.dll:

   http://www.microsoft.com/download/en/details.aspx?id=18909

8. Make the /usr/bin/awk symlink a real file by executing the following command:

   rm /usr/bin/awk ; cp /usr/bin/gawk.exe /usr/bin/awk.exe

9. Make sure that you have downloaded and installed the following Perl module from the http://www.cpan.org/modules/index.html website. Note that you will need to follow the instructions on the website to download and install the following modules:

   Archive::Zip
   URI
   XML::Parser

10. Download and install NSIS from the following URL:

   http://nsis.sourceforge.net/Download

    After installing NSIS, go to the following URL and download the latest "Vietnamese.nlf" and "Vietnamese.nsh" files. Once you have downloaded them, move or copy them into your "C:\Program Files\NSIS\Contrib\Language files\" folder:

   http://nsis.svn.sourceforge.net/viewvc/nsis/NSIS/trunk/Contrib/Language%20files/
