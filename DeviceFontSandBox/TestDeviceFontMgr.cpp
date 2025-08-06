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
 *   October 16, 2002
 *   $Id: TestDeviceFontMgr.cpp,v 1.9 2002/11/14 06:00:51 rcktscientist Exp $
 */

#include "ByteArray.hpp"
#include "CommandSequence.hpp"
#include "DeviceFontMgr.hpp"
#include "DeviceFont.hpp"
#include "Event.hpp"
#include "Panose.hpp"
#include "Parameter.hpp"
#include "DeviceFontException.hpp"

#include <iostream>
#include <string>

using namespace std;
using namespace DevFont;

/**
 * A main program that tries to exercise the Device Font Manager by calling
 * different methods in a variety of ways.
 *
 * @author David L. Wagner
 * @version 1.0
 */


/**
 * Print out the usage summary, then quit.
 */
void usage (char *progname)
{
    cout << "Usage: " << progname << " [options]" << endl;
    cout << "Options:  -full       display full font information" << endl;
    cout << "          -arial      get the bytes for the arial font" << endl;
    cout << "          -cmds       list command sequences" << endl;
    cout << "          -events     list events" << endl;
    cout << "          -params     list parameters" << endl;
    cout << "          -panose     demonstrate Panose lookup" << endl;
    cout << "          -drawtext   use drawtext() method (experimental)" << endl;
    cout << "          -all        all of the above" << endl;
    cout << "          -printer p  specify the printer name" << endl;
    exit (0);
}

