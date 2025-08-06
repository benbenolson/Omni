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

#include <iostream.h>
#include <string>

#include "ParserGammas.hpp"
#include "GammaTableProperty.hpp"

using namespace OmniDeviceCreationTool;

void ParserGammas::
start_element(const string &n, const XMLPropertyHash &p)
{
   if (n == "deviceGammaTable")
   {
      pPropertyNode = (GammaTableProperty *) getGammasHead()->getNewChildNode();
      pPropertyNode->setSaved(true);
   }
}
// end of start_element

void ParserGammas::
end_element(const string &n)
{
   if (n == "deviceGammaTable")
   {
      getGammasHead()->addChild( pPropertyNode ) ;
   }
   else if (n == "gammaTableResolution" )
   {
      pPropertyNode->setGammaTableResolution( charsRead );
   }
   else if (n == "gammaTableMedia" )
   {
      pPropertyNode->setGammaTableMedia( charsRead );
   }
   else if (n == "gammaTablePrintMode" )
   {
      pPropertyNode->setGammaTablePrintMode( charsRead );
   }
   else if (n == "gammaTableDitherCatagory" )
   {
      pPropertyNode->setGammaTableDitherCatagory( charsRead );
   }
   else if (n == "gammaTableCGamma" )
   {
      pPropertyNode->setGammaTableCGamma( charsRead );
   }
   else if (n == "gammaTableMGamma" )
   {
      pPropertyNode->setGammaTableMGamma( charsRead );
   }
   else if (n == "gammaTableYGamma" )
   {
      pPropertyNode->setGammaTableYGamma( charsRead );
   }
   else if (n == "gammaTableKGamma" )
   {
      pPropertyNode->setGammaTableKGamma( charsRead );
   }
   else if (n == "gammaTableCBias" )
   {
      pPropertyNode->setGammaTableCBias( charsRead );
   }
   else if (n == "gammaTableMBias" )
   {
      pPropertyNode->setGammaTableMBias( charsRead );
   }
   else if (n == "gammaTableYBias" )
   {
      pPropertyNode->setGammaTableYBias( charsRead );
   }
   else if (n == "gammaTableKBias" )
   {
      pPropertyNode->setGammaTableKBias( charsRead );
   }

   clear_charsRead();

}
