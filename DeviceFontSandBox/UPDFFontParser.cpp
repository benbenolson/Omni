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
 *   Nov. 25, 2002
 *   $Id: UPDFFontParser.cpp,v 1.8 2004/02/17 16:23:56 hamzy Exp $
 */

#include "UPDFFontParser.hpp"
#include "DeviceFont.hpp"
#include "CommandSequence.hpp"
#include "Event.hpp"
#include "Panose.hpp"
#include "Parameter.hpp"
#include "DeviceFontException.hpp"

#include <iostream>
#include <string>
#include <stdio.h>
#include <sys/stat.h>


using namespace std;
using namespace DevFont;

/**
 * Construct a new parser object that will read in the indicated file.  It
 * will then construct a list of DeviceFont objects and a list of command
 * sequence strings.  The path must point to the top-level UPDF Device
 * Configuration file; this object will then read the Device Description file
 * to get the font information and the Command Sequence file.
 *
 * Since the XmlFile objects hold on to the DOM tree for their entire lifetime,
 * we try to delete the objects as soon as we possibly can for memory
 * efficiency.
 *
 * @param dcPath  path to the Device Configuration file
 */
UPDFFontParser::UPDFFontParser (const char *dcPath)
{
  //
  // First verify that the configuration file exists
  //
  struct stat dcFileStat;
  if ( -1 == stat (dcPath, &dcFileStat))
    throw DeviceFontException ("Device UPDF configuration file not found!");
  //
  // Open the Device Configuration file
  //
  XmlFile *dcFile = new XmlFile (dcPath);
  const XmlNodePtr dcElement = dcFile->findElement ("DeviceConfiguration");
  //
  // Get the names of the files containing the Device Description and the
  // Command Sequences
  //
  const char *ddName = getAttribute(dcElement, "Unit_Description_Reference");
  const char *csName = getAttribute(dcElement, "CommandSequences_Reference");
  //
  // Construct paths to the files, using the Device Configuration file path
  // as the base.
  //
  const char *ddPath = getPath (dcPath, ddName);
  const char *csPath = getPath (dcPath, csName);
  //
  // Run the common initialization
  //
  init (ddPath, csPath);
  //
  // And clean up
  //
  delete [] ddPath;
  delete [] csPath;
  delete dcFile;
}


/**
 * Constructor that takes paths to the Device Description and Command
 * Sequence files.
 *
 * @param ddPath  path to the Device Description file
 * @param csPath  path to the Device Description file
 */
UPDFFontParser::UPDFFontParser (const char *ddPath, const char *csPath)
{
    //
    // Run the common initialization
    //
    init (ddPath, csPath);
}

/**
 * Common initialize function shared by both constructors.
 */
void UPDFFontParser::init (const char *ddPath, const char *csPath)
{
    //
    // First verify that the files exist
    //
    struct stat ddFileStat;
    if ( -1 == stat (ddPath, &ddFileStat))
        throw DeviceFontException ("Device UPDF description file not found!");

    struct stat csFileStat;
    if ( -1 == stat (csPath, &csFileStat))
        throw DeviceFontException ("Device UPDF command sequence file not found!");
    //
    // Parse the Device Configuration file
    //
    XmlFile *ddFile = new XmlFile(ddPath);
    const XmlNodePtr ddRootElement = ddFile->getRootElement ();
    myFonts = findFonts (ddRootElement);
    myEvents = findEvents (ddRootElement);
    delete ddFile;
    //
    // Parse the Command Sequences file
    //
    XmlFile *csFile = new XmlFile(csPath);
    const XmlNodePtr csRootElement = csFile->getRootElement ();
    myCmdSeqs = findCmdSeqs (csRootElement);
    myParameters = findParameters (csRootElement);
    delete csFile;
}


/**
 * The destructor must get rid of all the objects this class owns.
 */
