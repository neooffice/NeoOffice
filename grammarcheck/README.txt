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
grammarcheck.oxt that bundles up the UNO type library, component, StarBasic
code, and supporting xcu and manifest files for deployment.  To deploy as
an extension, use Tools > Extension Manager and add the .oxt.  A new
menu item "Check Document Grammar" will be present under the Add-ons menu
for Writer documents.  Note that any existing documents will need to be
reopened in order for the menu item to appear.

Edward Peterlin
12/10/07
