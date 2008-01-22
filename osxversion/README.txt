This directory contains an OSXVersion component that can be used to get
the responses of the OS X sysv Gestalt from any language.  This allows
StarBasic macros to obtain information about the underlying operating
system before attempting to load components that reference undefined
symbols on the version.

After the extension is installed, it may be used from StarBasic similar
to the following example:

Dim o as Object
    o = CreateUnoService("org.neooffice.OSXVersion")
    
    Dim i as Integer
    i = o.osMajor()
    MsgBox i
    
    i = o.osMinor()
    MsgBox i
    
    i = o.osRevision()
    MsgBox i

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
      /path/to/this/module/unxmacx[ip].pro/bin/osxversion.oxt

The extension has no visible user interface;  it may only be loaded through
its UNO interfaces.

Edward Peterlin
1/21/08
