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
#include <xml++.h>

#include "ParserDatas.hpp"
#include "DataProperty.hpp"

using namespace std;
using namespace OmniDeviceCreationTool;


void ParserDatas::
start_element(const string &n, const XMLPropertyHash &p)
{
   XMLPropertyHash propHash = p;
   XMLProperty *prop;

   if (n == "deviceData")
   {
      pPropertyNode = (DataProperty *) getDatasHead()->getNewChildNode();
      pPropertyNode->setSaved(true);
      prop = propHash["name"];
      pPropertyNode->setDataName(prop->value());
      prop = propHash["type"];
      pPropertyNode->setDataType(prop->value());
   }

}
// end of start_element


void ParserDatas::
end_element(const string &n)
{
   if (n == "deviceData")
   {
      pPropertyNode->setDeviceData(charsRead);
      getDatasHead()->addChild(pPropertyNode) ;
   }

   clear_charsRead();

}
