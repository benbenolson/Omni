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
 *   $Id: StressDeviceFontMgr.cpp,v 1.2 2002/10/25 04:32:50 rcktscientist Exp $
 */

#include "ByteArray.hpp"
#include "DeviceFontMgr.hpp"
#include "DeviceFont.hpp"

#include <iostream>

using namespace std;
using namespace DevFont;

/**
 * A main program that tries to exercise the Device Font Manager by calling
 * its methods many times over.  One can then check for memory leaks and such.
 *
 * @author David L. Wagner
 * @version 1.0
 * $Id: StressDeviceFontMgr.cpp,v 1.2 2002/10/25 04:32:50 rcktscientist Exp $
 */

int main (int argc, char **argv)
{
  for (int i = 0; i < 10000; i++)
  {
    if (i % 50 == 0)
      cout << i << "..." << endl;
    //
    // Create the device font manager
    //
    DeviceFontMgr mgr ("UPDF Device Configuration-HP LaserJet 9000");
    const DeviceFont *font = mgr.getFont ("Arial");
    if (font != NULL)
    {
      const ByteArray *ba = mgr.getFontCommand (font, 12);
      if (ba)
      {
        const unsigned char *b = ba->getBytes();
        delete ba;
      }
    }
  }
}
