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
#include <fstream>
#include <cstdio>
#include <cstdlib>
#include <stdarg.h>
#include <memory.h>
#include <glob.h>

#include "XMLInterface.hpp"

char *
getElementType (XmlElementType type)
{
   switch (type)
   {
   case XMLLIB_ELEMENT_NODE:       return "XMLLIB_ELEMENT_NODE";
   case XMLLIB_ATTRIBUTE_NODE:     return "XMLLIB_ATTRIBUTE_NODE";
   case XMLLIB_TEXT_NODE:          return "XMLLIB_TEXT_NODE";
   case XMLLIB_CDATA_SECTION_NODE: return "XMLLIB_CDATA_SECTION_NODE";
   case XMLLIB_ENTITY_REF_NODE:    return "XMLLIB_ENTITY_REF_NODE";
   case XMLLIB_ENTITY_NODE:        return "XMLLIB_ENTITY_NODE";
   case XMLLIB_PI_NODE:            return "XMLLIB_PI_NODE";
   case XMLLIB_COMMENT_NODE:       return "XMLLIB_COMMENT_NODE";
   case XMLLIB_DOCUMENT_NODE:      return "XMLLIB_DOCUMENT_NODE";
   case XMLLIB_DOCUMENT_TYPE_NODE: return "XMLLIB_DOCUMENT_TYPE_NODE";
   case XMLLIB_DOCUMENT_FRAG_NODE: return "XMLLIB_DOCUMENT_FRAG_NODE";
   case XMLLIB_NOTATION_NODE:      return "XMLLIB_NOTATION_NODE";
   case XMLLIB_HTML_DOCUMENT_NODE: return "XMLLIB_HTML_DOCUMENT_NODE";
   default:                     return "???";
   }
}

char *
getAttributeType (XmlAttributeType type)
{
   switch (type)
   {
   case XMLLIB_ATTRIBUTE_CDATA:       return "XMLLIB_ATTRIBUTE_CDATA";
   case XMLLIB_ATTRIBUTE_ID:          return "XMLLIB_ATTRIBUTE_ID";
   case XMLLIB_ATTRIBUTE_IDREF:       return "XMLLIB_ATTRIBUTE_IDREF";
   case XMLLIB_ATTRIBUTE_IDREFS:      return "XMLLIB_ATTRIBUTE_IDREFS";
   case XMLLIB_ATTRIBUTE_ENTITY:      return "XMLLIB_ATTRIBUTE_ENTITY";
   case XMLLIB_ATTRIBUTE_ENTITIES:    return "XMLLIB_ATTRIBUTE_ENTITIES";
   case XMLLIB_ATTRIBUTE_NMTOKEN:     return "XMLLIB_ATTRIBUTE_NMTOKEN";
   case XMLLIB_ATTRIBUTE_NMTOKENS:    return "XMLLIB_ATTRIBUTE_NMTOKENS";
   case XMLLIB_ATTRIBUTE_ENUMERATION: return "XMLLIB_ATTRIBUTE_ENUMERATION";
   case XMLLIB_ATTRIBUTE_NOTATION:    return "XMLLIB_ATTRIBUTE_NOTATION";
   default:                        return "???";
   }
}

void
dumpNode (XmlNodePtr cur, int iLevel)
{
   static char achSpaces[] = "                                                                                            ";

   iLevel = strlen (achSpaces) - iLevel;

   printf ("%sname       = %s\n",   achSpaces+iLevel, XMLGetName (cur));
   printf ("%stype       = %s\n",   achSpaces+iLevel, getElementType (XMLGetType (cur)));
   printf ("%schildren   = %08X\n", achSpaces+iLevel, (int)XMLGetChildrenNode (cur));
   printf ("%snext       = %08X\n", achSpaces+iLevel, (int)XMLGetNextNode (cur));
   printf ("%sproperties = %08X\n", achSpaces+iLevel, (int)XMLGetProperties (cur));
   printf ("%scontent    = %s\n",   achSpaces+iLevel, XMLGetContent (cur));
}

