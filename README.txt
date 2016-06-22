Intructions for Building NeoOffice
----------------------------------


Steps for building on 10.9 Yosmite for Intel
--------------------------------------------

At this time, NeoOffice will only build on Mac OS X 10.10 Yosemite.

1. Make sure that you have downloaded and installed the following dependencies from http://developer.apple.com/ website:

   Xcode v7.2.1
   Command Line Tools for Xcode v7.2

2. Download and install Apple's Java 1.6 Development Kit (JDK) from the following URL. Note: the build will ignore Oracle's JDKs:

   http://support.apple.com/kb/DL1572

3. Download and install Oracle's Java 1.8 Development Kit (JDK) from the following URL. Note: the build will ignore Oracle's JDKs. Oracle JDKs are only used when running NeoOffice:

   http://www.oracle.com/technetwork/java/javase/downloads/jdk8-downloads-2133151.html

4. Install the following Mac Ports packages by downloading, compiling, and installing the following packages from the http://www.macports.org/ website. Note that you will need download and install the latest MacPorts Yosemite package to install the MacPorts "port" command. The "port" command is then used to do the downloading, compiling, and installation of the following packages:

   sudo /opt/local/bin/port install coreutils -x11
   sudo /opt/local/bin/port install pkgconfig -x11
   sudo /opt/local/bin/port install libIDL -x11
   sudo /opt/local/bin/port install gperf -x11
   sudo /opt/local/bin/port install flex -x11
   sudo /opt/local/bin/port install wget -x11
   sudo /opt/local/bin/port install gnutar -x11
   sudo /opt/local/bin/port install poppler -x11
   sudo /opt/local/bin/port install libwpd@0.9.9 -x11
   sudo /opt/local/bin/port install cvs -x11

   After running the above command, add "/opt/local/bin" to the end of your shell's PATH environment variable so that the build can all of the commands installed by /opt/local/bin/port command in the previous step.

5. Installed the Perl Archive::Zip module using the following command:

   cpan Archive::Zip

6. Start the build by invoking the following commands. Note that you should replace $NEO_HOME with absolute path of your workspace's "neojava" directory:

   cd $NEO_HOME
   make