UPDFFontParser::~UPDFFontParser()
{
  //
  // Delete all the fonts
  //
  if (myFonts != NULL)
    {
      const DeviceFont *current = myFonts;
      while (current != NULL)
        {
          const DeviceFont *temp = current->getNext();
          delete current;
          current = temp;
        }
    }

  //
  // Delete all the command sequences
  //
  if (myCmdSeqs != NULL)
    {
      const CommandSequence *current = myCmdSeqs;
      while (current != NULL)
        {
          const CommandSequence *temp = current->getNext();
          delete current;
          current = temp;
        }
    }

  //
  // Delete all the events
  //
  if (myEvents != NULL)
    {
      const Event *current = myEvents;
      while (current != NULL)
        {
          const Event *temp = current->getNext();
          delete current;
          current = temp;
        }
    }

  //
  // Set these to null for good measure
  //
  myFonts = NULL;
  myCmdSeqs = NULL;
  myEvents = NULL;
}


/**
 * Find all the DeviceFont elements and return a linked list of DeviceFont
 * objects with all their information.  Or it will return NULL if there is
 * no font information, but this may be a bit pathological.
 */
DeviceFont *UPDFFontParser::findFonts (const XmlNodePtr root) const
{
#ifdef DEBUG
  cout << "UPDFFontParser::findFonts(" << XMLGetName (root) << ") called" << endl;
#endif
  //
  // First get the DeviceFontList node
  //
  const XmlNodePtr devicefontlist = XmlFile::findElement (root, "DeviceFontList");

  //
  // Set the head of the list to null
  //
  DeviceFont *head = NULL;
  DeviceFont *current = NULL;

  //
  // Now create a DeviceFont object from each of the children of this node
  //
  XmlNodePtr cur;

  for ( cur = XMLGetChildrenNode (devicefontlist); cur != NULL; cur = XMLNextNode (cur))
    {
      //
      // If the child is a "DeviceFont" element, findElement() will just return
      // that element; otherwise it returns NULL.
      //
      XmlNodePtr tmp = XmlFile::findElement (cur, "DeviceFont");
      if(tmp)
	{
	  //
	  // Convert this Element into a DeviceFont object.  We throw away the
	  // const'ness, since we need to modify its next pointer (which still
	  // preserves the logical constness of the object).
	  //
	  DeviceFont *fn = const_cast<DeviceFont *>(getDeviceFont (tmp));
	  //
	  // If the list head is null, then set the current node to be the list
	  // head; otherwise, the setNext() function adds the node to the tail.
	  //
	  if (head == NULL)
	     head = fn;
	  else
	    current->setNext (fn);
	  //
	  // Set the pointer for the next time around
	  //
	  current = fn;
	}
    }
  return head;
}


/**
 * Convert an XML DOM Element into a DeviceFont object containing a new
 * DeviceFont object.  The given element must be a "DeviceFont" element.
 *
 * @param element  the DeviceFont element
 */
DeviceFont *UPDFFontParser::
getDeviceFont (const XmlNodePtr element) const
{
#ifdef DEBUG
cout << "UPDFFontParser::getDeviceFont(" << XMLGetName (element) << ") called" << endl;
#endif
  //
  // Initialize the resulting DeviceFont object
  //
  DeviceFont* deviceFont = new DeviceFont();
  //
  // Get the string parameters and fill them in the DeviceFont
  //
  deviceFont->pszID = getAttribute(element, "ID");
  deviceFont->pszFont_Vendor = getAttribute(element, "Font_Vendor");
  deviceFont->pszEncoding = getAttribute(element, "Encoding");
  deviceFont->pszFontCommandSequenceID = getAttribute(element, "Font_Command_Sequence_ID");
  deviceFont->pszPassive_Font = getAttribute(element, "Passive_Font");

  //
  // Convert the font family string into an enumeration
  //
  char *fontfamily = getAttribute(element, "Font_Family");
  deviceFont->fontFamily = DeviceFont::getFontFamily (fontfamily);

  //
  // Move to Global Metrics, Panose and Name ID Node
  //
  XmlNodePtr grandChild;
  for (grandChild = XMLGetChildrenNode (element);
       grandChild != NULL;
       grandChild = XMLNextNode (grandChild)) {

    const char *nodeName = XMLGetName (grandChild);
    // Global metrics
    if(strcmp(nodeName, "GlobalMetrics") == 0) {
      getGlobalMetrics(deviceFont, grandChild);
    }
    // Panose
    else if(strcmp(nodeName, "Panose") == 0) {
      getPanose(deviceFont, grandChild);
    }
    // Name IDs
    else if(strcmp(nodeName, "NameIDs") == 0) {
      getNameIDs(deviceFont, grandChild);
    }
  }
  //
  // And return the result
  //
  return deviceFont;
}