void
printNode (XmlNodePtr cur)
{
   XmlNodePtr node;

   if (NULL == cur)
      return;

   node = cur;

   do
   {
      if (XMLGetName (node))
        printf ("%s (%s)\n",
                getElementType (XMLGetType (node)),
                XMLGetName (node));
      else
         printf ("%s (null)\n",
                 getElementType (XMLGetType (node)));

      PSZRO buffer;

#ifndef XMLLIB_USE_BUFFER_CONTENT
      buffer = XMLEncodeEntitiesReentrant (XMLGetDocNode (node), (PSZCRO)XMLGetContent (node));
#else
      buffer = XMLEncodeEntitiesReentrant (XMLGetDocNode (node), XMLBufferContent (XMLGetContent (node)));
#endif

      if (buffer)
      {
         printf("content: %s\n", buffer);

         XMLFree ((void *)buffer);
      }

      PSZRO value;

      value = XMLNodeListGetString (XMLGetDocNode (node), XMLGetChildrenNode (node), 1);

      if (value)
      {
         printf ("hrm: %s\n", value);

         XMLFree ((void *)value);
      }

      if (XMLGetProperties (node))
      {
         XmlAttrPtr attr = XMLGetProperties (node);

         do
         {
            printf ("property name (%s) type (%s)\n",
                    XMLGetName (attr),
                    getElementType (XMLGetType (attr)));
#if 0
            printf("value is\n");
            printf("-----\n");
            printNode (attr->val);
#endif
            printf("-----\n");

            attr = XMLGetNextNode (attr);

         } while (attr);

      }

      node = XMLGetNextNode (node);

   } while (node);

   if (XMLGetChildrenNode (cur))
   {
      printNode (XMLGetChildrenNode (cur));
   }
}

char *
bundleStringData (XmlNodePtr nodeItem)
{
   return (char *)XMLNodeListGetString (XMLGetDocNode (nodeItem), XMLGetChildrenNode (nodeItem), 1);
}