int main (int argc, char **argv)
{
    //
    // Flags to indicate what we should do
    //
    bool dofull = false;
    bool docmds = false;
    bool doarial = false;
    bool doevents = false;
    bool doparams = false;
    bool dopanose = false;
    bool dodrawtext = false;
    bool doall = false;
    char *printer = "HP LaserJet 9000";
    //
    // Look at the command line arguments
    //
    if (argc > 1)
      {
	if (argc >9)
	  {
	    cout << "Too many arguments passed! \n"
		 << "*************************** \n";
	    usage(argv[0]);
	  }
	else
	  {
	    for (int i = 1; i < argc; i++)
	      {
		if (strcmp(argv[i], "-cmds") == 0)
		  docmds = true;
		else if (strcmp(argv[i], "-arial") == 0)
		  doarial = true;
                else if (strcmp(argv[i], "-events") == 0)
                    doevents = true;
		else if (strcmp(argv[i], "-params") == 0)
		  doparams = true;
		else if (strcmp(argv[i], "-panose") == 0)
		  dopanose = true;
		else if (strcmp(argv[i], "-full") == 0)
		  dofull = true;
		else if (strcmp(argv[i], "-drawtext") == 0)
		  dodrawtext = true;
		else if (strcmp(argv[i], "-all") == 0)
		  doall = true;
		else if (strcmp(argv[i], "-printer") == 0)
		  {
		    if (argv[i+1] != NULL)
		      {
			if (!strcmp(argv[i+1], "-cmds") || !strcmp(argv[i+1], "-params") 
			    || !strcmp(argv[i+1], "-panose") || !strcmp(argv[i+1], "-full")
			    || !strcmp(argv[i+1], "-drawtext") || !strcmp(argv[i+1], "-all")
			    || !strcmp(argv[i+1], "-printer"))
			  {
			    cout << "\n\nPrinter Device Name Needed!! \n"
				 << "*******************************************************\n";
			    usage(argv[0]);
			  }

			printer = argv[i+1];
			i++;
		      }
		    else usage(argv[0]);
		  }
		else
		  usage(argv[0]);
	      }
	  }

      }
    //else
    //  {
//	usage(argv[0]);
  //    }
    //
    // Create the device font manager
    //
    const char * prefix = "UPDF Device Configuration-";
    char *device = new char[strlen(prefix) + strlen(printer) + 1];
    strcpy (device, prefix);
    strcat (device, printer);
  //
  // Create the device font manager
  //
  try
    {
      DeviceFontMgr mgr (device);
      delete [] device;
      //
      // List out all the fonts
      //
      cout << "Fonts for " << printer << ":" << endl;
      const DeviceFont *font = mgr.getFonts();
      while (font != NULL)
	{
	  //cout << font->pszID << "   /   " << font->iGlobal_Units_Per_Em << endl;
	  cout << *font << endl;
	  if (dofull)
	    {
	      cout << "Full font information:" << endl;
	      font->print();
	    }
	  font = font->getNext();
	}
      cout << endl;
      //
      // List all the command sequences
      //
      if (docmds || doall)
	{
	  const CommandSequence *cs = mgr.getCommandSequences();
	  if (cs == NULL)
	    cout << "No command sequences read" << endl;
	  else
	    {
	      cout << "All the command sequences:" << endl;
	      while (cs != NULL)
		{
		  cout << *cs << endl;
		  cs = cs->getNext();
		}
	    }
	  cout << endl;
	}
      //
      // List all the events
      //
      if (doevents || doall)
	{
	  const Event *ev = mgr.getEvents();
	  if (ev == NULL)
	    cout << "No events read" << endl;
	  else
	    {
	      cout << "All the events:" << endl;
	      while (ev != NULL)
		{
		  cout << *ev << endl;
		  ev = ev->getNext();
		}
	    }
	  cout << endl;
	}
      //
      // List all the parameters
      //
      if (doparams)
	{
	  const Parameter *param = mgr.getParameters();
	  if (param == NULL)
	    cout << "No parameters read" << endl;
	  else
	    {
	      cout << "All the parameters:" << endl;
	      while (param != NULL)
		{
		  cout << param << ": " << param->getName() <<  " = \"" << param->getValue() << "\"" << endl;
		  param = param->getNext();
		}
	      cout << endl;
	    }
	}
      //
      // Check to see if we can get and set the Arial font
      //
      if (doarial || doall)
      {
        cout << "Trying to set the font to Arial..." << endl;
        font = mgr.getFont ("Arial");
        if (font != NULL)
	{
	  const ByteArray *ba = mgr.getFontCommand (font, 12);
	  //
	  // If no command sequence is found, then DeviceFontMgr::setFont() returns
	  // NULL.
	  //
	  if (ba)
	    {
	      cout << "Resulting bytes: " << *ba << " (\"" << ba->getASCII() << "\")" << endl;
	      delete ba;
	    }
	  else
	    {
	      cout << "No command sequence found for setting font to Arial" << endl;
	    }
	}
        else
	  cout << "No Arial font found" << endl;
        cout << endl;
      }
      //
      // Construct a Panose value to represent a monospaced font
      //
      if (dopanose || doall)
	{
	  cout << "Trying to find the best monospaced font using Panose values..." << endl;
	  Panose p (Panose::FAMILY_TEXT_DISPLAY, Panose::SERIF_ANY,
		    Panose::WEIGHT_ANY, Panose::PROP_MONOSPACED,
		    Panose::CONTRAST_ANY, Panose::STROKE_ANY,
		    Panose::ARMS_ANY, Panose::LETT_ANY,
		    Panose::MIDLINE_ANY, Panose::XHEIGHT_ANY);
	  cout << "  ... p = " << p << endl;
	  //
	  // Find a monospaced font
	  //
	  font = mgr.getFont (&p);
	  if (font)
	    cout << "Best monospaced font is " << font->pszID << endl;
	  cout << endl;
	}
      //
      // Print out the bytes that would happen in a drawtext() call
      //
      if (dodrawtext || doall)
	{
          mgr.startPage();
	  mgr.drawText (100, 200, "Hello");
          mgr.endPage();
          const ByteArray *ba = mgr.getBytes();
          cout << "Bytes from drawtext(): " << *ba << endl;
	}
    }
  catch (const DeviceFontException& e)
    {
      cout << "Caught DeviceFontException" << endl;
      cout << e.message << endl;
      exit (1);
    }
  catch (...)
    {
      cout << "\n\nException message is: Unexpected Excpetion!\n";
      exit (1);
    }
}
