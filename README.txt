Intructions for Building NeoOffice
----------------------------------

This git branch - NeoOffice-2017_branch - contains code for building the latest NeoOffice 2017 releases on macOS Sierra or Mojave for Intel.

All of the official NeoOffice 2017 releases are built from this branch. If you want to build NeoOffice on macOS Big Sur for Intel, switch to the NeoOffice-2021_branch git branch.

Important: In order to build NeoOffice, all of the steps below must be followed. These steps install several third party tools that are needed by NeoOffice's underlying LibreOffice code. Also, these steps will install the git LFS extension so that several very large files needed by the build can be downloaded from GitHub's LFS repository.

If any third party tools are not installed or the build is run on a different version of macOS or with a different version of Xcode, the build will likely fail.


Steps for building on macOS 10.12 Sierra or 10.14 Mojave for Intel
------------------------------------------------------------------

At this time, NeoOffice will only build on macOS 10.12 Sierra or macOS 10.14 Mojave.

1. Make sure that you have downloaded and installed the following dependencies from http://developer.apple.com/ website:

   macOS 10.12 Sierra:
     Xcode v8.3.3
     Command Line Tools for Xcode v8.3.2
   macOS 10.14 Mojave:
     Xcode v10.3
     Command Line Tools for Xcode v10.3

2. Download and install Oracle's Java 1.8 Development Kit (JDK) from the following URL:

   http://www.neooffice.org/neojava/javadownload.php

   Important: the build will fail if Oracle's Java 9 or 10 are installed so be sure to delete such versions from the /Library/Java/JavaVirtualMachines folder before starting the build.

3. Install the following Mac Ports packages by downloading, compiling, and installing the following packages from the http://www.macports.org/ website. Note that you will need download and install the latest MacPorts Yosemite package to install the MacPorts "port" command. The "port" command is then used to do the downloading, compiling, and installation of the following packages:

   sudo /opt/local/bin/port install autoconf -x11
   sudo /opt/local/bin/port install automake -x11
   sudo /opt/local/bin/port install cvs -x11
   sudo /opt/local/bin/port install gnutar -x11
   sudo /opt/local/bin/port install xz -x11
   sudo /opt/local/bin/port install git-lfs -x11

   After running the above command, add "/opt/local/bin" to the end of your shell's PATH environment variable so that the build can all of the commands installed by /opt/local/bin/port command in the previous step.

4. Make sure the git LFS extension is installed:

   git lfs install

5. Download all LFS files from Github's LFS repository:

   cd "<source folder>"
   git lfs fetch
   git lfs checkout

6. Installed the Perl Archive::Zip module using the following command. You may need to run this command more than once as the unit tests may fail the first time that you run it:

   sudo cpan -i Archive::Zip

7. Disable System Integrity Protection (SIP). SIP must be disabled as it causes exporting DYLD_* environment variables in makefiles to fail which will break the build. To disable SIP, reboot into Recovery mode, run the following command in the Terminal, and then reboot normally:

   csrutil disable

8. To build the installers, obtain the following types of codesigning certificates from Apple and install the certificates in the macOS Keychain Access application:

   3rd Party Mac Developer Application
   3rd Party Mac Developer Installer
   Developer ID Application
   Developer ID Installer

9. Assign the codesigning certificates obtained in the previous step by copying the "<source folder>/certs.neo.mk" file to "<source folder>/certs.mk". Then, open the "<source folder>/certs.mk" file and replace all of Planamesa Inc.'s certificate names and team ID with your certificate names team ID. Important note: each certificate name assigned in the "<source folder>/certs.mk" file must match the certificate's "Common Name" field in the macOS Keychain Access application.

10. Start the build by invoking the following commands:

   cd "<source folder>"
   make

   A successful build will create the following 3 "<source folder>/install*/*.dmg" files:

      "<source folder>/install/*.dmg" - Installer for the Mac App Store version
      "<source folder>/install2/*.dmg" - Installer for the Viewer version
      "<source folder>/install3/*.dmg" - Installer for the Professional Edition version

   Important note: if the build fails in the build.neo_tests make target, uncheck iCloud Drive in the System Preferences iCloud panel and reinvoke the above commands to continue the build.

11. After a successful build, you can optionally build patch installers by invoking the following commands:

   cd "<source folder>"
   make build.all_patches

   A successful build will create the following 3 "<source folder>/patch_install*/*.dmg" files:

      "<source folder>/patch_install/*.dmg" - Patch installer for the Mac App Store version
      "<source folder>/patch_install3/*.dmg" - Patch installer for the Professional Edition version

12. If you are building NeoOffice on macOS 10.14 Mojave, you can notarize the installers using Apple's notarization service by opening the "<source folder>/certs.mk" file that you created and setting the APPLEDEVELOPERID macro to the e-mail of your Apple Developer ID. Then, invoke the following command:

   make build.notarize_all

   If you built patch installers, also invoke the following command:

   make build.notarize_all_patches

13. If you notarized the installers in the previous step and Apple has sent you  an e-mail saving that your installers were successfully notarized, "staple" Apple's notarization to the installers by invoking the following command:

   make build.staple_all

   If you built patch installers, also invoke the following command:

   make build.staple_all_patches
