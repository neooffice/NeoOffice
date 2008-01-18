The files in this directory contain the basis for interfacing the Mac OS X
Image Capture library with OOo.  The Image Capture framework allows for
extraction of images on an attached camera, scanner, or other IC supported
device.  The interface between OOo and IC consists of a UNO component and
some simple StarBasic driver code.  The UNO component consists of a class
that invokes IC, converts the returned image to TIFF format using
CoreGraphics, and places the image on the clipboard.  The StarBasic
driver code simply loads the component, executes the entry point, and
then issues a paste operation to place the image into the current document.

The ImageCapture.odt document contains a development version of the driver
code.  The actual driver code is exported into the ImageCapture StarBasic
library in the ImageCapture subdirectory.  This is the library that is
incorporated into the oxt.  The eventual output is an extension package
named imagecapture.oxt within the bin platform output directory.

The Context for this extension is left blank as bitmaps are supported within
most major OOo components.  This leaves the add on always available.

To deplay this extension file, you must *not* use the
Tools :: Extensions Manager menu in NeoOffice as that will run the extension
in a separate process instead of within the NeoOffice process itself.
The ImageCapture library requires a valid event loop in order to display
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
   sudo -H ./unopkg.bin add --shared --force \
      /path/to/this/module/unxmacx[ip].pro/bin/imagecapture.oxt

3. Launch NeoOffice and the extension should be active. A new menu item
   called Tools :: Add-ons :: Paste Image from Camera or Scanner should be 
   present in all OOo components.
