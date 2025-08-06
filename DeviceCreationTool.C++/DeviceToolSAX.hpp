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
#ifndef _com_ibm_ltc_omni_guitool_DeviceToolSAX_hpp
#define _com_ibm_ltc_omni_guitool_DeviceToolSAX_hpp

#include <string>
#include <libxml++/libxml++.h>

#include "RootNode.hpp"

namespace OmniDeviceCreationTool {

class DeviceToolSAX
{

private:
   static DeviceToolSAX * pInstance ;

   RootNode * pDataRoot;
   vector<string> filesQ;     // stores the files to be processed
   string errorMessage;
   bool fError;

   XMLParser<XMLParserCallback> * getParser(string fileName);

   void    parseCommands      (string fileName);
   void    parseResolutions     (string fileName);
   void    parsePrintModes     (string fileName);
   void    parseForms             (string fileName);
   void    parseTrays              (string fileName);
   void    parseMedias           (string fileName);
   void    parseConnections   (string fileName);
   void    parseGammas         (string fileName);
   void    parseDatas              (string fileName);
   void    parseOrientations    (string fileName);

public:

   static DeviceToolSAX * getInstance();
   int parseFile(string fileName, RootNode * pNewDataRoot);
   int loadFile(string fileName, RootNode *pNewDataRoot, string nodeName);

   RootNode * getDataRoot()
   {
      return pDataRoot ;
   }

   void addFile( string newFileName )
   {
      if (!newFileName.empty())
         filesQ.push_back(newFileName);
   }

   void setError(bool fE, string msg)
   {
      fError = fE;
      errorMessage = msg;
   }

}; // end class

}; // end of namespace

#endif