/**
 * Find all the CommandSequence elements and return a linked list of
 * CommandSequence objects with all their information.  It will return null
 * if there is no command sequence information.
 */

CommandSequence *UPDFFontParser::findCmdSeqs (const XmlNodePtr root) const
{
  //
  // First get the CommandSequences node
  //
  XmlNodePtr commandsequences = XmlFile::findElement (root, "CommandSequences");
  if (commandsequences == NULL)
    return NULL;
  //
  // Set the head of the list to null
  //
  CommandSequence *head = NULL;
  CommandSequence *current = NULL;

  //
  // Now create a  object from each of the children of this node
  //
  for (XmlNodePtr child = XMLGetChildrenNode (commandsequences); child != NULL; child = XMLNextNode (child))
    {
      //
      // If the child is a "CommandSequence" element, findElement() will just
      // return that element; otherwise it returns NULL.
      //

      XmlNodePtr tmp = XmlFile::findElement (child, "CommandSequence");
      if(tmp) {
        //
        // Convert this Element into a CommandSequence object.  We throw away
        // the const'ness, since we need to modify its next pointer (which
        // still preserves the logical constness of the object).
        //
        CommandSequence *csn =
          const_cast<CommandSequence *>(getCommandSequence (tmp));
        //
        // If the list head is null, then set the current node to be the list
        // head; otherwise, the setNext() function adds the node to the tail.
        //
        if (head == NULL)
          head = csn;
        else
          current->setNext (csn);
        //
        // And set the pointer for the next time around
        //
        current = csn;
      }
    }
  return head;
}


/**
 * Find all the Event elements and return a linked list of
 * Event objects with all their information.  It will return null
 * if there is no command sequence information.
 */

Event *UPDFFontParser::findEvents (const XmlNodePtr root) const
{
  //
  // First get the Events node
  //
  XmlNodePtr events = XmlFile::findElement (root, "Events");
  if (events == NULL)
    return NULL;
  //
  // Set the head of the list to null
  //
  Event *head = NULL;
  Event *current = NULL;
  //
  // Now create a  object from each of the children of this node
  //
  for (XmlNodePtr child = XMLGetChildrenNode (events); child != NULL; child = XMLNextNode (child))
    {
      //
      // If the child is a "Event" element, findElement() will just
      // return that element; otherwise it returns NULL.
      //
      XmlNodePtr tmp = XmlFile::findElement (child, "Event");
      if(tmp) {
        //
        // Convert this Element into a Event object.  We throw away the
        // const'ness, since we need to modify its next pointer (which still
        // preserves the logical constness of the object).
        //
        Event *ev = const_cast<Event *>(getEvent (tmp));
        //
        // If the list head is null, then set the current node to be the list
        // head; otherwise, the setNext() function adds the node to the tail.
        //
        if (head == NULL)
          head = ev;
        else
          current->setNext (ev);
        //
        // And set the pointer for the next time around
        //
        current = ev;
      }
    }
  return head;
}


/**
 * Find all the Parameter elements and return a linked list of
 * Parameter objects with all their information.  It will return null
 * if there is no command sequence information.
 */

