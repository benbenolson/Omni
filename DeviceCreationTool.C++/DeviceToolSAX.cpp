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

#include <iostream>
#include <string>
#include <xml++.h>

#include "DeviceToolSAX.hpp"
#include "RootNode.hpp"
#include "DeviceToolConstants.hpp"
#include "ParserDevice.hpp"
#include "ParserCommands.hpp"
#include "ParserResolutions.hpp"
#include "ParserPrintModes.hpp"
#include "ParserTrays.hpp"
#include "ParserForms.hpp"
#include "ParserMedias.hpp"
#include "ParserConnections.hpp"
#include "ParserGammas.hpp"
#include "ParserDatas.hpp"
#include "ParserOrientations.hpp"

using namespace OmniDeviceCreationTool;

// static member initialization
DeviceToolSAX * DeviceToolSAX::pInstance = NULL;

DeviceToolSAX * DeviceToolSAX::
getInstance()
{
   if (DeviceToolSAX::pInstance == NULL)
      pInstance = new DeviceToolSAX();

   pInstance->fError = false;
   return pInstance;
}


int DeviceToolSAX::
parseFile(string fileName, RootNode * pNewDataRoot)
{
      // clear previously held data
      pDataRoot = NULL;
      errorMessage.erase();
      fError = false;
      filesQ.clear();
      //......

      pDataRoot = pNewDataRoot;

      XMLTree *pXmlTree;

      pXmlTree = new XMLTree();
      bool fSuccess;
      fSuccess = pXmlTree->read(fileName);

      if (fSuccess == false)
      {
         cerr << "An error has occured in parsing file -" << fileName << endl;
         return -1;
      }

      XMLParser<ParserDevice> saxP;

      string buf = pXmlTree->write_buffer();

      saxP.parse_chunk(buf);

      if (fError == true)
         return -1;
     /*
     * when control comes here
     * parsing of base file will be completed
     * and the file names present between
     * <uses> and <has> will be stored in
     * the filesQ vector for further processing
     */

      // extract directory name from file name
      int lastSlash = fileName.find_last_of('/');
      string dir;

      if(lastSlash != string::npos)
         dir = fileName.substr(0,lastSlash+1);

      vector<string>::iterator iter;
      for(iter=filesQ.begin(); iter!=filesQ.end() && fError!=true; iter++)
      {
            int lastBlank = (*iter).find_last_of(' ');
	    // search for last blank space - the blank just before aaa.xml
            int lastDot = (*iter).find_last_of('.');
	    // search for last . (dot) - the dot before .xml

            // extract property name - eg Resolution, Command etc
            string property=(*iter).substr(lastBlank+1, lastDot-(lastBlank+1));

            if (property == RESOLUTIONS)
                  parseResolutions(dir+(*iter));
            else if (property == COMMANDS)
                  parseCommands( dir+(*iter));
            else if (property == PRINTMODES)
                  parsePrintModes(dir+(*iter));
            else if (property == TRAYS)
                  parseTrays(dir+(*iter));
            else if (property == FORMS)
                  parseForms(dir+(*iter));
            else if (property == MEDIAS)
                  parseMedias(dir+(*iter)) ;
            else if (property == CONNECTIONS)
                  parseConnections(dir+(*iter));
            else if (property == GAMMAS)
                  parseGammas(dir+(*iter)) ;
            else if (property == DATA)
                  parseDatas(dir+(*iter)) ;
            else if (property == ORIENTATIONS)
                  parseOrientations(dir+(*iter)) ;
      }

      if (fError)
      {
         cerr << "An error has occured during parsing " << errorMessage << endl;
         return -1;
      }

      return 0;
}

int DeviceToolSAX::
loadFile(string fileName, RootNode * pNewDataRoot, string nodeName)
{
      // clear previously held data
      pDataRoot = NULL;
      errorMessage.erase();
      fError = false;
      filesQ.clear();
      //......

      pDataRoot = pNewDataRoot;

            if (nodeName == RESOLUTIONS)
                  parseResolutions(fileName);
            else if (nodeName == COMMANDS)
                  parseCommands( fileName);
            else if (nodeName == PRINTMODES)
                  parsePrintModes(fileName);
            else if (nodeName == TRAYS)
                  parseTrays(fileName);
            else if (nodeName == FORMS)
                  parseForms(fileName);
            else if (nodeName == MEDIAS)
                  parseMedias(fileName) ;
            else if (nodeName == CONNECTIONS)
                  parseConnections(fileName);
            else if (nodeName == GAMMAS)
                  parseGammas(fileName) ;
            else if (nodeName == DATA)
                  parseDatas(fileName) ;
            else if (nodeName == ORIENTATIONS)
                  parseOrientations(fileName) ;
	   else
	         cerr << "Parser Error, unknown property " << nodeName << endl;

      if (fError)
      {
         cerr << "An error has occured during parsing " << errorMessage << endl;
         return -1;
      }

      return 0;


}


