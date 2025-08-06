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
#ifndef _XMLInterface
#define _XMLInterface

#include "defines.hpp"
#include <stdio.h>

#define LIBXML_TEST_VERSION

typedef void *XmlNodePtr;
typedef void *XmlDocPtr;
typedef void *XmlNsPtr;
typedef void *XmlAttrPtr;
typedef void *XmlBufferPtr;
typedef void *XmlElementContent;
typedef void *XmlAttr;

typedef enum {
    XMLLIB_ELEMENT_NODE        =  1,
    XMLLIB_ATTRIBUTE_NODE      =  2,
    XMLLIB_TEXT_NODE           =  3,
    XMLLIB_CDATA_SECTION_NODE  =  4,
    XMLLIB_ENTITY_REF_NODE     =  5,
    XMLLIB_ENTITY_NODE         =  6,
    XMLLIB_PI_NODE             =  7,
    XMLLIB_COMMENT_NODE        =  8,
    XMLLIB_DOCUMENT_NODE       =  9,
    XMLLIB_DOCUMENT_TYPE_NODE  = 10,
    XMLLIB_DOCUMENT_FRAG_NODE  = 11,
    XMLLIB_NOTATION_NODE       = 12,
    XMLLIB_HTML_DOCUMENT_NODE  = 13,
    XMLLIB_DTD_NODE            = 14,
    XMLLIB_ELEMENT_DECL        = 15,
    XMLLIB_ATTRIBUTE_DECL      = 16,
    XMLLIB_ENTITY_DECL         = 17,
    XMLLIB_NAMESPACE_DECL      = 18,
    XMLLIB_XINCLUDE_START      = 19,
    XMLLIB_XINCLUDE_END        = 20
} XmlElementType;

typedef enum {
    XMLLIB_ATTRIBUTE_CDATA        = 1,
    XMLLIB_ATTRIBUTE_ID,
    XMLLIB_ATTRIBUTE_IDREF	,
    XMLLIB_ATTRIBUTE_IDREFS,
    XMLLIB_ATTRIBUTE_ENTITY,
    XMLLIB_ATTRIBUTE_ENTITIES,
    XMLLIB_ATTRIBUTE_NMTOKEN,
    XMLLIB_ATTRIBUTE_NMTOKENS,
    XMLLIB_ATTRIBUTE_ENUMERATION,
    XMLLIB_ATTRIBUTE_NOTATION
} XmlAttributeType;

class XmlParseError
{
public:
   virtual void setErrorCondition () = 0;
};

void               XMLInitialize              ();
void               XMLCleanup                 ();
void               XMLKeepBlanksDefault       (int                    iVal);
XmlNodePtr         XMLGetChildrenNode         (XmlNodePtr             xmlElm);
XmlDocPtr          XMLGetDocNode              (XmlNodePtr             xmlElm);
PSZCRO             XMLGetName                 (XmlNodePtr             xmlElm);
XmlNodePtr         XMLGetPreviousNode         (XmlNodePtr             xmlElm);
XmlNodePtr         XMLGetNextNode             (XmlNodePtr             xmlElm);
XmlElementContent  XMLGetContent              (XmlNodePtr             xmlElm);
XmlElementType     XMLGetType                 (XmlNodePtr             xmlElm);
XmlAttr            XMLGetProperties           (XmlNodePtr             xmlElm);
void               XMLFreeDoc                 (XmlDocPtr              xmlDoc);
void               XMLFreeNode                (XmlNodePtr             xmlElm);
void               XMLFreeNodeList            (XmlNodePtr             xmlElm);
PSZCRO             XMLGetProp                 (XmlNodePtr             xmlElm,
                                               PSZCRO                 pszProp);
PSZCRO             XMLNodeListGetString       (XmlDocPtr              xmlDoc,
                                               XmlNodePtr             xmlElm,
                                               int                    inLine);
