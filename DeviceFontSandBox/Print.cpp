/*
 *   IBM Linux Device Font Library
 *   Copyright (c) International Business Machines Corp., 2002
 *
 *   This library is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Lesser General Public License as published
 *   by the Free Software Foundation; either version 2.1 of the License, or
 *   (at your option) any later version.
 *
 *   This library is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 *   the GNU Lesser General Public License for more details.
 *
 *   You should have received a copy of the GNU Lesser General Public License
 *   along with this library; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 *   Author: David L. Wagner
 *   November 11, 2002
 *   $Id: Print.cpp,v 1.4 2002/11/21 16:47:48 rcktscientist Exp $
 */

#include "ByteArray.hpp"
#include "DeviceFontMgr.hpp"
#include "DeviceFont.hpp"

#include <fstream>
#include <iostream>
#include <string>

using namespace std;
using namespace DevFont;

/**
 * A main program to demonstrate the functionality of the Device Font Package
 * by printing a plain text document in a specified font.  The format of the
 * command is either:
 *
 *   ./Print -P <printer> -F <font> file
 *
 * or
 *
 *   ./Print -P <printer> file
 *
 * In the latter, the program will try to find the best monospaced font to
 * use.  The output will go to the standard output, so you might want to
 * pipe the output to a file.  This also only works for pure text; the
 * outbytes() function would have to be modified streams of binary data.
 *
 * @author David L. Wagner
 * @version 1.0
 * $Id: Print.cpp,v 1.4 2002/11/21 16:47:48 rcktscientist Exp $
 */


/**
 * Print out the usage summary, then quit.
 */
void usage (char *progname)
{
    cerr << "Usage: " << progname << " -P <printer> [-F <font>] <infile>" << endl;
    exit (1);
}


/** Needs to be fixed for binary data. */
void outbytes (const ByteArray *ba)
{
    if (ba == NULL)
        return;
    int n = ba->getLength();
    const unsigned char *data = ba->getBytes();
    for (int i = 0; i < n; i++)
        cout << data[i];
}

int main (int argc, char **argv)
{
    if (argc != 4 && argc != 6)
        usage(argv[0]);
    if (strcmp (argv[1], "-P") != 0)
        usage(argv[0]);
    if (argc == 6 && strcmp (argv[3], "-F") != 0)
        usage(argv[0]);
    //
    // Get the printer and font names and the input file name
    //
    const char *printername = argv[2];
    bool userfont = argc == 6;
    const char *fontname = NULL;
    if (userfont)
        fontname = argv[4];
    const char *filename = argv[userfont ? 5 : 3];
    //
    // Open the input file
    //
    ifstream file;
    file.open (filename);
    //
    // Get the name of the UPDF file
    //
    const char * prefix = "UPDF Device Configuration-";
    char *device = new char[strlen(prefix) + strlen(printername) + 1];
    strcpy (device, prefix);
    strcat (device, printername);
    //
    // Create the device font manager
    //
    DeviceFontMgr mgr (device);
    delete [] device;
    //
    // Get the font we want
    //
    const DeviceFont *font = NULL;
    if (userfont)
    {
      font = mgr.getFont (fontname);
      if (font == NULL)
      {
        cerr << "Could not find a font named " << argv[4] << endl;
        exit (2);
      }
    }
    else
    {
        //
        // Look for a medium monospaced font
        //
        Panose p (Panose::FAMILY_TEXT_DISPLAY, Panose::SERIF_ANY,
                  Panose::WEIGHT_MEDIUM, Panose::PROP_MONOSPACED,
                  Panose::CONTRAST_ANY, Panose::STROKE_ANY,
                  Panose::ARMS_ANY, Panose::LETT_NORMAL_BOXED,
                  Panose::MIDLINE_ANY, Panose::XHEIGHT_ANY);
        //
        // Find a monospaced font
        //
        font = mgr.getFont (&p);
        if (font == NULL)
        {
            cerr << "Could not find a usable font" << endl;
            exit (2);
        }
    }
    //
    // Start the document
    //
    //cout << "Calling startJob()" << endl;
    mgr.startJob();
    outbytes (mgr.getBytes());
    //cout << "Calling startDocument()" << endl;
    mgr.startDocument();
    outbytes (mgr.getBytes());
    //
    // Use a 10-point version of whatever font
    //
    //cout << "Calling setFont()" << endl;
    mgr.setFont (font);
    mgr.setSize (1000);
    int line = 0;
    //
    // Read each line into a buffer and then format it
    //
    char buf[1024];
    while (file.getline(buf,1024))
    {
        if (line == 0)
            mgr.startPage();
        //
        // Read a line from the input file; if we are done, then break
        // We indent by 72 points (1 inch) and we assume that the coordinate
        // system is PostScript's, so we have to go from the top of the page
        // to the bottom.
        //
        mgr.drawText (72, 708 - line * 12, buf);
        //
        // See if we are at the end of the page
        //
        line++;
        if (line == 54)
        {
            mgr.endPage();
            outbytes (mgr.getBytes());
            line = 0;
        }
    }
    //
    // Flush the last page
    //
    if (line != 0)
        mgr.endPage();
    mgr.endDocument();
    mgr.endJob();
    outbytes (mgr.getBytes());
    file.close();
}