void DeviceToolSAX::
parseCommands(string fileName)
{
   XMLTree *pXmlTree;

   pXmlTree = new XMLTree();
   bool fSuccess;
   fSuccess = pXmlTree->read(fileName);

   if (fSuccess == false)
   {
      cerr << "An error has occured in parsing file " << fileName << endl;
      fError = true;
      return;
   }

   XMLParser<ParserCommands> saxP;
   string buf = pXmlTree->write_buffer();
   saxP.parse_chunk(buf);
}

void DeviceToolSAX::
parseResolutions(string fileName)
{
   XMLTree *pXmlTree;

   pXmlTree = new XMLTree();
   bool fSuccess;
   fSuccess = pXmlTree->read(fileName);

   if (fSuccess == false)
   {
      cerr << "An error has occured in parsing file " << fileName << endl;
      fError = true;
      return;
   }

   XMLParser<ParserResolutions> saxP;
   string buf = pXmlTree->write_buffer();
   saxP.parse_chunk(buf);
}

void DeviceToolSAX::
parsePrintModes(string fileName)
{
   XMLTree *pXmlTree;

   pXmlTree = new XMLTree();
   bool fSuccess;
   fSuccess = pXmlTree->read(fileName);

   if (fSuccess == false)
   {
      cerr << "An error has occured in parsing file " << fileName << endl;
      fError = true;
      return;
   }

   XMLParser<ParserPrintModes> saxP;
   string buf = pXmlTree->write_buffer();
   saxP.parse_chunk(buf);
}

void DeviceToolSAX::
parseForms(string fileName)
{
   XMLTree *pXmlTree;

   pXmlTree = new XMLTree();
   bool fSuccess;
   fSuccess = pXmlTree->read(fileName);

   if (fSuccess == false)
   {
      cerr << "An error has occured in parsing file " << fileName << endl;
      fError = true;
      return;
   }

   XMLParser<ParserForms> saxP;
   string buf = pXmlTree->write_buffer();
   saxP.parse_chunk(buf);
}

void DeviceToolSAX::
parseTrays(string fileName)
{
   XMLTree *pXmlTree;

   pXmlTree = new XMLTree();
   bool fSuccess;
   fSuccess = pXmlTree->read(fileName);

   if (fSuccess == false)
   {
      cerr << "An error has occured in parsing file " << fileName << endl;
      fError = true;
      return;
   }

   XMLParser<ParserTrays> saxP;
   string buf = pXmlTree->write_buffer();
   saxP.parse_chunk(buf);
}

void DeviceToolSAX::
parseMedias(string fileName)
{
   XMLTree *pXmlTree;

   pXmlTree = new XMLTree();
   bool fSuccess;
   fSuccess = pXmlTree->read(fileName);

   if (fSuccess == false)
   {
      cerr << "An error has occured in parsing file " << fileName << endl;
      fError = true;
      return;
   }

   XMLParser<ParserMedias> saxP;
   string buf = pXmlTree->write_buffer();
   saxP.parse_chunk(buf);
}

void DeviceToolSAX::
parseConnections(string fileName)
{
   XMLTree *pXmlTree;

   pXmlTree = new XMLTree();
   bool fSuccess;
   fSuccess = pXmlTree->read(fileName);

   if (fSuccess == false)
   {
      cerr << "An error has occured in parsing file " << fileName << endl;
      fError = true;
      return;
   }

   XMLParser<ParserConnections> saxP;
   string buf = pXmlTree->write_buffer();
   saxP.parse_chunk(buf);
}

void DeviceToolSAX::
parseGammas(string fileName)
{
   XMLTree *pXmlTree;

   pXmlTree = new XMLTree();
   bool fSuccess;
   fSuccess = pXmlTree->read(fileName);

   if (fSuccess == false)
   {
      cerr << "An error has occured in parsing file " << fileName << endl;
      fError = true;
      return;
   }

   XMLParser<ParserGammas> saxP;
   string buf = pXmlTree->write_buffer();
   saxP.parse_chunk(buf);
}

void DeviceToolSAX::
parseDatas(string fileName)
{
   XMLTree *pXmlTree;

   pXmlTree = new XMLTree();
   bool fSuccess;
   fSuccess = pXmlTree->read(fileName);

   if (fSuccess == false)
   {
      cerr << "An error has occured in parsing file " << fileName << endl;
      fError = true;
      return;
   }

   XMLParser<ParserDatas> saxP;
   string buf = pXmlTree->write_buffer();
   saxP.parse_chunk(buf);
}

void DeviceToolSAX::
parseOrientations(string fileName)
{
   XMLTree *pXmlTree;

   pXmlTree = new XMLTree();
   bool fSuccess;
   fSuccess = pXmlTree->read(fileName);

   if (fSuccess == false)
   {
      cerr << "An error has occured in parsing file " << fileName << endl;
      fError = true;
      return;
   }

   XMLParser<ParserOrientations> saxP;
   string buf = pXmlTree->write_buffer();
   saxP.parse_chunk(buf);
}