XmlNodePtr         XMLDocGetRootElement       (XmlDocPtr              xmlDoc);
void               XMLNodeSetName             (XmlNodePtr             xmlElm,
                                               PSZCRO                 pszName);
void		          XMLNodeSetContent          (XmlNodePtr             xmlElm,
                                               PSZCRO                 pszContent);
XmlNodePtr         XMLNewNode		             (XmlNsPtr               xmlNs,
                                               PSZRO                  pszName);
XmlNodePtr         XMLNewChild                (XmlNodePtr             xmlElm,
                                               XmlNsPtr               xmlNs,
                                               PSZCRO                 pszName,
                                               PSZCRO                 pszContent);
XmlNodePtr	       XMLNewText                 (PSZRO                  pszContent);
XmlNodePtr	       XMLNewTextChild            (XmlNodePtr             xmlElm,
                                               XmlNsPtr               xmlNs,
                                               PSZCRO                 pszName,
                                               PSZCRO                 pszContent);
XmlNodePtr	       XMLAddNextSibling          (XmlNodePtr             xmlElm,
                                               XmlNodePtr             xmlElm2);
XmlNodePtr	       XMLAddPrevSibling          (XmlNodePtr             xmlElm,
                                               XmlNodePtr             xmlElm2);
XmlNodePtr	       XMLAddChild                (XmlNodePtr             xmlElm,
                                               XmlNodePtr             xmlElm2);
void		          XMLUnlinkNode              (XmlNodePtr             xmlElm);
XmlNodePtr         XMLFirstNode               (XmlNodePtr             xmlElm);
XmlNodePtr         XMLNextNode                (XmlNodePtr             xmlElm);
XmlNodePtr         XMLFindEntry               (XmlNodePtr             xmlRoot,
                                               PSZCRO                 pszName,
                                               bool                   fShouldDebugOutput);
void               XMLPrintDocument           (XmlDocPtr              xmlDoc,
                                               int                    iLevel);
int                XMLDocDump                 (FILE                  *fp,
                                               XmlDocPtr              xmlDoc);
void               XMLPrintNode               (FILE                  *pfpOut,
                                               XmlNodePtr             xmlElm);
void               XMLElemDump                (FILE                  *pfpOut,
                                               XmlNodePtr             xmlElm);
XmlDocPtr          XMLParseFile               (PSZCRO                 pszXMLFile);
bool               XMLValidateFile            (XmlDocPtr              xmlDoc,
                                               XmlParseError         *pParseError);
int                XMLStrcmp                  (PSZCRO                 pszString1,
                                               PSZCRO                 pszString2);
int                XMLSaveFile                (PSZCRO                 pszFileName,
                                               XmlDocPtr              xmlDoc);
void               XMLFree                    (void                  *pvPtr);
XmlAttrPtr	       XMLNewProp                 (XmlNodePtr             xmlElm,
                                               PSZCRO                 pszName,
                                               PSZCRO                 pszValue);
XmlNsPtr          *XMLGetNsList               (XmlDocPtr              xmlDoc,
                                               XmlNodePtr             xmlElm);
int		          XMLUnsetNsProp             (XmlNodePtr             xmlElm,
                                               XmlNsPtr               xmlNs,
                                               PSZCRO                 pszName);
XmlNsPtr	          XMLNewNs                   (XmlNodePtr             xmlElm,
                                               PSZCRO                 pszHref,
                                               PSZCRO                 pszPrefix);
void		          XMLSetNs                   (XmlNodePtr             xmlElm,
                                               XmlNsPtr               xmlNs);
PSZCRO             XMLEncodeEntitiesReentrant (XmlDocPtr              xmlDoc,
                                               PSZCRO                 pszInput);
PSZCRO             XMLBufferContent           (XmlBufferPtr           xmlBuf);
XmlAttrPtr         XMLSetProp                 (XmlNodePtr             xmlElm,
                                               PSZCRO                 pszName,
                                               PSZCRO                 pszValue);

#endif
