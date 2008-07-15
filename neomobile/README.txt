The files in this directory contain the basis for interfacing the Mac OS X
NeoOffice Mobile library with OOo.  The interface between OOo and IC consists
of a UNO component and some simple StarBasic driver code.  The UNO component
consists of a class that invokes displays a native Cocoa WebView instance
that displays the NeoOffice Mobile website's pages.

The actual driver code is in the NeoOfficeMobile StarBasic library in
the NeoOfficeMobile subdirectory.  This is the library that is
incorporated into the oxt.  The eventual output is an extension package
named NeoOfficeMobile.oxt within the bin platform output directory.

The Context for this extension is left blank as NeoOffice Mobile supports
most major OOo file types.  This leaves the add on always available.

To deplay this extension file, you must *not* use the
Tools :: Extensions Manager menu in NeoOffice as that will run the extension
in a separate process instead of within the NeoOffice process itself.
The NeoOfficeMobile library requires a valid event loop in order to display
the dialog properly.

Instead, to properly deplay the extension, you must install it as a shared
extension using the following steps:

1. Launch NeoOfice and ensure that all previous installations of the extension
   in the My Extensions group are removed.

2. Quit NeoOffice, open a new Terminal window with a clean environment, and
   execute the following command in the new Terminal window. Note that you
   must cd within the NeoOffice installation otherwise unopkg.bin will abort:

   sudo rm -Rf ~root/Library/Preferences/NeoOffice-[version]
   cd /path/to/NeoOffice/installation/Contents/MacOS
   sudo -H ./unopkg.bin remove --shared --force NeoOfficeMobile.oxt
   sudo -H ./unopkg.bin add --shared --force \
      /path/to/this/module/unxmacx[ip].pro/bin/NeoOfficeMobile.oxt

3. Launch NeoOffice and the extension should be active. New menu items
   under the Tools :: Add-ons menu and new toolbars under the View :: Toolbars
   menu should be present in all OOo components.
