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
#ifndef _com_ibm_ltc_omni_guitool_GammaTableProperty_hpp
#define _com_ibm_ltc_omni_guitool_GammaTableProperty_hpp

#include <string>

#include "Node.hpp"
#include "GenericView.hpp"

namespace OmniDeviceCreationTool {

class GammaTableProperty : public Node
{
   private:
      // string & gammaName;
      string gammaTableResolution;
      string gammaTableMedia;
      string gammaTablePrintMode;
      string gammaTableDitherCatagory;

      string gammaTableCGamma;
      string gammaTableMGamma;
      string gammaTableYGamma;
      string gammaTableKGamma;

      string gammaTableCBias;
      string gammaTableMBias;
      string gammaTableYBias;
      string gammaTableKBias;


   public:
      GammaTableProperty(string newNodeName, Node * pParent);
      GammaTableProperty(GammaTableProperty *);

      /* Methods inherited and for which different implementation is required */

      virtual GenericView * getGUIPanel();
      virtual void          writeXML(ofstream & outFile, string deviceName);


   // Other methods required
   // getters and setters


         string getGammaTableResolution()
         { return gammaTableResolution; }

	 void setGammaTableResolution(string newGammaTableResolution)
         { gammaTableResolution = newGammaTableResolution; }

         string getGammaTableMedia()
         { return gammaTableMedia; }

	 void setGammaTableMedia(string newGammaTableMedia)
         { gammaTableMedia = newGammaTableMedia; }

         string getGammaTablePrintMode()
         { return gammaTablePrintMode; }

	 void setGammaTablePrintMode(string newGammaTablePrintMode)
         { gammaTablePrintMode = newGammaTablePrintMode; }

         string getGammaTableDitherCatagory()
         { return gammaTableDitherCatagory; }

	 void setGammaTableDitherCatagory(string newGammaTableDitherCatagory)
         { gammaTableDitherCatagory = newGammaTableDitherCatagory; }

         string getGammaTableCGamma()
         { return gammaTableCGamma; }

	 void setGammaTableCGamma(string newGammaTableCGamma)
         { gammaTableCGamma = newGammaTableCGamma; }

         string getGammaTableMGamma()
         { return gammaTableMGamma; }

	 void setGammaTableMGamma(string newGammaTableMGamma)
         { gammaTableMGamma = newGammaTableMGamma; }

         string getGammaTableYGamma()
         { return gammaTableYGamma; }

	 void setGammaTableYGamma(string newGammaTableYGamma)
         { gammaTableYGamma = newGammaTableYGamma; }

         string getGammaTableKGamma()
         { return gammaTableKGamma; }

	 void setGammaTableKGamma(string newGammaTableKGamma)
         { gammaTableKGamma = newGammaTableKGamma; }

         string getGammaTableCBias()
         { return gammaTableCBias; }

	 void setGammaTableCBias(string newGammaTableCBias)
         { gammaTableCBias = newGammaTableCBias; }

         string getGammaTableMBias()
         { return gammaTableMBias; }

	 void setGammaTableMBias(string newGammaTableMBias)
         { gammaTableMBias = newGammaTableMBias; }

         string getGammaTableYBias()
         { return gammaTableYBias; }

	 void setGammaTableYBias(string newGammaTableYBias)
         { gammaTableYBias = newGammaTableYBias; }

         string getGammaTableKBias()
         { return gammaTableKBias; }

	 void setGammaTableKBias(string newGammaTableKBias)
         { gammaTableKBias = newGammaTableKBias; }

}; // end class definition

}; // end of namespace

#endif
