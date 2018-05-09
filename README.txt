Intructions for Building NeoOffice
----------------------------------


Steps for building on macOS 10.12 Sierra for Intel
--------------------------------------------------

At this time, NeoOffice will only build on macOS 10.12 Sierra.

1. Make sure that you have downloaded and installed the following dependencies from http://developer.apple.com/ website:

   Xcode v8.3.3
   Command Line Tools for Xcode v8.3.2

2. Download and install Oracle's Java 1.8 Development Kit (JDK) from the following URL. Note: the build will ignore Oracle's JDKs. Oracle JDKs are only used when running NeoOffice:

   http://www.neooffice.org/neojava/javadownload.php

   Important: the build will fail if Oracle's Java 9 or 10 are installed so be sure to delete such versions from the /Library/Java/JavaVirtualMachines folder before starting the build.

3. Install the following Mac Ports packages by downloading, compiling, and installing the following packages from the http://www.macports.org/ website. Note that you will need download and install the latest MacPorts Yosemite package to install the MacPorts "port" command. The "port" command is then used to do the downloading, compiling, and installation of the following packages:

   sudo /opt/local/bin/port install autoconf -x11
   sudo /opt/local/bin/port install automake -x11
   sudo /opt/local/bin/port install cvs -x11
   sudo /opt/local/bin/port install gnutar -x11
   sudo /opt/local/bin/port install xz -x11

   After running the above command, add "/opt/local/bin" to the end of your shell's PATH environment variable so that the build can all of the commands installed by /opt/local/bin/port command in the previous step.

4. Installed the Perl Archive::Zip module using the following command. You may need to run this command more than once as the unit tests may fail the first time that you run it:

   sudo cpan -i Archive::Zip

5. Disable System Integrity Protection (SIP). SIP must be disabled as it causes exporting DYLD_* environment variables in makefiles to fail which will break the build. To disable SIP, reboot into Recovery mode, run the following command in the Terminal, and then reboot normally:

   csrutil disable

6. Start the build by invoking the following commands. Note that you should replace $NEO_HOME with absolute path of your workspace's "neojava" directory:

   cd $NEO_HOME
   make

   Important note: if the build fails in the build.neo_tests make target, uncheck iCloud Drive in the System Preferences iCloud panel and reinvoke the above commands to continue the build.
