/*
 *   IBM Omni driver
 *   Copyright (c) International Business Machines Corp., 2000
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
 */
#ifndef _com_ibm_ltc_omni_guitool_DefaultJobProperties_hpp
#define _com_ibm_ltc_omni_guitool_DefaultJobProperties_hpp

#include <fstream>
#include <string>

#include "Node.hpp"
#include "GenericView.hpp"

namespace OmniDeviceCreationTool {

class DefaultJobProperties : public Node
{
   private:
      string orientation;
      string form;
      string tray;
      string media;
      string resolution;
      string dither;
      string printMode;
      string other;
   /*
      NOTE : other - This string will
      contain a comma separated list of values.
      The individual values are extracted when output XML
      file is being written (in function writeXML)
   */

   public:
      DefaultJobProperties(string newNodeName, Node * pParent);

      /* Methods inherited and for which different implementation is required */

      virtual GenericView * getGUIPanel();
      virtual void writeXML(ofstream & outFile, string deviceName);

   // Other methods required
   // getter and set methods

      string getOrientation()
      { return orientation; }

      void setOrientation(string newOrientation)
      { orientation = newOrientation; }

      string getForm()
      { return form; }

      void setForm(string newForm)
      { form = newForm; }

      string getTray()
      { return tray; }

      void setTray(string newTray)
      { tray = newTray; }

      string getMedia()
      { return media; }

      void setMedia(string newMedia)
      { media = newMedia; }

      string getResolution()
      { return resolution; }

      void setResolution(string newResolution)
      { resolution = newResolution; }

      string getDither()
      { return dither; }

      void setDither(string newDither)
      { dither = newDither; }

      string getPrintMode()
      { return printMode; }

      void setPrintMode(string newPrintMode)
      { printMode = newPrintMode; }

      string getOther()
      { return other; }

      void setOther(string newOther)
      { other = newOther; }

}; // end class definition

}; // end of namespace

#endif
