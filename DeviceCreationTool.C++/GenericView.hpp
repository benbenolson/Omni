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
#ifndef _COM_IBM_LTC_OMNI_GenericView_Hpp_
#define _COM_IBM_LTC_OMNI_GenericView_Hpp_

#include <gtk--/box.h>

// #include "Node.hpp"

using namespace Gtk;

namespace OmniDeviceCreationTool {

class Node; // Forward declaration
	
class GenericView : public VBox
{

protected:
   Node * pDataNode;
   static GenericView *pUniqueInstance;
   static GenericView *pPreviousInstance;
public:
   GenericView(){};
   virtual void  populateView(Node*){};
   void retainPreviousView();

protected:
   Node * getDataNode()
   {   return pDataNode; }

   void setDataNode(Node * pNewNode)
   {   pDataNode = pNewNode;  }

};

}; // end of namespace

#endif