Parameter *UPDFFontParser::findParameters (const XmlNodePtr root) const
{
    //
    // First get the Parameters node
    //
    XmlNodePtr parm = XmlFile::findElement (root, "Parameters");
    if (parm == NULL)
        return NULL;
    //
    // Set the head of the list to null
    //
    Parameter *head = NULL;
    Parameter *current = NULL;
    //
    // Now create a  object from each of the children of this node
    //
    for (XmlNodePtr child = XMLGetChildrenNode (parm); child != NULL; child = XMLNextNode (child))
    {
        //
        // If the child is a "Parameter" element, findElement() will just
        // return that element; otherwise it returns NULL.
        //
        XmlNodePtr tmp = XmlFile::findElement (child, "Parameter");
        if(tmp) {
            //
            // Convert this Element into a Parameter object.  We throw away the
            // const'ness, since we need to modify its next pointer (which still
            // preserves the logical constness of the object).
            //
            Parameter *p = const_cast<Parameter *>(getParameter (tmp));
            //
            // If the list head is null, then set the current node to be the list
            // head; otherwise, the setNext() function adds the node to the tail.
            //
            if (head == NULL)
                head = p;
            else
                current->setNext (p);
            //
            // And set the pointer for the next time around
            //
            current = p;
        }
    }
    return head;
}


/**
 * Convert an XML DOM Element into a CommandSequence object containing a pair
 * of strings with the name and attribute value.  The given element must be
 * a "CommandSequence" element.
 *
 * @param element  pointer to a "CommandSequence" element
 */


CommandSequence *UPDFFontParser::getCommandSequence (const XmlNodePtr element) const
{
  //
  // Extract the strings and create the object
  //
  char* id = getAttribute (element, "CommandSequence_ID");
  char* value = getAttribute (element, "CommandSequence");
  return new CommandSequence (id, value);
}


/**
 * Convert an XML DOM Element into an Event object containing the strings.
 * The given element must be an "Event" element.
 *
 * @param element  pointer to an "Event" element
 */


Event *UPDFFontParser::getEvent (const XmlNodePtr element) const
{
  char * id = getAttribute (element, "ID");
  char * startend = getAttribute (element, "Event_StartEnd");
  char * type = getAttribute (element, "Event_ID");
  char * cmdseq = getAttribute (element, "CommandSequence_ID");
  return new Event (id, startend, type, cmdseq);
}


/**
* Convert an XML DOM Element into an Parameter object containing the strings.
* The given element must be an "Parameter" element.
*
* @param element  pointer to an "Parameter" element
*/

Parameter *UPDFFontParser::getParameter (const XmlNodePtr element) const
{
    char * id = getAttribute (element, "Parameter_ID");
    char * value = getAttribute (element, "Parameter");
    return new Parameter (id, value);
}


/**
 * Extract the global metrics information and store in the given DeviceFont
 * object.  The indicated DOM Element must be "GlobalMetrics" node.
 *
 * @param deviceFont  the object to modify
 * @param element     the "GlobalMetrics" DOM element for the font
 */

