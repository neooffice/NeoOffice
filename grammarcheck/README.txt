The files in this directory contain the basis for interfacing the Mac OS X
system level grammar checker with OOo.  This consists of an UNO library
providing C++ glue to the NSSpellChecker class.  A basic modal user interface
is defined in StarBasic along with the driver code to pass each sentence
to the implementation library.  The GrammarGUI.odt file contains the macros
for development;  to deploy in the oxt the library should be exported from
the macro organization dialog.  After exporting, the location in the dialog's
definition file must be modified by hand to change "location=document"
to "location=application" to function for deployment.  It is not possible
to make this modification from within the odt file as the user interface
of OOo prevents manual editing of this parameter.

The eventual output of the makefile is an extension package named
unxmacx[ip].pro/bin/grammarcheck.oxt that bundles up the UNO type library,
component, StarBasic code, and supporting xcu and manifest files for
deployment.

To deplay this extension file, you must *not* use the
Tools :: Extensions Manager menu in NeoOffice as that will run the extension
in a separate process instead of within the NeoOffice process itself.

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
      /path/to/this/module/unxmacx[ip].pro/bin/grammarcheck.oxt

3. Launch NeoOffice and the extension should be active. 2 new menu items
   called Tools :: Add-ons :: Check Grammar of Entire Document and
   Tools :: Add-ons :: Check Grammar of Selected Text should be present in
   in NeoOffice Writer.

Edward Peterlin
12/10/2007

Patrick Luby
01/14/2008
