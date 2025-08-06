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

#include "ParserForms.hpp"
#include "FormProperty.hpp"

using namespace OmniDeviceCreationTool;

void ParserForms::
start_element(const string &n, const XMLPropertyHash &p)
{
   if (n == "deviceForm")
   {
      pPropertyNode = (FormProperty *) getFormsHead()->getNewChildNode();
      pPropertyNode->setSaved(true);
   }
}
// end of start_element


void ParserForms::
end_element(const string &n)
{
   if (n == "deviceForm")
   {
      getFormsHead()->addChild( pPropertyNode ) ;
   }
   else if (n == "name")
   {
      pPropertyNode->setFormName( charsRead );
   }
   else if (n == "formCapabilities" )
   {
      pPropertyNode->setFormCapability( charsRead );
   }
   else if (n == "command" )
   {
      pPropertyNode->setCommand( charsRead );
   }
   else if (n == "hardCopyCapLeft" )
   {
      pPropertyNode->setHardCopyCapLeft( charsRead );
   }
   else if (n == "hardCopyCapTop" )
   {
      pPropertyNode->setHardCopyCapTop( charsRead );
   }
   else if (n == "hardCopyCapRight" )
   {
      pPropertyNode->setHardCopyCapRight( charsRead );
   }
   else if (n == "hardCopyCapBottom" )
   {
      pPropertyNode->setHardCopyCapBottom( charsRead );
   }

   clear_charsRead();

}
