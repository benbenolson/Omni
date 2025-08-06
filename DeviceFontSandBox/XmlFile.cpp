/*
 *   IBM Linux Device Font Library
 *   Copyright (c) International Business Machines Corp., 2002
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
 *
 *   Author: Julie Zhuo
 *   Nov 20, 2002
 *   $Id: XmlFile.cpp,v 1.9 2004/02/17 16:23:56 hamzy Exp $
 */

#include "XmlFile.hpp"
#include "DeviceFontException.hpp"
#include <iostream>
#include <string>
#include <stdlib.h>

using namespace std;
using namespace DevFont;

/**
 * The constructor parses the given XML file and remembers the root element
 * of the document.  We keep track of the parser as well, since the document
 * goes away when the parser does.
 *
 * @param path  the path to the XML file
 */
XmlFile::XmlFile (const char* path)
{
#ifdef DEBUG
  cout << "XmlFile::XmlFile(" << path << ") called" << endl;
#endif

  XMLInitialize ();

  //
  // Store file name
  //
  fileName = new char[1+strlen(path)];
  strcpy(fileName, path);

  /** COMPAT: Do not genrate nodes for formatting spaces */
  LIBXML_TEST_VERSION
    XMLKeepBlanksDefault(0);

  /*
   * build an XML tree from the file;
   */

  xmlDOMDoc = XMLParseFile(path);

  if (xmlDOMDoc == NULL)
    {
      throw DeviceFontException ("Can not Parse! Invalid UPDF File!");
    }

  /*
   * Check the document is of the right kind
   */

    rootNode = XMLDocGetRootElement(xmlDOMDoc);
  if (rootNode == NULL)
    {
      throw DeviceFontException ("UPDF File is empty!");
    }
  //cout << rootNode -> name << endl;
}


/**
 * The destructor deletes the xml DOM tree and its rootNode pointer.
 */
XmlFile::~XmlFile ()
{
  //
  // Delete the xml DOM document structure and the xml node structure
  //
  if (xmlDOMDoc != NULL)
    {
      XMLFreeDoc (xmlDOMDoc);
      xmlDOMDoc = NULL;
    }

  XMLCleanup ();

  //
  // Delete the file name
  //
  if (fileName)
    {
      delete fileName;
      fileName = 0;
    }

}


/**
 * Find an Element with the given name starting from the root of the document.
 * If we were being fancy, we should really use XPATH expressions to find a
 * particular node, but here we are relying on the fact that in UPDF files
 * there are no Elements with the same name in different contexts.
 * Otherwise, it would be likely that one would get the wrong node.
 *
 * @param elementName  name of the node to retrieve
 */
XmlNodePtr XmlFile::findElement (const char *elementName) const
{
  return findElement (rootNode, elementName);
}


/**
 * Find an Element with the given name in the tree start a the given Node.  If
 * the given node is the one we are looking for, then just return that.
 * Otherwise it returns the (first) descendent that it finds with that name
 * (therefore, this will be mostly useful only if there is exactly one Element
 * with that name in the whole tree, or if we checking to see if the given node
 * is exactly the one we want).
 *
 * @param node     starting node to search
 * @param elementName  name of the node to retrieve
 */

XmlNodePtr XmlFile::findElement (const XmlNodePtr node, const char *elementName)
{
  XmlNodePtr cur;

  if ( node )
    {
      //
      // First check to see if the node we are starting at is the node we want.
      //
      if ( strcmp ( XMLGetName (node), elementName) == 0)
	{
	  return node;
	}

      //
      // check to see if there is a child of this node that is the one we want
      //
      for (cur = XMLGetChildrenNode (node); cur != NULL; cur = XMLNextNode (cur))
        {
          //
          // Check to see if the child is the node
          //
	  XmlNodePtr tmp = findElement (cur, elementName);
          if (tmp)
            return tmp;
        }

    }
  return NULL;
}



namespace DevFont
{
  /** Print out the XmlFile name for debugging. */
  ostream & DevFont::operator<< (ostream & s, const XmlFile & file)
  {
    s << "XmlFile: " << file.fileName;
    return s;
  }
}
