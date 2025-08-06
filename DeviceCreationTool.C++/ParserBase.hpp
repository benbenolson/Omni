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
#ifndef _com_ibm_ltc_omni_guitool_ParserBase_hpp
#define _com_ibm_ltc_omni_guitool_ParserBase_hpp

#include <string>
#include <xml++.h>

#include "Node.hpp"
#include "RootNode.hpp"
#include "HeadNode.hpp"
#include "GeneralProperties.hpp"
#include "DefaultJobProperties.hpp"
#include "DeviceToolSAX.hpp"
#include "DeviceToolConstants.hpp"


namespace OmniDeviceCreationTool {

class ParserBase : public XMLParserCallback
{
   private:
      GeneralProperties *pGeneralProperties;
      DefaultJobProperties *pDefaultJobProperties;
      HeadNode *pResolutionsHead;
      HeadNode *pCommandsHead;
      HeadNode *pMediasHead;
      HeadNode *pPrintModesHead;
      HeadNode *pFormsHead;
      HeadNode *pConnectionsHead;
      HeadNode *pDatasHead;
      HeadNode *pGammasHead;
      HeadNode *pTraysHead;
      HeadNode *pOrientationsHead;

   protected:
      string charsRead;   // stores the strings read by the parser

      RootNode * pDataRoot;

     void clear_charsRead()
     {
         charsRead.erase();
     }

      string process_entity_reference(string input, char searchChar, string replaceStr)
      {
         int i=0;
         string::pos_type j=0;
         while(true)
         {
            j = input.find(searchChar,i);
            if ( j == string::npos )
               break;
            else
               input.replace(j, 1, replaceStr);
            i=j+1;
         }
         return input;
      }

      string process_entity_reference_amp(string input)
      {
         /* This method searches for the character & in the input string
            and replaces it with the string &amp;
            &amp; - is the Entity reference type used in XML to
            represent the character &
         */

         string output = process_entity_reference(input, '&', "&amp;");
         return output;
      }

      string process_entity_reference_apos(string input)
      {
            // find ' and replace it with &apos;
         string output = process_entity_reference(input, '\'', "&apos;");
         return output;
      }

      string process_entity_reference_lt(string input)
      {
            // find < and replace it with &lt;
         string output = process_entity_reference(input, '<', "&lt;");
         return output;
      }

      string process_entity_reference_gt(string input)
      {
            // find > and replace it with &gt;
         string output = process_entity_reference(input, '>', "&gt;");
         return output;
      }

      string process_entity_reference_quot(string input)
      {
            // find " and replace it with &quot;
         string output = process_entity_reference(input, '\"', "&quot;");
         return output;
      }


   public:

      ParserBase()
      {
            pDataRoot = DeviceToolSAX::getInstance()->getDataRoot();
            initialiseVariables();
      }

      virtual ~ParserBase ()
      {
      }

      virtual void start_element(const string &n, const XMLPropertyHash &p) {};
      virtual void end_element(const string &n) {};

      void characters(const string &s)
      {
         if (!charsRead.empty())
            charsRead = charsRead + s;
         else
            charsRead = s;
      }

      void error(const string &s)
      {
         // cerr << "An error has occured during parsing" << endl;
         DeviceToolSAX::getInstance()->setError(true,s);
      }

      void fatal_error(const string &s)
      {
         // cerr << "A fatal error has occured during parsing" << endl;
         DeviceToolSAX::getInstance()->setError(true,s);
      }


   private:
      void initialiseVariables()
      {
         vector<Node *>::iterator iter;
         for(iter=pDataRoot->getChildVector()->begin();
              iter!=pDataRoot->getChildVector()->end();
              iter++)
         {
             string name = (*iter)->getNodeName();

             if (name == GENERALPROPERTIES)
             {
                pGeneralProperties = (GeneralProperties *)(*iter);
             }
             else if (name == DEFAULTJP)
            {
                pDefaultJobProperties = (DefaultJobProperties *)(*iter);
            }
             else if (name == RESOLUTIONS)
             {
                pResolutionsHead = (HeadNode *)(*iter);
             }
             else if (name == COMMANDS)
            {
                pCommandsHead = (HeadNode *)(*iter);
            }
           else if (name == PRINTMODES)
           {
               pPrintModesHead = (HeadNode *)(*iter);
           }
           else if (name == TRAYS)
          {
              pTraysHead = (HeadNode *)(*iter);
          }
         else if (name == FORMS)
         {
              pFormsHead = (HeadNode *)(*iter);
         }
         else if (name == MEDIAS)
        {
             pMediasHead = (HeadNode *)(*iter);
        }
        else if (name == CONNECTIONS)
        {
            pConnectionsHead = (HeadNode *)(*iter);
        }
        else if (name == GAMMAS)
        {
            pGammasHead = (HeadNode *)(*iter);
        }
        else if (name == DATA)
        {
            pDatasHead = (HeadNode *)(*iter);
        }
       else if (name == ORIENTATIONS)
       {
           pOrientationsHead = (HeadNode *)(*iter);
       }
     }
   }


   protected:
      // get methods

      GeneralProperties * getGeneralProperties()
      {  return pGeneralProperties;  }

      DefaultJobProperties * getDefaultJobProperties()
      {  return pDefaultJobProperties;  }

      HeadNode* getCommandsHead()
      {  return pCommandsHead;  }

      HeadNode* getResolutionsHead()
      {   return pResolutionsHead;  }

      HeadNode* getPrintModesHead()
      {   return pPrintModesHead;   }

      HeadNode* getTraysHead()
      {  return pTraysHead;  }

      HeadNode* getFormsHead()
      {  return pFormsHead;  }

      HeadNode* getMediasHead()
      {    return pMediasHead;    }

      HeadNode* getConnectionsHead()
      {    return pConnectionsHead;   }

      HeadNode* getGammasHead()
      {    return pGammasHead;   }

      HeadNode* getDatasHead()
      {    return pDatasHead;   }

      HeadNode* getOrientationsHead()
      {    return pOrientationsHead;   }


}; // end class

}; // end of namespace

#endif
