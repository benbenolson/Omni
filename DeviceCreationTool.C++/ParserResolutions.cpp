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

#include "ParserResolutions.hpp"
#include "ResolutionProperty.hpp"

using namespace OmniDeviceCreationTool;

void ParserResolutions::
start_element(const string &n, const XMLPropertyHash &p)
{
   if (n == "deviceResolution" )
   {
      pPropertyNode = (ResolutionProperty *) getResolutionsHead()->getNewChildNode();
      pPropertyNode->setSaved(true);
   }
}
 // end of start_element


void ParserResolutions::
end_element(const string &n)
{
   if (n == "deviceResolution" )
   {
      getResolutionsHead()->addChild( pPropertyNode ) ;
   }
   else if (n == "name" )
   {
      pPropertyNode->setResolutionName( charsRead );
   }
   else if (n == "xRes" )
   {
      pPropertyNode->setXRes( charsRead );
   }
   else if (n == "yRes" )
   {
      pPropertyNode->setYRes( charsRead );
   }
   else if (n == "xInternalRes")
   {
      pPropertyNode->setXInternalRes(charsRead);
   }
   else if (n == "yInternalRes")
   {
      pPropertyNode->setYInternalRes(charsRead);
   }
   else if (n == "command")
   {
      string in = charsRead;
      in = process_entity_reference_amp(in);
      in = process_entity_reference_apos(in);
      pPropertyNode->setCommand(in);
   }
   else if (n == "resolutionCapability" )
   {
      pPropertyNode->setResolutionCapability( charsRead );
   }
   else if (n == "resolutionDestinationBitsPerPel" )
   {
      pPropertyNode->setResolutionDestinationBitsPerPel( charsRead );
   }
   else if (n == "resolutionScanlineMultiple" )
   {
      pPropertyNode->setResolutionScanLineMultiple( charsRead );
   }

   clear_charsRead();

}