void UPDFFontParser::getGlobalMetrics(DeviceFont* deviceFont, const XmlNodePtr element) const
{
#ifdef DEBUG
cout << "UPDFFontParser::getGlobalMetrics(" << deviceFont << "," << element << ") called" << endl;
#endif
  deviceFont->iGlobal_Units_Per_Em = getInt (element, "Units_Per_Em");
  deviceFont->iGlobal_Points_Per_Inch = getInt (element, "Points_Per_Inch");
  deviceFont->iGlobal_Spacing = getInt (element, "Spacing");
  deviceFont->iGlobal_Minimum_Height = getInt (element, "Minimum_Height");
  deviceFont->iGlobal_Maximum_Height = getInt (element, "Maximum_Height");
  deviceFont->iGlobal_HeightStep = getInt (element, "HeightStep");
  deviceFont->iGlobal_Slant = getInt (element, "Slant");
  deviceFont->iGlobal_WindowsWeight = getInt (element, "WindowsWeight");
  deviceFont->iGlobal_Windows_Ascender = getInt (element, "Windows_Ascender");
  deviceFont->iGlobal_Windows_Descender = getInt (element, "Windows_Descender");
  deviceFont->iGlobal_Mac_Ascender = getInt (element, "Mac_Ascender");
  deviceFont->iGlobal_Mac_Descender = getInt (element, "Mac_Descender");
  deviceFont->iGlobal_Mac_Line_Gap = getInt (element, "Mac_Line_Gap");
  deviceFont->iGlobal_Typographic_Ascender = getInt (element, "Typographic_Ascender");
  deviceFont->iGlobal_Typographic_Descender = getInt (element, "Typographic_Descender");
  deviceFont->iGlobal_Typographic_Line_Gap = getInt (element, "Typographic_Line_Gap");
  deviceFont->iGlobal_Average_Character_Width = getInt (element, "Average_Character_Width");
  deviceFont->iGlobal_Maximum_Character_Increment = getInt (element, "Maximum_Character_Increment");
  deviceFont->iGlobal_Cap_Height = getInt (element, "Cap_Height");
  deviceFont->iGlobal_x_Height = getInt (element, "x_Height");
  deviceFont->iGlobal_Subscript_X_Size = getInt (element, "Subscript_X_Size");
  deviceFont->iGlobal_Subscript_Y_Size = getInt (element, "Subscript_Y_Size");
  deviceFont->iGlobal_Subscript_X_Offset = getInt (element, "Subscript_X_Offset");
  deviceFont->iGlobal_Subscript_Y_Offset = getInt (element, "Subscript_Y_Offset");
  deviceFont->iGlobal_Superscript_X_Size = getInt (element, "Superscript_X_Size");
  deviceFont->iGlobal_Superscript_Y_Size = getInt (element, "Superscript_Y_Size");
  deviceFont->iGlobal_Superscript_X_Offset = getInt (element, "Superscript_X_Offset");
  deviceFont->iGlobal_Superscript_Y_Offset = getInt (element, "Superscript_Y_Offset");
  deviceFont->iGlobal_Underscore_Size = getInt (element, "Underscore_Size");
  deviceFont->iGlobal_Underscore_Offset = getInt (element, "Underscore_Offset");
  deviceFont->iGlobal_Underscore_Absolute_Values = getInt (element, "Underscore_Absolute_Values");
  deviceFont->iGlobal_Strikeout_Size = getInt (element, "Strikeout_Size");
  deviceFont->iGlobal_Strikeout_Offset = getInt (element, "Strikeout_Offset");
  deviceFont->iGlobal_Strikeout_Absolute_Values = getInt (element, "Strikeout_Absolute_Values");
  deviceFont->iGlobal_Baseline = getInt (element, "Baseline");
  deviceFont->iGlobal_Aspect = getInt (element, "Aspect");
  deviceFont->iGlobal_Caret = getInt (element, "Caret");
  deviceFont->iGlobal_Orientation = getInt (element, "Orientation");
  deviceFont->iGlobal_Degree = getInt (element, "Degree");
  deviceFont->iGlobal_CharacterWidth_A_Value = getInt (element, "CharacterWidth_A_Value");
  deviceFont->iGlobal_CharacterWidth_B_Value = getInt (element, "CharacterWidth_B_Value");
  deviceFont->iGlobal_CharacterWidth_C_Value = getInt (element, "CharacterWidth_C_Value");
}


/**
 * Get the Panose information from the given XML DOM Element and store in
 * the given DeviceFont object.  The Element must be a "Panose" Element.
 *
 * @param deviceFont  the object to modify
 * @param element     the "Panose" DOM element for the font
 */
