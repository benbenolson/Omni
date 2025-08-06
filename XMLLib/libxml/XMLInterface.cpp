/*
 *   IBM Omni driver
 *   Copyright (c) International Business Machines Corp., 2004
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

#include "XMLInterface.hpp"
#undef LIBXML_TEST_VERSION

#include <string.h>
#include <stdarg.h>

#include <libxml/parser.h>
#include <libxml/tree.h>
//#define DTD 1
#define SCHEMA 1
#ifdef SCHEMA
#include <libxml/xmlschemas.h>
#include <libxml/xmlschemastypes.h>
#endif

void
XMLInitialize ()
{
}

void
XMLCleanup ()
{
#ifdef SCHEMA
   xmlSchemaCleanupTypes ();
#endif

   xmlCleanupParser ();
}

void
XMLKeepBlanksDefault (int iVal)
{
   xmlKeepBlanksDefault (iVal);
}

XmlNodePtr
XMLGetChildrenNode (XmlNodePtr xmlElm)
{
   xmlNodePtr elm = (xmlNodePtr)xmlElm;

   return (XmlNodePtr)elm->xmlChildrenNode;
}

XmlDocPtr
XMLGetDocNode (XmlNodePtr xmlElm)
{
   xmlNodePtr elm = (xmlNodePtr)xmlElm;

   return (XmlDocPtr)elm->doc;
}

PSZCRO
XMLGetName (XmlNodePtr xmlElm)
{
   xmlNodePtr elm = (xmlNodePtr)xmlElm;

   return (PSZCRO)elm->name;
}

XmlNodePtr
XMLGetPreviousNode (XmlNodePtr xmlElm)
{
   xmlNodePtr elm = (xmlNodePtr)xmlElm;

   return (XmlNodePtr)elm->prev;
}

XmlNodePtr
XMLGetNextNode (XmlNodePtr xmlElm)
{
   xmlNodePtr elm = (xmlNodePtr)xmlElm;

   return (XmlNodePtr)elm->next;
}

XmlElementContent
XMLGetContent (XmlNodePtr xmlElm)
{
   xmlNodePtr elm = (xmlNodePtr)xmlElm;

   return (XmlElementContent)elm->content;
}

XmlElementType
XMLGetType (XmlNodePtr xmlElm)
{
   xmlNodePtr elm = (xmlNodePtr)xmlElm;

   return (XmlElementType)elm->type;
}

XmlAttr
XMLGetProperties (XmlNodePtr xmlElm)
{
   xmlNodePtr elm = (xmlNodePtr)xmlElm;

   return (XmlAttr)elm->properties;
}

void
XMLFreeDoc (XmlDocPtr xmlDoc)
{
   xmlDocPtr doc = (xmlDocPtr)xmlDoc;

   xmlFreeDoc (doc);
}

void
XMLFreeNode (XmlNodePtr xmlElm)
{
   xmlNodePtr elm = (xmlNodePtr)xmlElm;

   xmlFreeNode (elm);
}

void
XMLFreeNodeList (XmlNodePtr xmlElm)
{
   xmlNodePtr elm = (xmlNodePtr)xmlElm;

   xmlFreeNodeList (elm);
}

PSZCRO
XMLGetProp (XmlNodePtr xmlElm, PSZCRO pszProp)
{
   xmlNodePtr elm = (xmlNodePtr)xmlElm;

   return (PSZCRO)xmlGetProp (elm, (xmlChar *)pszProp);
}

PSZCRO
XMLNodeListGetString (XmlDocPtr  xmlDoc,
                      XmlNodePtr xmlElm,
                      int        inLine)
{
   xmlDocPtr  doc = (xmlDocPtr)xmlDoc;
   xmlNodePtr elm = (xmlNodePtr)xmlElm;

   return (PSZCRO)xmlNodeListGetString (doc, elm, inLine);
}

XmlNodePtr
XMLDocGetRootElement (XmlDocPtr xmlDoc)
{
   xmlDocPtr doc = (xmlDocPtr)xmlDoc;

   return xmlDocGetRootElement (doc);
}

void
XMLNodeSetName (XmlNodePtr xmlElm, PSZCRO pszName)
{
   xmlNodePtr elm = (xmlNodePtr)xmlElm;

   xmlNodeSetName (elm, (const xmlChar *)pszName);
}

void
XMLNodeSetContent (XmlNodePtr xmlElm,
                   PSZCRO     pszContent)
{
   xmlNodePtr elm = (xmlNodePtr)xmlElm;

   xmlNodeSetContent (elm, (const xmlChar *)pszContent);
}

XmlNodePtr
XMLNewNode  (XmlNsPtr xmlNs,
             PSZRO    pszName)
{
   xmlNsPtr   ns  = (xmlNsPtr)xmlNs;

   return (XmlNodePtr)xmlNewNode (ns, (xmlChar *)pszName);
}

XmlNodePtr
XMLNewChild (XmlNodePtr xmlElm,
             XmlNsPtr   xmlNs,
             PSZCRO     pszName,
             PSZCRO     pszContent)
{
   xmlNodePtr elm = (xmlNodePtr)xmlElm;
   xmlNsPtr   ns  = (xmlNsPtr)xmlNs;

   return (XmlNodePtr)xmlNewChild (elm, ns, (xmlChar *)pszName, (xmlChar *)pszContent);
}

XmlNodePtr	
XMLNewText (PSZRO pszContent)
{
   return (XmlNodePtr)xmlNewText ((xmlChar *)pszContent);
}

XmlNodePtr
XMLNewTextChild (XmlNodePtr xmlElm,
                 XmlNsPtr   xmlNs,
                 PSZCRO     pszName,
                 PSZCRO     pszContent)
{
   xmlNodePtr elm  = (xmlNodePtr)xmlElm;
   xmlNsPtr   ns  = (xmlNsPtr)xmlNs;

   return (XmlNodePtr)xmlNewTextChild (elm, ns, (xmlChar *)pszName, (xmlChar *)pszContent);
}

XmlNodePtr	
XMLAddNextSibling (XmlNodePtr xmlElm,
                   XmlNodePtr xmlElm2)
{
   xmlNodePtr elm  = (xmlNodePtr)xmlElm;
   xmlNodePtr elm2 = (xmlNodePtr)xmlElm2;

   return (XmlNodePtr)xmlAddNextSibling (elm, elm2);
}

XmlNodePtr	
XMLAddPrevSibling (XmlNodePtr xmlElm,
                   XmlNodePtr xmlElm2)
{
   xmlNodePtr elm  = (xmlNodePtr)xmlElm;
   xmlNodePtr elm2 = (xmlNodePtr)xmlElm2;

   return (XmlNodePtr)xmlAddPrevSibling (elm, elm2);
}

XmlNodePtr
XMLAddChild (XmlNodePtr xmlElm,
             XmlNodePtr xmlElm2)
{
   xmlNodePtr elm  = (xmlNodePtr)xmlElm;
   xmlNodePtr elm2 = (xmlNodePtr)xmlElm2;

   return (XmlNodePtr)xmlAddChild (elm, elm2);
}

void
XMLUnlinkNode (XmlNodePtr xmlElm)
{
   xmlNodePtr elm  = (xmlNodePtr)xmlElm;

   xmlUnlinkNode (elm);
}

XmlNodePtr
XMLFirstNode (XmlNodePtr xmlElm)
{
   xmlNodePtr elm = (xmlNodePtr)xmlElm;

   while (  0 != elm
         && (  XML_COMMENT_NODE == elm->type
            || XML_TEXT_NODE    == elm->type
            )
         )
   {
      // Move to the next node
      elm = elm->next;
   }

   return (XmlNodePtr)elm;
}

XmlNodePtr
XMLNextNode (XmlNodePtr xmlElm)
{
   xmlNodePtr elm = (xmlNodePtr)xmlElm;

   do
   {
      // Move to the next node
      elm = elm->next;

   } while (  0 != elm
           && (  XML_COMMENT_NODE == elm->type
              || XML_TEXT_NODE    == elm->type
              )
           );

   return (XmlNodePtr)elm;
}

XmlNodePtr
XMLFindEntry (XmlNodePtr xmlRoot,
              PSZCRO     pszName,
              bool       fShouldDebugOutput)
{
   xmlNodePtr root = (xmlNodePtr)xmlRoot;

   if (root == 0)
      return 0;

   xmlNodePtr elm = (xmlNodePtr)XMLFirstNode ((XmlNodePtr)root->xmlChildrenNode);

   while (elm != 0)
   {
      if (0 == strcmp ((char *)elm->name, pszName))
      {
         break;
      }

      elm = (xmlNodePtr)XMLNextNode ((XmlNodePtr)elm);
   }

#ifndef RETAIL // @TODO @TBD
///if (fShouldDebugOutput)
///{
///   if (elm)
///   {
///      DebugOutput::getErrorStream () << __FUNCTION__ << ": Found ";
///   }
///   else
///   {
///      DebugOutput::getErrorStream () << __FUNCTION__ << ": Did not find ";
///   }
///   DebugOutput::getErrorStream () << pszName << std::endl;
///}
#endif

   return elm;
}

void
XMLPrintDocument (XmlDocPtr xmlDoc, int iLevel)
{
   xmlDocPtr doc = (xmlDocPtr)xmlDoc;

   xmlDocDump (stdout, doc);
}

int
XMLDocDump (FILE *fp, XmlDocPtr xmlDoc)
{
   xmlDocPtr doc = (xmlDocPtr)xmlDoc;

   return xmlDocDump (fp, doc);
}

void
XMLPrintNode (FILE       *pfpOut,
              XmlNodePtr  xmlElm)
{
   xmlNodePtr elm = (xmlNodePtr)xmlElm;

   xmlElemDump (pfpOut, elm->doc, elm);
   fprintf (pfpOut, "\n");
}

void
XMLElemDump (FILE       *pfpOut,
             XmlNodePtr  xmlElm)
{
   xmlNodePtr elm = (xmlNodePtr)xmlElm;

   xmlElemDump (pfpOut, elm->doc, elm);
}

XmlDocPtr
XMLParseFile (PSZCRO pszXMLFile)
{
   XmlDocPtr xmlDoc = 0;

   xmlDoc = (XmlDocPtr)xmlParseFile (pszXMLFile);

   return xmlDoc;
}

void
MyValidityErrorFunc (void *ctx, const char *msg, ...)
{
   va_list list;

   va_start (list, msg);

   printf ("Error: ");
   vprintf (msg, list);

   va_end (list);

   if (ctx)
   {
      XmlParseError *pParseError = (XmlParseError *)ctx;

      pParseError->setErrorCondition ();
   }
}

void
MyValidityWarningFunc (void *ctx, const char *msg, ...)
{
   va_list list;

   va_start (list, msg);

   printf ("Warning: ");
   vprintf (msg, list);

   va_end (list);
}

bool
XMLValidateFile (XmlDocPtr            xmlDoc,
                 XmlParseError       *pParseError)
{
#if 0
   xmlDocPtr           doc             = (xmlDocPtr)xmlDoc;
#endif
#ifdef DTD
   xmlValidCtxt        validatorDTD    = {
      pParseError,
      MyValidityErrorFunc,
      MyValidityWarningFunc
   };
#endif
#if 0 //def SCHEMA
   xmlSchemaValidCtxt  validatorSchema = {
      pParseError,
      MyValidityErrorFunc,
      MyValidityWarningFunc
   };
#endif

#ifdef DTD
   xmlValidateDocument (&validatorDTD, doc);
#endif
#if 0 //def SCHEMA
   xmlSchemaValidateDoc (&validatorSchema, doc);
#endif
#if 0 //def SCHEMA
   xmlSchemaPtr            schema        = 0;
   xmlSchemaParserCtxtPtr  ctxtParser    = 0;
   xmlSchemaValidCtxtPtr   ctxtValid     = 0;
   const char             *pszSchemaFile = "../OmniDevice.xsd";
   int                     iRC           = 0;

   ctxtParser = xmlSchemaNewParserCtxt (pszSchemaFile);
   std::cerr << "ctxtParser = " << std::hex << (int)ctxtParser << std::dec << std::endl;
   xmlSchemaSetParserErrors (ctxtParser,
                             (xmlSchemaValidityErrorFunc)MyValidityErrorFunc,
                             (xmlSchemaValidityWarningFunc)MyValidityWarningFunc,
                             0);
   schema = xmlSchemaParse (ctxtParser);
   std::cerr << "schema = " << std::hex << (int)schema << std::dec << std::endl;
   xmlSchemaFreeParserCtxt (ctxtParser);

   ctxtValid = xmlSchemaNewValidCtxt (schema);
   std::cerr << "ctxtValid = " << std::hex << (int)ctxtValid << std::dec << std::endl;
   xmlSchemaSetValidErrors (ctxtValid,
                            (xmlSchemaValidityErrorFunc)MyValidityErrorFunc,
                            (xmlSchemaValidityWarningFunc)MyValidityWarningFunc,
                            0);
   iRC = xmlSchemaValidateDoc (ctxtValid, doc_d);
   std::cerr << "iRC = " << iRC << std::endl;
   xmlSchemaFreeValidCtxt (ctxtValid);
   xmlSchemaFree (schema);
#endif

   return true; // @TBD @HACK
}

int
XMLStrcmp (PSZCRO pszString1, PSZCRO pszString2)
{
   return xmlStrcmp ((const xmlChar *)pszString1, (const xmlChar *)pszString2);
}

int
XMLSaveFile (PSZCRO pszFileName, XmlDocPtr xmlDoc)
{
   xmlDocPtr doc = (xmlDocPtr)xmlDoc;

   return xmlSaveFile (pszFileName, doc);
}

void
XMLFree (void *pvPtr)
{
   xmlFree (pvPtr);
}

XmlAttrPtr
XMLNewProp (XmlNodePtr xmlElm,
            PSZCRO     pszName,
            PSZCRO     pszValue)
{
   xmlNodePtr elm = (xmlNodePtr)xmlElm;

   return (XmlAttrPtr)xmlNewProp (elm, (xmlChar *)pszName, (xmlChar *)pszValue);
}

XmlNsPtr *
XMLGetNsList (XmlDocPtr  xmlDoc,
              XmlNodePtr xmlElm)
{
   xmlDocPtr  doc = (xmlDocPtr)xmlDoc;
   xmlNodePtr elm = (xmlNodePtr)xmlElm;

   return (XmlNsPtr *)xmlGetNsList (doc, elm);
}

int
XMLUnsetNsProp (XmlNodePtr xmlElm,
                XmlNsPtr   xmlNs,
                PSZCRO     pszName)
{
   xmlNodePtr elm = (xmlNodePtr)xmlElm;
   xmlNsPtr   ns  = (xmlNsPtr)xmlNs;

   return xmlUnsetNsProp (elm, ns, (xmlChar *)pszName);
}

XmlNsPtr
XMLNewNs (XmlNodePtr xmlElm,
          PSZCRO     pszHref,
          PSZCRO     pszPrefix)
{
   xmlNodePtr elm = (xmlNodePtr)xmlElm;

   return (XmlNsPtr)xmlNewNs (elm, (xmlChar *)pszHref, (xmlChar *)pszPrefix);
}

void
XMLSetNs (XmlNodePtr xmlElm,
          XmlNsPtr   xmlNs)
{
   xmlNodePtr elm = (xmlNodePtr)xmlElm;
   xmlNsPtr   ns  = (xmlNsPtr)xmlNs;

   xmlSetNs (elm, ns);
}

PSZCRO
XMLEncodeEntitiesReentrant (XmlDocPtr xmlDoc,
                            PSZCRO    pszInput)
{
   xmlDocPtr  doc = (xmlDocPtr)xmlDoc;

   return (PSZCRO)xmlEncodeEntitiesReentrant (doc, (xmlChar *)pszInput);
}

PSZCRO
XMLBufferContent (XmlBufferPtr xmlBuf)
{
   xmlBufferPtr buf = (xmlBufferPtr)xmlBuf;

   return (PSZCRO)xmlBufferContent (buf);
}

XmlAttrPtr
XMLSetProp (XmlNodePtr xmlElm,
            PSZCRO     pszName,
            PSZCRO     pszValue)
{
   xmlNodePtr elm = (xmlNodePtr)xmlElm;

   return (XmlAttrPtr)xmlSetProp (elm, (xmlChar *)pszName, (xmlChar *)pszValue);
}
