README.txt
David L. Wagner
Oct 24, 2002
$Id: README.txt,v 1.2 2002/10/25 04:32:50 rcktscientist Exp $

-----------------------
IBM Device Font Library
-----------------------

This collection of classes provides a simple interface to the device
fonts on a printer.  It assumes that all the printer capabilities,
including the fonts, are described in an associated UPDF file (and
this code will only be as accurate as the corresponding UPDF file).

There are only three classes that are particularly relevant for the
casual user: DeviceFontMgr, DeviceFont and ByteArray.  One creates a
single DeviceFontMgr instance for each printer, specifying the
corresponding UPDF file.  One can then retrieve a linked list of
DeviceFont objects to find the one desired.  (In the near future, one
will be able to find fonts based on their general characteristics,
using Panose values.)  A bit of sample code to select a font might
look like:

{
  //
  // Parse the UPDF file
  //
  DevFont::DeviceFontMgr manager ("HP Laserjet 500");
  //
  // Get a font by name
  //
  DevFont::DeviceFont *font = manager.getFont("Times-Roman");
  //
  // Find the bytes needed to set the font (at 12 point size)
  //
  DevFont::ByteArray *bytes = manager.setFont (font, 1200);
  //
  // And write the bytes to the printer output stream
  //
  printer.write (bytes.getBytes(), bytes.getLength());
}