void UPDFFontParser::getPanose(DeviceFont* deviceFont, const XmlNodePtr element) const
{
#ifdef DEBUG
cout << "UPDFFontParser::getPanose(" << deviceFont << "," << element << ") called" << endl;
#endif

 char* str = (char *)"Panose_Family_Type";
 char* familyType = getAttribute(element, str);
  if (familyType)
  {
    deviceFont->panose.familyType = Panose::getFamilyType (familyType);
  }

  str = (char *)("Panose_Serif_Style");
  char* serifStyle = getAttribute (element, str);
  if (serifStyle)
  {
    deviceFont->panose.serifStyle = Panose::getSerifStyle (serifStyle);
  }

  str = (char *)("Panose_Weight");
  char* weight = getAttribute(element, str);
  if (weight)
  {
    deviceFont->panose.weight = Panose::getWeight(weight);
  }

  str = (char *)("Panose_Proportion");
  char* proportion = getAttribute(element, str);
  if (proportion)
  {
    deviceFont->panose.proportion = Panose::getProportion(proportion);
  }

  str = (char *)("Panose_Contrast");
  char* contrast = getAttribute(element, str);
  if (contrast)
  {
    deviceFont->panose.contrast = Panose::getContrast (contrast);
  }

  str = (char *)("Panose_Stroke_Variation");
  char* stroke = getAttribute(element, str);
  if (stroke)
  {
    deviceFont->panose.strokeVariation = Panose::getStrokeVariation(stroke);
  }

  str = (char *)("Panose_Arm_Style");
  char* armStyle = getAttribute(element, str);
  if (armStyle)
  {
    deviceFont->panose.armStyle = Panose::getArmStyle(armStyle);
  }

  str = (char *)("Panose_Letterform");
  char* letterform = getAttribute(element, str);
  if (letterform)
  {
    deviceFont->panose.letterform = Panose::getLetterform(letterform);
  }

  str = (char *)("Panose_Midline");
  char* midline = getAttribute(element, str);
  if (midline)
  {
    deviceFont->panose.midline = Panose::getMidline(midline);
  }

  str = (char *)("Panose_X_Height");
  char* xHeight = getAttribute(element, str);
  if (xHeight)
  {
    deviceFont->panose.xHeight = Panose::getXHeight(xHeight);
  }
}


/**
 * Extract the NameID information from the given XML DOM Element.  The Element
 * must be a "NameIDs" node.
 *
 * @param deviceFont  the object to modify
 * @param element     the "NameIDs" DOM element for the font
 */

void UPDFFontParser::getNameIDs(DeviceFont* deviceFont,
                                const XmlNodePtr element) const
{
#ifdef DEBUG
cout << "UPDFFontParser::getNameIDs(" << deviceFont << "," << element << ") called" << endl;
#endif
  XmlNodePtr idChild;
  const char* str = (const char *)"Name_ID";

  for (idChild = XMLGetChildrenNode (element); idChild != NULL; idChild = XMLNextNode (idChild)) {

    const char* nodeName = XMLGetName (idChild);

    if(strcmp(nodeName, "FaceName") == 0) {
      char* faceNameId = getAttribute(idChild, str);
      deviceFont->pszFaceName_Name_ID = faceNameId;
    }

    if(strcmp(nodeName, "FamilyName") == 0){
      char* familyNameId = getAttribute(idChild, str);
      deviceFont->pszFamilyName_Name_ID = familyNameId;
    }

    if(strcmp(nodeName, "FamilyName") == 0){
      char* uniqueNameId = getAttribute(idChild, str);
      deviceFont->pszUniqueName_Name_ID = uniqueNameId;
    }

    if(strcmp(nodeName, "SlantName") == 0){
      char* slantNameId = getAttribute(idChild, str);
      deviceFont->pszSlantName_Name_ID = slantNameId;
    }

    if(strcmp(nodeName, "WeightName") == 0){
      char* weightNameId = getAttribute(idChild, str);
      deviceFont->pszWeightName_Name_ID = weightNameId;
    }

  }
  //
  // And clean up
  //
  //delete str;
}


/**
 * A convenience method that reads an integer attribute from the given
 * XML DOM Element and stores that value in the given integer.  Stores
 * a 0 if the attribute doesn't exist or is blank.
 *
 * @param element  XML DOM Element that contains the attribute
 * @param attr     name of the attribute whose value contains an integer
 */
