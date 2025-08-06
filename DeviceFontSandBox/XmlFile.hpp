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
 *   Author: David L. Wagner
 *   Nov 13, 2002
 *   $Id: XmlFile.hpp,v 1.4 2004/02/17 16:23:56 hamzy Exp $
 */

#ifndef _XmlFile
#define _XmlFile

#include <iostream>
#include "XMLInterface.hpp"

namespace DevFont
{

  class XmlFile
  {

  public:
    /** Construct the XML file from the given path. */
    explicit XmlFile (const char* path);

    /** Delete the parser when we are done. */
    ~XmlFile();

    /** Find an Element with the given name starting at the given Node. */
    static XmlNodePtr findElement (const XmlNodePtr, const char *);

    /** Find the first Element with the given name. */
    XmlNodePtr findElement (const char *) const;

    /** Return the root Element of the file. */
    const XmlNodePtr getRootElement () const { return rootNode; }

    /** The UPDF file name */
    char* fileName;

  private:

     /**The xml document pointer*/
    XmlDocPtr xmlDOMDoc;

    /** The root node pointer of the DOM tree. */
    XmlNodePtr rootNode;

  };
  /** Print out the xml file name */
  std::ostream & operator<< (std::ostream &, const XmlFile &);


}

#endif
