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
#ifndef _DeviceInfo
#define _DeviceInfo

#include <string>
#include <map>
#include <list>

#include "XMLInterface.hpp"

typedef std::list <std::string> OrientationList;
typedef std::list <std::string> ResolutionList;
typedef std::list <std::string> PrintModeList;
typedef std::list <std::string> TrayList;
typedef std::list <std::string> FormList;
typedef std::list <std::string> MediaList;
typedef std::list <std::string> OutputBinList;
typedef std::list <std::string> SidesList;
typedef std::list <std::string> SheetCollateList;
typedef std::list <std::string> TrimmingList;

typedef std::map <std::string, int> FileMap;

typedef struct _DeviceInfo
{
#ifdef INCLUDE_JP_UPDF_BOOKLET
   std::string     *pstringBookletClassName;
#endif
   std::string     *pstringCopyClassName;
   std::string     *pstringFormClassName;
#ifdef INCLUDE_JP_UPDF_JOGGING
   std::string     *pstringJoggingClassName;
#endif
   std::string     *pstringMediaClassName;
   std::string     *pstringNUpClassName;
   std::string     *pstringOrientationClassName;
   std::string     *pstringOutputBinClassName;
   std::string     *pstringPrintModeClassName;
   std::string     *pstringResolutionClassName;
   std::string     *pstringScalingClassName;
   std::string     *pstringSheetCollateClassName;
   std::string     *pstringSideClassName;
   std::string     *pstringStitchingClassName;
   std::string     *pstringTrayClassName;
   std::string     *pstringTrimmingClassName;
   std::string     *pstringGammaClassName;
   std::string     *pstringCommandClassName;
   std::string     *pstringDataClassName;
   std::string     *pstringStringClassName;

   std::string     *pstringInstanceClassName;
   std::string     *pstringBlitterClassName;
   std::string     *pstringExeName;
   std::string     *pstringData;

   XmlNodePtr       nodeGammaTables;
   std::string     *pstrGammaTablesXMLFile_d = NULL;

   OrientationList  listOrientations;
   ResolutionList   listResolutions;
   PrintModeList    listPrintModes;
   TrayList         listTrays;
   FormList         listForms;
   MediaList        listMedias;
   OutputBinList    listOutputBins;
   SidesList        listSides;
   SheetCollateList listSheetCollates;
   TrimmingList     listTrimmings;

   FileMap         *mapSeenFiles;

} DeviceInfo, *PDeviceInfo;

#endif