int UPDFFontParser::getInt (const XmlNodePtr element, const char *attr) const
{
  //
  // Initialize the return value
  //
  int ivalue = 0;
  char* cval = getAttribute (element, (const char *) attr);
  //cout << "cval: " << cval << endl;
  //
  // Check to see if the value is not blank
  //
  if(cval == NULL)
    return 0;

  if (strlen((char *)cval) > 0)
    {
      //
      // Convert to an integer and store in the indicate memory
      //
      sscanf((char *)cval, "%d", &ivalue);

    }
  //
  // Clean up
  //

  return ivalue;
}

/**
 * Get the attribute of the element as an array of UTF-8 characters.
 *
 *
 * @param e     DOM Element to get the attribute from
 * @param attr  name of the attribute
 */
char * UPDFFontParser::getAttribute (const XmlNodePtr e, const char * attr) const
{
  //
  // Get the attribute value as a UTF-8 string
  //
  const char *value = XMLGetProp (e, attr);

  return (char *)value;
}


/**
 * Return a linked list of DeviceFont objects which contain DeviceFont objects.
 * We have to make a copy of our internal list, and the caller is responsible
 * for deleting this new list.
 */

DeviceFont *UPDFFontParser::getFonts() const
{
  if (myFonts == NULL)
    return NULL;
  //
  // The list head that we will return
  //
  DeviceFont *head = new DeviceFont(*myFonts);
  DeviceFont *current = head;
  //
  // Copy all the elements in the linked list
  //
  const DeviceFont *source = myFonts->getNext();
  while (source != NULL)
    {
      DeviceFont *acopy = new DeviceFont (*source);
      current->setNext(acopy);
      current = acopy;
      source = source->getNext();
    }
  //
  // And return the new list we just created
  //
  return head;
}


/**
 * Return a linked list of CommandSequence objects which contain strings with
 * the command sequence name and value.  We have to make a copy of our
 * internal list, and the caller is responsible for deleting this new list.
 */

CommandSequence *UPDFFontParser::getCmdSeqs() const
{
  if (myCmdSeqs == NULL)
    return NULL;
  //
  // The list head that we will return
  //
  CommandSequence *head = new CommandSequence(*myCmdSeqs);
  CommandSequence *current = head;
  //
  // Copy all the elements in the linked list
  //
  const CommandSequence *source = myCmdSeqs->getNext();
  while (source != NULL)
    {
      CommandSequence *acopy = new CommandSequence (*source);
      current->setNext(acopy);
      current = acopy;
      source = source->getNext();
    }
  //
  // And return the new list we just created
  //
  return head;
}


/**
 * Return a linked list of Event objects.  We have to make a copy of our
 * internal list, and the caller is responsible for deleting this new list.
 */

Event *UPDFFontParser::getEvents() const
{
  if (myEvents == NULL)
    return NULL;
  //
  // The list head that we will return
  //
  Event *head = new Event(*myEvents);
  Event *current = head;
  //
  // Copy all the elements in the linked list
  //
  const Event *source = myEvents->getNext();
  while (source != NULL)
    {
      Event *acopy = new Event (*source);
      current->setNext(acopy);
      current = acopy;
      source = source->getNext();
    }
  //
  // And return the new list we just created
  //
  return head;
}


/**
* Return a linked list of Parameter objects.  We have to make a copy of our
* internal list, and the caller is responsible for deleting this new list.
*/

Parameter *UPDFFontParser::getParameters() const
{
    if (myParameters == NULL)
        return NULL;
    //
    // The list head that we will return
    //
    Parameter *head = new Parameter(*myParameters);
    Parameter *current = head;
    //
    // Copy all the elements in the linked list
    //
    const Parameter *source = myParameters->getNext();
    while (source != NULL)
    {
        Parameter *acopy = new Parameter (*source);
        current->setNext(acopy);
        current = acopy;
        source = source->getNext();
    }
    //
    // And return the new list we just created
    //
    return head;
}

