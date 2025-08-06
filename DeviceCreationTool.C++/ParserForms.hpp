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
#ifndef _com_ibm_ltc_omni_guitool_ParserForms_hpp
#define _com_ibm_ltc_omni_guitool_ParserForms_hpp

#include <string>

#include "ParserBase.hpp"
#include "FormProperty.hpp"

namespace OmniDeviceCreationTool {

class ParserForms : public ParserBase
{
   private:
      FormProperty * pPropertyNode;

   public:

      void start_element(const string &n, const XMLPropertyHash &p);
      void end_element(const string &n);

}; // end class

}; // end of namespace

#endif