void
printNodeName (XmlNodePtr cur)
{
   const char *pszNodeName = 0;

   pszNodeName = bundleStringData (cur);

   std::cerr << "Node <"
             << XMLGetName (cur)
             << " "
             << pszNodeName
             << ">"
             << std::endl;

   if (pszNodeName)
   {
      XMLFree ((void *)pszNodeName);
   }
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
      *((bool *)ctx) = false;
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

XmlNodePtr
XMLNextNode (XmlNodePtr elm)
{
   do
   {
      // Move to the next node
      elm = XMLGetNextNode (elm);

   } while (  0 !=  elm
           && XMLLIB_COMMENT_NODE == XMLGetType (elm)
           );

   return elm;
}

bool
fixCommands (char      *pszFilename,
             XmlDocPtr  doc,
             XmlNodePtr deviceCommandsNode)
{
   XmlNodePtr elem;

   if (!deviceCommandsNode)
      return false;

   XmlNodePtr deviceCommandNode = XMLGetChildrenNode (deviceCommandsNode);

   if (deviceCommandNode == 0)
      return false;

///while (deviceCommandNode != 0)
///{
///   dumpNode (deviceCommandNode, 0);
///   dumpNode (XMLGetChildrenNode (deviceCommandNode), 3);
///
///   deviceCommandNode = XMLGetNextNode (deviceCommandNode);
///}

   // XMLNewNode (ns, name);
   // XmlNewChild (parent, ns, name, content);
   // XMLNodeSetName (node, name);
   // XMLSetProp (node, name, value);

   elem = XMLNewChild (deviceCommandsNode, NULL, "command", "_ESC_ \"2\"");
   XMLSetProp (elem, "name", "cmdSetSixthLineSpacing");

   elem = XMLNewChild (deviceCommandsNode, NULL, "command", "_ESC_ \"C%c\"");
   XMLSetProp (elem, "name", "cmdSetPageLengthInLines");

   FILE *fp = fopen (pszFilename, "w");

   printf ("fp = %08x\n", (int)fp);

   if (fp)
   {
      XMLDocDump (fp, doc);

      fclose (fp);
   }

   return true;
}

bool
fixDriver (char      *pszFilename,
           XmlDocPtr  doc,
           XmlNodePtr driverNode)
{
   const char *pszDriverName = 0;

   if (!driverNode)
      return false;

   XmlNodePtr nodeItem = XMLGetChildrenNode (driverNode);

   if (nodeItem == 0)
      return false;

   pszDriverName = bundleStringData (nodeItem);

   // Move to the next node
   nodeItem = XMLNextNode (nodeItem);
   if (nodeItem == NULL)
      return false;

   while (  NULL != nodeItem
         && 0 == strcmp ((char *)XMLGetName (nodeItem), "Capability")
         )
   {
      // Move to the next node
      nodeItem = XMLNextNode (nodeItem);
   }

   if (nodeItem == NULL)
      return false;

   while (  NULL != nodeItem
         && 0 == strcmp ((char *)XMLGetName (nodeItem), "RasterCapabilities")
         )
   {
      // Move to the next node
      nodeItem = XMLNextNode (nodeItem);
   }

   if (nodeItem == NULL)
      return false;

   while (  NULL != nodeItem
         && 0 == strcmp ((char *)XMLGetName (nodeItem), "DeviceOptions")
         )
   {
      // Move to the next node
      nodeItem = XMLNextNode (nodeItem);
   }

   if (nodeItem == NULL)
      return false;

   // Move to the next node
   nodeItem = XMLNextNode (nodeItem);
   if (nodeItem == NULL)
      return false;

// printNodeName (nodeItem);

   XmlNodePtr nodeNew;
   XmlNodePtr nodeChild;
   XmlNodePtr nodeInsertTo;
   char       achText[512]; // @TBD

   sprintf (achText, "%s Orientations.xml", pszDriverName);

   nodeNew      = XMLNewNode (0, "Has");
   nodeChild    = XMLNewText (achText);
   nodeInsertTo = nodeItem;

   while (  NULL != nodeItem
         && (  0 == strcmp ((char *)XMLGetName (nodeItem), "Uses")
            || 0 == strcmp ((char *)XMLGetName (nodeItem), "Has")
            )
         )
   {
      const char *pszText = 0;

      pszText = bundleStringData (nodeItem);

      if (0 == strcmp (pszText, achText))
      {
         nodeInsertTo = 0;
      }

      // Move to the next node
      nodeItem = XMLNextNode (nodeItem);

      if (pszText)
      {
         XMLFree ((void *)pszText);
      }
   }

   if (nodeInsertTo != NULL)
   {
      XMLAddChild (nodeNew, nodeChild);
      XMLAddPrevSibling (nodeInsertTo, nodeNew);

      XMLSaveFile (pszFilename, doc);
   }

   if (pszDriverName)
   {
      XMLFree ((void *)pszDriverName);
   }

   return true;
}

bool
fixForm (char      *pszFilename,
         XmlDocPtr  doc,
         XmlNodePtr formNodes)
{
   if (!formNodes)
      return false;

   XmlNodePtr formNode = XMLGetChildrenNode (formNodes);

   if (formNode == 0)
      return false;

   while (formNode != 0)
   {
      XmlNodePtr nodeItem = XMLGetChildrenNode (formNode);

      // Move to the next one
      nodeItem = XMLNextNode (nodeItem);
      if (nodeItem == 0)
         return false;

      // Move to the next one
      nodeItem = XMLNextNode (nodeItem);
      if (nodeItem == 0)
         return false;

      // Move to the next one
      nodeItem = XMLNextNode (nodeItem);
      if (nodeItem == 0)
         return false;

      // Descend HCC
      nodeItem = XMLGetChildrenNode (nodeItem);

      const char *pszNodeValue = 0;
      char        achValue[64];     // @TBD

      // hardCopyCapLeft
      pszNodeValue = bundleStringData (nodeItem);

      sprintf (achValue, "%d", atoi (pszNodeValue) * 10);

      XMLNodeSetContent (XMLGetChildrenNode (nodeItem), achValue);

      if (pszNodeValue)
      {
         XMLFree ((void *)pszNodeValue);
      }

      // Move to the next one
      nodeItem = XMLNextNode (nodeItem);
      if (nodeItem == 0)
         return false;

      // hardCopyCapTop
      pszNodeValue = bundleStringData (nodeItem);

      sprintf (achValue, "%d", atoi (pszNodeValue) * 10);

      XMLNodeSetContent (XMLGetChildrenNode (nodeItem), achValue);

      if (pszNodeValue)
      {
         XMLFree ((void *)pszNodeValue);
      }

      // Move to the next one
      nodeItem = XMLNextNode (nodeItem);
      if (nodeItem == 0)
         return false;

      // hardCopyCapRight
      pszNodeValue = bundleStringData (nodeItem);

      sprintf (achValue, "%d", atoi (pszNodeValue) * 10);

      XMLNodeSetContent (XMLGetChildrenNode (nodeItem), achValue);

      if (pszNodeValue)
      {
         XMLFree ((void *)pszNodeValue);
      }

      // Move to the next one
      nodeItem = XMLNextNode (nodeItem);
      if (nodeItem == 0)
         return false;

      // hardCopyCapBottom
      pszNodeValue = bundleStringData (nodeItem);

      sprintf (achValue, "%d", atoi (pszNodeValue) * 10);

      XMLNodeSetContent (XMLGetChildrenNode (nodeItem), achValue);

      if (pszNodeValue)
      {
         XMLFree ((void *)pszNodeValue);
      }

      // Get the next deviceForm node
      formNode = XMLNextNode (formNode);
   }

///XMLDocDump (stdout, doc);
   XMLSaveFile (pszFilename, doc);

   return true;
}

void
findAndFixCommands ()
{
   int          rc;
   glob_t       globbuf;
   XmlDocPtr    doc;
   XmlNodePtr   root;
   bool         fValid;
   char        *pszFilename = 0;
///xmlValidCtxt validator   = {
///   &fValid,
///   MyValidityErrorFunc,
///   MyValidityWarningFunc
///};

   memset (&globbuf, 0, sizeof (globbuf));
   rc = glob ("* Commands.xml", 0, 0, &globbuf);

   if (0 == rc)
   {
      for (size_t i = 0; i < globbuf.gl_pathc; i++)
      {
         pszFilename = globbuf.gl_pathv[i];

         printf ("%s\n", pszFilename);

         doc = XMLParseFile (pszFilename);

         if (NULL == doc)
            return;

         fValid = true;
/////////xmlValidateDocument (&validator, doc);

         if (!fValid)
            return;

         root = XMLDocGetRootElement (doc);

         if (0 == strcmp ((char *)XMLGetName (root), "deviceCommands"))
            fixCommands (pszFilename,
                         doc,
                         root);

         XMLFreeDoc (doc);
      }
   }

   globfree (&globbuf);
}

void
findAndFixDrivers ()
{
   int          rc;
   glob_t       globbuf;
   XmlDocPtr    doc;
   XmlNodePtr   root;
   bool         fValid;
   char        *pszFilename = 0;
///xmlValidCtxt validator   = {
///   &fValid,
///   MyValidityErrorFunc,
///   MyValidityWarningFunc
///};

   memset (&globbuf, 0, sizeof (globbuf));
   rc = glob ("*.xml", 0, 0, &globbuf);

   if (0 == rc)
   {
      for (size_t i = 0; i < globbuf.gl_pathc; i++)
      {
         pszFilename = globbuf.gl_pathv[i];

         printf ("%s\n", pszFilename);

         doc = XMLParseFile (pszFilename);

         if (NULL == doc)
            return;

         fValid = true;
/////////xmlValidateDocument (&validator, doc);

         if (!fValid)
            return;

         root = XMLDocGetRootElement (doc);

         if (0 == strcmp ((char *)XMLGetName (root), "Device"))
            fixDriver (pszFilename,
                       doc,
                       root);

         XMLFreeDoc (doc);
      }
   }

   globfree (&globbuf);
}

void
findAndFixForms ()
{
   int          rc;
   glob_t       globbuf;
   XmlDocPtr    doc;
   XmlNodePtr   root;
   bool         fValid;
   char        *pszFilename = 0;
///xmlValidCtxt validator   = {
///   &fValid,
///   MyValidityErrorFunc,
///   MyValidityWarningFunc
///};

   memset (&globbuf, 0, sizeof (globbuf));
   rc = glob ("* Forms.xml", 0, 0, &globbuf);

   if (0 == rc)
   {
      for (size_t i = 0; i < globbuf.gl_pathc; i++)
      {
         pszFilename = globbuf.gl_pathv[i];

         printf ("%s\n", pszFilename);

         doc = XMLParseFile (pszFilename);

         if (NULL == doc)
            return;

         fValid = true;
/////////xmlValidateDocument (&validator, doc);

         if (!fValid)
            return;

         root = XMLDocGetRootElement (doc);

         if (0 == strcmp ((char *)XMLGetName (root), "deviceForms"))
            fixForm (pszFilename,
                     doc,
                     root);

         XMLFreeDoc (doc);
      }
   }

   globfree (&globbuf);
}

int
main (int argc, char *argv[])
{
   findAndFixDrivers ();
   findAndFixForms ();

   return 0;
}
