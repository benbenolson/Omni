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
#include <set>

#include "Device.hpp"
#include "Omni.hpp"

void
printUsage (char *pszProgramName)
{
   std::cerr << "Usage: "
             << pszProgramName
             << " ( --buildOnly | --all | --UsePDC | --!UsePDC | --driver printer_library_name | '-sproperties=\"...\"' | printer_short_name )+"
             << std::endl;
}

#ifdef INCLUDE_JP_UPDF_BOOKLET

void
showBooklet (Device *pDevice,
             PSZRO   pszJP)
{
   DeviceBooklet *pBookletCurrent = 0;
   DeviceBooklet *pBooklet        = 0;
   std::string   *pstringRet      = 0;

   pBookletCurrent = pDevice->getCurrentBooklet ();

   if (!pBookletCurrent)
   {
      std::cerr << "Error: No current Booklet for " << SAFE_PRINT_PSZ (pszJP) << std::endl;

      return;
   }

   pBooklet = pBookletCurrent->create (pDevice, pszJP);

   if (!pBooklet)
   {
      std::cerr << "Error: Cannot create a new Booklet from \"" << pszJP << "\" !" << std::endl;

      return;
   }

   pstringRet = pBooklet->getJobProperties (false);
   if (pstringRet)
   {
      if (0 == pstringRet->compare (pszJP))
      {
         std::cout << pszJP         // @TBD
                   << std::endl;
      }
      else
      {
         std::cerr << "Error: Job Properties (\"" << *pstringRet << "\", \"" << pszJP << "\") should match!"  << std::endl;
      }

      delete pstringRet;
   }
   else
   {
      std::cerr << "Error: Empty Job Properties on new object!" << std::endl;
   }

   delete pBooklet;
}

#endif

void
showCopies (Device *pDevice,
            PSZRO   pszJP)
{
   DeviceCopies *pCopyCurrent = 0;
   DeviceCopies *pCopy        = 0;
   std::string  *pstringRet   = 0;

   pCopyCurrent = pDevice->getCurrentCopies ();

   if (!pCopyCurrent)
   {
      std::cerr << "Error: No current Copy for " << SAFE_PRINT_PSZ (pszJP) << std::endl;

      return;
   }

   pCopy = pCopyCurrent->create (pDevice, pszJP);

   if (!pCopy)
   {
      std::cerr << "Error: Cannot create a new Copy from \"" << pszJP << "\" !" << std::endl;

      return;
   }

   pstringRet = pCopy->getJobProperties (false);
   if (pstringRet)
   {
      if (0 == pstringRet->compare (pszJP))
      {
         std::string *pstringValue = 0;

         std::cout << pszJP
                   << " - #: "
                   << pCopy->getNumCopies ()
                   << " min: "
                   << pCopy->getMinimum ()
                   << " max: "
                   << pCopy->getMaximum ()
                   << " valid: "
                   << pCopy->isValid (pszJP)
                   << " equal: "
                   << pCopy->isEqual (pszJP)
                   << " supported: "
                   << pCopy->isSupported (pszJP)
                   << std::endl;

         JobProperties          jp (pszJP);
         JobPropertyEnumerator *pEnum      = jp.getEnumeration ();

         std::cout << pszJP
                   << " - ";
         while (  pEnum
               && pEnum->hasMoreElements ()
               )
         {
            PSZCRO pszKey   = pEnum->getCurrentKey ();
            PSZCRO pszValue = pEnum->getCurrentValue ();

            std::cout << pCopy->handlesKey (pszKey)
                      << " = handlesKey (\""
                      << pszKey
                      << "\")";

            pstringValue = pCopy->getJobPropertyType (pszKey);
            std::cout << ", type: " << SAFE_PRINT_STRING (pstringValue);
            delete pstringValue;
            pstringValue = pCopy->getJobProperty (pszKey);
            std::cout << ", property: " << SAFE_PRINT_STRING (pstringValue);
            delete pstringValue;
            pstringValue = pCopy->translateKeyValue (pszKey, pszValue);
            std::cout << ", xlate: " << SAFE_PRINT_STRING (pstringValue) << " ";
            delete pstringValue;

            pEnum->nextElement ();
         }
         delete pEnum;
         std::cout << std::endl;

         pstringValue = pCopy->getAllTranslation ();
         std::cout << pszJP
                   << " - allXlate: " << SAFE_PRINT_STRING (pstringValue)
                   << std::endl;
         delete pstringValue;
         pstringValue = pCopy->getJobProperties (true);
         std::cout << pszJP
                   << " - deviceJP: " << SAFE_PRINT_STRING (pstringValue)
                   << std::endl;
         delete pstringValue;

         BinaryData *pbdData = pCopy->getData ();
         std::cout << pszJP
                   << " - data: ";
         if (pbdData)
         {
            std::cout << *pbdData;
         }
         else
         {
            std::cout << "NULL";
         }
         std::cout << " simul: "
                   << pCopy->needsSimulation ()
                   << " deviceID: "
                   << SAFE_PRINT_PSZ (pCopy->getDeviceID ())
                   << std::endl;
      }
      else
      {
         std::cerr << "Error: Job Properties (\"" << *pstringRet << "\", \"" << pszJP << "\") should match!"  << std::endl;
      }

      delete pstringRet;
   }
   else
   {
      std::cerr << "Error: Empty Job Properties on new object!" << std::endl;
   }

   delete pCopy;
}

void
showForm (Device *pDevice,
          PSZRO   pszJP)
{
   DeviceForm  *pFormCurrent = 0;
   DeviceForm  *pForm        = 0;
   std::string *pstringRet   = 0;

   pFormCurrent = pDevice->getCurrentForm ();

   if (!pFormCurrent)
   {
      std::cerr << "Error: No current Form for " << SAFE_PRINT_PSZ (pszJP) << std::endl;

      return;
   }

   pForm = pFormCurrent->create (pDevice, pszJP);

   if (!pForm)
   {
      std::cerr << "Error: Cannot create a new Form from \"" << pszJP << "\" !" << std::endl;

      return;
   }

   pstringRet = pForm->getJobProperties (false);
   if (pstringRet)
   {
      if (0 == pstringRet->compare (pszJP))
      {
         HardCopyCap *pHCC         = pForm->getHardCopyCap ();
         std::string *pstringValue = 0;

         pstringValue = pForm->getForm ();

         std::cout << pszJP << " - Form dimensions are: ("
                   << pHCC->getCx ()
                   << ","
                   << pHCC->getCy ()
                   << ") (L:"
                   << pHCC->getLeftClip ()
                   << ",T:"
                   << pHCC->getTopClip ()
                   << ",R:"
                   << pHCC->getRightClip ()
                   << ",B:"
                   << pHCC->getBottomClip ()
                   << ") - Form:"
                   << SAFE_PRINT_STRING (pstringValue)
                   << " valid: "
                   << pForm->isValid (pszJP)
                   << " equal: "
                   << pForm->isEqual (pszJP)
                   << " supported: "
                   << pForm->isSupported (pszJP)
                   << std::endl;
         delete pstringValue;

         JobProperties          jp (pszJP);
         JobPropertyEnumerator *pEnum      = jp.getEnumeration ();

         std::cout << pszJP
                   << " - ";
         while (  pEnum
               && pEnum->hasMoreElements ()
               )
         {
            PSZCRO pszKey   = pEnum->getCurrentKey ();
            PSZCRO pszValue = pEnum->getCurrentValue ();

            std::cout << pForm->handlesKey (pszKey)
                      << " = handlesKey (\""
                      << pszKey
                      << "\")";

            pstringValue = pForm->getJobPropertyType (pszKey);
            std::cout << ", type: " << SAFE_PRINT_STRING (pstringValue);
            delete pstringValue;
            pstringValue = pForm->getJobProperty (pszKey);
            std::cout << ", property: " << SAFE_PRINT_STRING (pstringValue);
            delete pstringValue;
            pstringValue = pForm->translateKeyValue (pszKey, pszValue);
            std::cout << ", xlate: " << SAFE_PRINT_STRING (pstringValue) << " ";
            delete pstringValue;

            pEnum->nextElement ();
         }
         delete pEnum;
         std::cout << std::endl;

         pstringValue = pForm->getAllTranslation ();
         std::cout << pszJP
                   << " - allXlate: " << SAFE_PRINT_STRING (pstringValue)
                   << std::endl;
         delete pstringValue;
         pstringValue = pForm->getJobProperties (true);
         std::cout << pszJP
                   << " - deviceJP: " << SAFE_PRINT_STRING (pstringValue)
                   << std::endl;
         delete pstringValue;

         BinaryData *pbdData = pForm->getData ();
         std::cout << pszJP
                   << " - data: ";
         if (pbdData)
         {
            std::cout << *pbdData;
         }
         else
         {
            std::cout << "NULL";
         }
         std::cout << " deviceID: "
                   << SAFE_PRINT_PSZ (pForm->getDeviceID ())
                   << std::endl;
      }
      else
      {
         std::cerr << "Error: Job Properties (\"" << *pstringRet << "\", \"" << pszJP << "\") should match!"  << std::endl;
      }

      delete pstringRet;
   }
   else
   {
      std::cerr << "Error: Empty Job Properties on new object!" << std::endl;
   }

   delete pForm;
}

#ifdef INCLUDE_JP_UPDF_JOGGING

void
showJogging (DeviceJogging *pJogging)
{
   DeviceJogging *pJoggingCurrent = 0;
   DeviceJogging *pJogging        = 0;
   std::string   *pstringRet      = 0;

   pJoggingCurrent = pDevice->getCurrentJogging ();

   if (!pJoggingCurrent)
   {
      std::cerr << "Error: No current Jogging for " << SAFE_PRINT_PSZ (pszJP) << std::endl;

      return;
   }

   pJogging = pJoggingCurrent->create (pDevice, pszJP);

   if (!pJogging)
   {
      std::cerr << "Error: Cannot create a new Jogging from \"" << pszJP << "\" !" << std::endl;

      return;
   }

   pstringRet = pJogging->getJobProperties (false);
   if (pstringRet)
   {
      if (0 == pstringRet->compare (pszJP))
      {
         std::cout << pszJP         // @TBD
                   << " valid: "
                   << pJogging->isValid (pszJP)
                   << " equal: "
                   << pJogging->isEqual (pszJP)
                   << " supported: "
                   << pJogging->isSupported (pszJP)
                   << std::endl;
      }
      else
      {
         std::cerr << "Error: Job Properties (\"" << *pstringRet << "\", \"" << pszJP << "\") should match!"  << std::endl;
      }

      delete pstringRet;
   }
   else
   {
      std::cerr << "Error: Empty Job Properties on new object!" << std::endl;
   }

   delete pJogging;
}

#endif

void
showMedia (Device *pDevice,
           PSZRO   pszJP)
{
   DeviceMedia  *pMediaCurrent = 0;
   DeviceMedia  *pMedia        = 0;
   std::string  *pstringRet    = 0;

   pMediaCurrent = pDevice->getCurrentMedia ();

   if (!pMediaCurrent)
   {
      std::cerr << "Error: No current Media for " << SAFE_PRINT_PSZ (pszJP) << std::endl;

      return;
   }

   pMedia = pMediaCurrent->create (pDevice, pszJP);

   if (!pMedia)
   {
      std::cerr << "Error: Cannot create a new Media from \"" << pszJP << "\" !" << std::endl;

      return;
   }

   pstringRet = pMedia->getJobProperties (false);
   if (pstringRet)
   {
      if (0 == pstringRet->compare (pszJP))
      {
         std::string *pstringValue = 0;

         pstringValue = pMedia->getMedia ();

         std::cout << pszJP
                   << " - Media: "
                   << SAFE_PRINT_STRING (pstringValue)
                   << ", Color Adjust Required = "
                   << pMedia->getColorAdjustRequired ()
                   << ", Absorption = "
                   << pMedia->getAbsorption ()
                   << " valid: "
                   << pMedia->isValid (pszJP)
                   << " equal: "
                   << pMedia->isEqual (pszJP)
                   << " supported: "
                   << pMedia->isSupported (pszJP)
                   << std::endl;
         delete pstringValue;

         JobProperties          jp (pszJP);
         JobPropertyEnumerator *pEnum      = jp.getEnumeration ();

         std::cout << pszJP
                   << " - ";
         while (  pEnum
               && pEnum->hasMoreElements ()
               )
         {
            PSZCRO pszKey   = pEnum->getCurrentKey ();
            PSZCRO pszValue = pEnum->getCurrentValue ();

            std::cout << pMedia->handlesKey (pszKey)
                      << " = handlesKey (\""
                      << pszKey
                      << "\")";

            pstringValue = pMedia->getJobPropertyType (pszKey);
            std::cout << ", type: " << SAFE_PRINT_STRING (pstringValue);
            delete pstringValue;
            pstringValue = pMedia->getJobProperty (pszKey);
            std::cout << ", property: " << SAFE_PRINT_STRING (pstringValue);
            delete pstringValue;
            pstringValue = pMedia->translateKeyValue (pszKey, pszValue);
            std::cout << ", xlate: " << SAFE_PRINT_STRING (pstringValue) << " ";
            delete pstringValue;

            pEnum->nextElement ();
         }
         delete pEnum;
         std::cout << std::endl;

         pstringValue = pMedia->getAllTranslation ();
         std::cout << pszJP
                   << " - allXlate: " << SAFE_PRINT_STRING (pstringValue)
                   << std::endl;
         delete pstringValue;
         pstringValue = pMedia->getJobProperties (true);
         std::cout << pszJP
                   << " - deviceJP: " << SAFE_PRINT_STRING (pstringValue)
                   << std::endl;
         delete pstringValue;

         BinaryData *pbdData = pMedia->getData ();
         std::cout << pszJP
                   << " - data: ";
         if (pbdData)
         {
            std::cout << *pbdData;
         }
         else
         {
            std::cout << "NULL";
         }
         std::cout << " deviceID: "
                   << SAFE_PRINT_PSZ (pMedia->getDeviceID ())
                   << std::endl;
      }
      else
      {
         std::cerr << "Error: Job Properties (\"" << *pstringRet << "\", \"" << pszJP << "\") should match!"  << std::endl;
      }

      delete pstringRet;
   }
   else
   {
      std::cerr << "Error: Empty Job Properties on new object!" << std::endl;
   }

   delete pMedia;
}

void
showNUp (Device *pDevice,
         PSZRO   pszJP)
{
   DeviceNUp   *pNUpCurrent = 0;
   DeviceNUp   *pNUp        = 0;
   std::string *pstringRet  = 0;

   pNUpCurrent = pDevice->getCurrentNUp ();

   if (!pNUpCurrent)
   {
      std::cerr << "Error: No current NUp for " << SAFE_PRINT_PSZ (pszJP) << std::endl;

      return;
   }

   pNUp = pNUpCurrent->create (pDevice, pszJP);

   if (!pNUp)
   {
      std::cerr << "Error: Cannot create a new NUp from \"" << pszJP << "\" !" << std::endl;

      return;
   }

   pstringRet = pNUp->getJobProperties (false);
   if (pstringRet)
   {
      if (0 == pstringRet->compare (pszJP))
      {
         std::string *pstringValue = 0;

         pstringValue = pNUp->getDirection ();

         std::cout << pszJP
                   << " - X Pages: "
                   << pNUp->getXPages ()
                   << " Y Pages: "
                   << pNUp->getYPages ()
                   << " DirectionName: "
                   << SAFE_PRINT_STRING (pstringValue)
                   << " valid: "
                   << pNUp->isValid (pszJP)
                   << " equal: "
                   << pNUp->isEqual (pszJP)
                   << " supported: "
                   << pNUp->isSupported (pszJP)
                   << std::endl;
         delete pstringValue;

         JobProperties          jp (pszJP);
         JobPropertyEnumerator *pEnum      = jp.getEnumeration ();

         std::cout << pszJP
                   << " - ";
         while (  pEnum
               && pEnum->hasMoreElements ()
               )
         {
            PSZCRO pszKey   = pEnum->getCurrentKey ();
            PSZCRO pszValue = pEnum->getCurrentValue ();

            std::cout << pNUp->handlesKey (pszKey)
                      << " = handlesKey (\""
                      << pszKey
                      << "\")";

            pstringValue = pNUp->getJobPropertyType (pszKey);
            std::cout << ", type: " << SAFE_PRINT_STRING (pstringValue);
            delete pstringValue;
            pstringValue = pNUp->getJobProperty (pszKey);
            std::cout << ", property: " << SAFE_PRINT_STRING (pstringValue);
            delete pstringValue;
            pstringValue = pNUp->translateKeyValue (pszKey, pszValue);
            std::cout << ", xlate: " << SAFE_PRINT_STRING (pstringValue) << " ";
            delete pstringValue;

            pEnum->nextElement ();
         }
         delete pEnum;
         std::cout << std::endl;

         pstringValue = pNUp->getAllTranslation ();
         std::cout << pszJP
                   << " - allXlate: " << SAFE_PRINT_STRING (pstringValue)
                   << std::endl;
         delete pstringValue;
         pstringValue = pNUp->getJobProperties (true);
         std::cout << pszJP
                   << " - deviceJP: " << SAFE_PRINT_STRING (pstringValue)
                   << std::endl;
         delete pstringValue;

         BinaryData *pbdData = pNUp->getData ();
         std::cout << pszJP
                   << " - data: ";
         if (pbdData)
         {
            std::cout << *pbdData;
         }
         else
         {
            std::cout << "NULL";
         }
         std::cout << " deviceID: "
                   << SAFE_PRINT_PSZ (pNUp->getDeviceID ())
                   << std::endl;
      }
      else
      {
         std::cerr << "Error: Job Properties (\"" << *pstringRet << "\", \"" << pszJP << "\") should match!"  << std::endl;
      }

      delete pstringRet;
   }
   else
   {
      std::cerr << "Error: Empty Job Properties on new object!" << std::endl;
   }

   delete pNUp;
}

void
showOrientation (Device *pDevice,
                 PSZRO   pszJP)
{
   DeviceOrientation *pOrientationCurrent = 0;
   DeviceOrientation *pOrientation        = 0;
   std::string       *pstringRet          = 0;

   pOrientationCurrent = pDevice->getCurrentOrientation ();

   if (!pOrientationCurrent)
   {
      std::cerr << "Error: No current Orientation for " << SAFE_PRINT_PSZ (pszJP) << std::endl;

      return;
   }

   pOrientation = pOrientationCurrent->create (pDevice, pszJP);

   if (!pOrientation)
   {
      std::cerr << "Error: Cannot create a new Orientation from \"" << pszJP << "\" !" << std::endl;

      return;
   }

   pstringRet = pOrientation->getJobProperties (false);
   if (pstringRet)
   {
      if (0 == pstringRet->compare (pszJP))
      {
         std::string *pstringValue = 0;

         pstringValue = pOrientation->getRotation ();

         std::cout << pszJP
                   << " - Rotation: "
                   << SAFE_PRINT_STRING (pstringValue)
                   << " valid: "
                   << pOrientation->isValid (pszJP)
                   << " equal: "
                   << pOrientation->isEqual (pszJP)
                   << " supported: "
                   << pOrientation->isSupported (pszJP)
                   << std::endl;
         delete pstringValue;

         JobProperties          jp (pszJP);
         JobPropertyEnumerator *pEnum      = jp.getEnumeration ();

         std::cout << pszJP
                   << " - ";
         while (  pEnum
               && pEnum->hasMoreElements ()
               )
         {
            PSZCRO pszKey   = pEnum->getCurrentKey ();
            PSZCRO pszValue = pEnum->getCurrentValue ();

            std::cout << pOrientation->handlesKey (pszKey)
                      << " = handlesKey (\""
                      << pszKey
                      << "\")";

            pstringValue = pOrientation->getJobPropertyType (pszKey);
            std::cout << ", type: " << SAFE_PRINT_STRING (pstringValue);
            delete pstringValue;
            pstringValue = pOrientation->getJobProperty (pszKey);
            std::cout << ", property: " << SAFE_PRINT_STRING (pstringValue);
            delete pstringValue;
            pstringValue = pOrientation->translateKeyValue (pszKey, pszValue);
            std::cout << ", xlate: " << SAFE_PRINT_STRING (pstringValue) << " ";
            delete pstringValue;

            pEnum->nextElement ();
         }
         delete pEnum;
         std::cout << std::endl;

         pstringValue = pOrientation->getAllTranslation ();
         std::cout << pszJP
                   << " - allXlate: " << SAFE_PRINT_STRING (pstringValue)
                   << std::endl;
         delete pstringValue;
         pstringValue = pOrientation->getJobProperties (true);
         std::cout << pszJP
                   << " - deviceJP: " << SAFE_PRINT_STRING (pstringValue)
                   << std::endl;
         delete pstringValue;

         std::cout << pszJP
                   << " - deviceID: "
                   << SAFE_PRINT_PSZ (pOrientation->getDeviceID ())
                   << std::endl;
      }
      else
      {
         std::cerr << "Error: Job Properties (\"" << *pstringRet << "\", \"" << pszJP << "\") should match!"  << std::endl;
      }

      delete pstringRet;
   }
   else
   {
      std::cerr << "Error: Empty Job Properties on new object!" << std::endl;
   }

   delete pOrientation;
}

#ifdef INCLUDE_JP_COMMON_OUTPUT_BIN

void
showOutputBin (Device *pDevice,
               PSZRO   pszJP)
{
   DeviceOutputBin *pOutputBinCurrent = 0;
   DeviceOutputBin *pOutputBin        = 0;
   std::string     *pstringRet        = 0;

   pOutputBinCurrent = pDevice->getCurrentOutputBin ();

   if (!pOutputBinCurrent)
   {
      std::cerr << "Error: No current OutputBin for " << SAFE_PRINT_PSZ (pszJP) << std::endl;

      return;
   }

   pOutputBin = pOutputBinCurrent->create (pDevice, pszJP);

   if (!pOutputBin)
   {
      std::cerr << "Error: Cannot create a new OutputBin from \"" << pszJP << "\" !" << std::endl;

      return;
   }

   pstringRet = pOutputBin->getJobProperties (false);
   if (pstringRet)
   {
      if (0 == pstringRet->compare (pszJP))
      {
         std::string *pstringValue = 0;

         pstringValue = pOutputBin->getOutputBin ();

         std::cout << pszJP
                   << " - OutputBin: "
                   << SAFE_PRINT_STRING (pstringValue)
                   << " valid: "
                   << pOutputBin->isValid (pszJP)
                   << " equal: "
                   << pOutputBin->isEqual (pszJP)
                   << " supported: "
                   << pOutputBin->isSupported (pszJP)
                   << std::endl;
         delete pstringValue;

         JobProperties          jp (pszJP);
         JobPropertyEnumerator *pEnum      = jp.getEnumeration ();

         std::cout << pszJP
                   << " - ";
         while (  pEnum
               && pEnum->hasMoreElements ()
               )
         {
            PSZCRO pszKey   = pEnum->getCurrentKey ();
            PSZCRO pszValue = pEnum->getCurrentValue ();

            std::cout << pOutputBin->handlesKey (pszKey)
                      << " = handlesKey (\""
                      << pszKey
                      << "\")";

            pstringValue = pOutputBin->getJobPropertyType (pszKey);
            std::cout << ", type: " << SAFE_PRINT_STRING (pstringValue);
            delete pstringValue;
            pstringValue = pOutputBin->getJobProperty (pszKey);
            std::cout << ", property: " << SAFE_PRINT_STRING (pstringValue);
            delete pstringValue;
            pstringValue = pOutputBin->translateKeyValue (pszKey, pszValue);
            std::cout << ", xlate: " << SAFE_PRINT_STRING (pstringValue) << " ";
            delete pstringValue;

            pEnum->nextElement ();
         }
         delete pEnum;
         std::cout << std::endl;

         pstringValue = pOutputBin->getAllTranslation ();
         std::cout << pszJP
                   << " - allXlate: " << SAFE_PRINT_STRING (pstringValue)
                   << std::endl;
         delete pstringValue;
         pstringValue = pOutputBin->getJobProperties (true);
         std::cout << pszJP
                   << " - deviceJP: " << SAFE_PRINT_STRING (pstringValue)
                   << std::endl;
         delete pstringValue;

         BinaryData *pbdData = pOutputBin->getData ();
         std::cout << pszJP
                   << " - data: ";
         if (pbdData)
         {
            std::cout << *pbdData;
         }
         else
         {
            std::cout << "NULL";
         }
         std::cout << " deviceID: "
                   << SAFE_PRINT_PSZ (pOutputBin->getDeviceID ())
                   << std::endl;
      }
      else
      {
         std::cerr << "Error: Job Properties (\"" << *pstringRet << "\", \"" << pszJP << "\") should match!"  << std::endl;
      }

      delete pstringRet;
   }
   else
   {
      std::cerr << "Error: Empty Job Properties on new object!" << std::endl;
   }

   delete pOutputBin;
}

#endif

void
showPrintMode (Device *pDevice,
               PSZRO   pszJP)
{
   DevicePrintMode *pPrintModeCurrent = 0;
   DevicePrintMode *pPrintMode        = 0;
   std::string     *pstringRet        = 0;

   pPrintModeCurrent = pDevice->getCurrentPrintMode ();

   if (!pPrintModeCurrent)
   {
      std::cerr << "Error: No current PrintMode for " << SAFE_PRINT_PSZ (pszJP) << std::endl;

      return;
   }

   pPrintMode = pPrintModeCurrent->create (pDevice, pszJP);

   if (!pPrintMode)
   {
      std::cerr << "Error: Cannot create a new PrintMode from \"" << pszJP << "\" !" << std::endl;

      return;
   }

   pstringRet = pPrintMode->getJobProperties (false);
   if (pstringRet)
   {
      if (0 == pstringRet->compare (pszJP))
      {
         std::string *pstringValue = 0;

         std::cout << pszJP << " - CT: "
                   << pPrintMode->getColorTech ()
                   << ", P: "
                   << pPrintMode->getPhysicalCount ()
                   << ", L: "
                   << pPrintMode->getLogicalCount ()
                   << ", #planes:"
                   << pPrintMode->getNumPlanes ()
                   << " valid: "
                   << pPrintMode->isValid (pszJP)
                   << " equal: "
                   << pPrintMode->isEqual (pszJP)
                   << " supported: "
                   << pPrintMode->isSupported (pszJP)
                   << std::endl;

         JobProperties          jp (pszJP);
         JobPropertyEnumerator *pEnum      = jp.getEnumeration ();

         std::cout << pszJP
                   << " - ";
         while (  pEnum
               && pEnum->hasMoreElements ()
               )
         {
            PSZCRO pszKey   = pEnum->getCurrentKey ();
            PSZCRO pszValue = pEnum->getCurrentValue ();

            std::cout << pPrintMode->handlesKey (pszKey)
                      << " = handlesKey (\""
                      << pszKey
                      << "\")";

            pstringValue = pPrintMode->getJobPropertyType (pszKey);
            std::cout << ", type: " << SAFE_PRINT_STRING (pstringValue);
            delete pstringValue;
            pstringValue = pPrintMode->getJobProperty (pszKey);
            std::cout << ", property: " << SAFE_PRINT_STRING (pstringValue);
            delete pstringValue;
            pstringValue = pPrintMode->translateKeyValue (pszKey, pszValue);
            std::cout << ", xlate: " << SAFE_PRINT_STRING (pstringValue) << " ";
            delete pstringValue;

            pEnum->nextElement ();
         }
         delete pEnum;
         std::cout << std::endl;

         pstringValue = pPrintMode->getAllTranslation ();
         std::cout << pszJP
                   << " - allXlate: " << SAFE_PRINT_STRING (pstringValue)
                   << std::endl;
         delete pstringValue;
         pstringValue = pPrintMode->getJobProperties (true);
         std::cout << pszJP
                   << " - deviceJP: " << SAFE_PRINT_STRING (pstringValue)
                   << std::endl;
         delete pstringValue;

         std::cout << pszJP
                   << " - deviceID: "
                   << SAFE_PRINT_PSZ (pPrintMode->getDeviceID ())
                   << std::endl;
      }
      else
      {
         std::cerr << "Error: Job Properties (\"" << *pstringRet << "\", \"" << pszJP << "\") should match!"  << std::endl;
      }

      delete pstringRet;
   }
   else
   {
      std::cerr << "Error: Empty Job Properties on new object!" << std::endl;
   }

   delete pPrintMode;
}

void
showResolution (Device *pDevice,
                PSZRO   pszJP)
{
   DeviceResolution *pResolutionCurrent = 0;
   DeviceResolution *pResolution        = 0;
   std::string      *pstringRet         = 0;

   pResolutionCurrent = pDevice->getCurrentResolution ();

   if (!pResolutionCurrent)
   {
      std::cerr << "Error: No current Resolution for " << SAFE_PRINT_PSZ (pszJP) << std::endl;

      return;
   }

   pResolution = pResolutionCurrent->create (pDevice, pszJP);

   if (!pResolution)
   {
      std::cerr << "Error: Cannot create a new Resolution from \"" << pszJP << "\" !" << std::endl;

      return;
   }

   pstringRet = pResolution->getJobProperties (false);
   if (pstringRet)
   {
      if (0 == pstringRet->compare (pszJP))
      {
         std::string *pstringValue = 0;

         std::cout << pszJP << " - XRes: "
                   << pResolution->getXRes ()
                   << ", YRes: "
                   << pResolution->getYRes ()
                   << " valid: "
                   << pResolution->isValid (pszJP)
                   << " equal: "
                   << pResolution->isEqual (pszJP)
                   << " supported: "
                   << pResolution->isSupported (pszJP)
                   << std::endl;

         JobProperties          jp (pszJP);
         JobPropertyEnumerator *pEnum      = jp.getEnumeration ();

         std::cout << pszJP
                   << " - ";
         while (  pEnum
               && pEnum->hasMoreElements ()
               )
         {
            PSZCRO pszKey   = pEnum->getCurrentKey ();
            PSZCRO pszValue = pEnum->getCurrentValue ();

            std::cout << pResolution->handlesKey (pszKey)
                      << " = handlesKey (\""
                      << pszKey
                      << "\")";

            pstringValue = pResolution->getJobPropertyType (pszKey);
            std::cout << ", type: " << SAFE_PRINT_STRING (pstringValue);
            delete pstringValue;
            pstringValue = pResolution->getJobProperty (pszKey);
            std::cout << ", property: " << SAFE_PRINT_STRING (pstringValue);
            delete pstringValue;
            pstringValue = pResolution->translateKeyValue (pszKey, pszValue);
            std::cout << ", xlate: " << SAFE_PRINT_STRING (pstringValue) << " ";
            delete pstringValue;

            pEnum->nextElement ();
         }
         delete pEnum;
         std::cout << std::endl;

         pstringValue = pResolution->getAllTranslation ();
         std::cout << pszJP
                   << " - allXlate: " << SAFE_PRINT_STRING (pstringValue)
                   << std::endl;
         delete pstringValue;
         pstringValue = pResolution->getJobProperties (true);
         std::cout << pszJP
                   << " - deviceJP: " << SAFE_PRINT_STRING (pstringValue)
                   << std::endl;
         delete pstringValue;

         BinaryData *pbdData = pResolution->getData ();
         std::cout << pszJP
                   << " - data: ";
         if (pbdData)
         {
            std::cout << *pbdData;
         }
         else
         {
            std::cout << "NULL";
         }
         std::cout << " deviceID: "
                   << SAFE_PRINT_PSZ (pResolution->getDeviceID ())
                   << std::endl;
      }
      else
      {
         std::cerr << "Error: Job Properties (\"" << *pstringRet << "\", \"" << pszJP << "\") should match!"  << std::endl;
      }

      delete pstringRet;
   }
   else
   {
      std::cerr << "Error: Empty Job Properties on new object!" << std::endl;
   }

   delete pResolution;
}

#ifdef INCLUDE_JP_COMMON_SCALING

void
showScaling (Device *pDevice,
             PSZRO   pszJP)
{
   DeviceScaling *pScalingCurrent = 0;
   DeviceScaling *pScaling        = 0;
   std::string   *pstringRet      = 0;

   pScalingCurrent = pDevice->getCurrentScaling ();

   if (!pScalingCurrent)
   {
      std::cerr << "Error: No current Scaling for " << SAFE_PRINT_PSZ (pszJP) << std::endl;

      return;
   }

   pScaling = pScalingCurrent->create (pDevice, pszJP);

   if (!pScaling)
   {
      std::cerr << "Error: Cannot create a new Scaling from \"" << pszJP << "\" !" << std::endl;

      return;
   }

   pstringRet = pScaling->getJobProperties (false);
   if (pstringRet)
   {
      if (0 == pstringRet->compare (pszJP))
      {
         std::string *pstringValue = 0;

         pstringValue = pScaling->getType ();

         std::cout << pszJP
                   << " - ScalingType: "
                   << SAFE_PRINT_STRING (pstringValue)
                   << ", MinPercentage: "
                   << pScaling->getMinimumPercentage ()
                   << ", MaxPercentage: "
                   << pScaling->getMaximumPercentage ()
                   << ", ScalingPercentage: "
                   << pScaling->getScalingPercentage ()
                   << " valid: "
                   << pScaling->isValid (pszJP)
                   << " equal: "
                   << pScaling->isEqual (pszJP)
                   << " supported: "
                   << pScaling->isSupported (pszJP)
                   << std::endl;
         delete pstringValue;

         JobProperties          jp (pszJP);
         JobPropertyEnumerator *pEnum      = jp.getEnumeration ();

         std::cout << pszJP
                   << " - ";
         while (  pEnum
               && pEnum->hasMoreElements ()
               )
         {
            PSZCRO pszKey   = pEnum->getCurrentKey ();
            PSZCRO pszValue = pEnum->getCurrentValue ();

            std::cout << pScaling->handlesKey (pszKey)
                      << " = handlesKey (\""
                      << pszKey
                      << "\")";

            pstringValue = pScaling->getJobPropertyType (pszKey);
            std::cout << ", type: " << SAFE_PRINT_STRING (pstringValue);
            delete pstringValue;
            pstringValue = pScaling->getJobProperty (pszKey);
            std::cout << ", property: " << SAFE_PRINT_STRING (pstringValue);
            delete pstringValue;
            pstringValue = pScaling->translateKeyValue (pszKey, pszValue);
            std::cout << ", xlate: " << SAFE_PRINT_STRING (pstringValue) << " ";
            delete pstringValue;

            pEnum->nextElement ();
         }
         delete pEnum;
         std::cout << std::endl;

         pstringValue = pScaling->getAllTranslation ();
         std::cout << pszJP
                   << " - allXlate: " << SAFE_PRINT_STRING (pstringValue)
                   << std::endl;
         delete pstringValue;
         pstringValue = pScaling->getJobProperties (true);
         std::cout << pszJP
                   << " - deviceJP: " << SAFE_PRINT_STRING (pstringValue)
                   << std::endl;
         delete pstringValue;

         BinaryData *pbdData = pScaling->getData ();
         std::cout << pszJP
                   << " - data: ";
         if (pbdData)
         {
            std::cout << *pbdData;
         }
         else
         {
            std::cout << "NULL";
         }
         std::cout << " deviceID: "
                   << SAFE_PRINT_PSZ (pScaling->getDeviceID ())
                   << std::endl;
      }
      else
      {
         std::cerr << "Error: Job Properties (\"" << *pstringRet << "\", \"" << pszJP << "\") should match!"  << std::endl;
      }

      delete pstringRet;
   }
   else
   {
      std::cerr << "Error: Empty Job Properties on new object!" << std::endl;
   }

   delete pScaling;
}

#endif

#ifdef INCLUDE_JP_COMMON_SHEET_COLLATE

void
showSheetCollate (Device *pDevice,
                  PSZRO   pszJP)
{
   DeviceSheetCollate *pSheetCollateCurrent = 0;
   DeviceSheetCollate *pSheetCollate        = 0;
   std::string        *pstringRet           = 0;

   pSheetCollateCurrent = pDevice->getCurrentSheetCollate ();

   if (!pSheetCollateCurrent)
   {
      std::cerr << "Error: No current SheetCollate for " << SAFE_PRINT_PSZ (pszJP) << std::endl;

      return;
   }

   pSheetCollate = pSheetCollateCurrent->create (pDevice, pszJP);

   if (!pSheetCollate)
   {
      std::cerr << "Error: Cannot create a new SheetCollate from \"" << pszJP << "\" !" << std::endl;

      return;
   }

   pstringRet = pSheetCollate->getJobProperties (false);
   if (pstringRet)
   {
      if (0 == pstringRet->compare (pszJP))
      {
         std::string *pstringValue = 0;

         pstringValue = pSheetCollate->getSheetCollate ();

         std::cout << pszJP
                   << " - SheetCollate: "
                   << SAFE_PRINT_STRING (pstringValue)
                   << " valid: "
                   << pSheetCollate->isValid (pszJP)
                   << " equal: "
                   << pSheetCollate->isEqual (pszJP)
                   << " supported: "
                   << pSheetCollate->isSupported (pszJP)
                   << std::endl;
         delete pstringValue;

         JobProperties          jp (pszJP);
         JobPropertyEnumerator *pEnum      = jp.getEnumeration ();

         std::cout << pszJP
                   << " - ";
         while (  pEnum
               && pEnum->hasMoreElements ()
               )
         {
            PSZCRO pszKey   = pEnum->getCurrentKey ();
            PSZCRO pszValue = pEnum->getCurrentValue ();

            std::cout << pSheetCollate->handlesKey (pszKey)
                      << " = handlesKey (\""
                      << pszKey
                      << "\")";

            pstringValue = pSheetCollate->getJobPropertyType (pszKey);
            std::cout << ", type: " << SAFE_PRINT_STRING (pstringValue);
            delete pstringValue;
            pstringValue = pSheetCollate->getJobProperty (pszKey);
            std::cout << ", property: " << SAFE_PRINT_STRING (pstringValue);
            delete pstringValue;
            pstringValue = pSheetCollate->translateKeyValue (pszKey, pszValue);
            std::cout << ", xlate: " << SAFE_PRINT_STRING (pstringValue) << " ";
            delete pstringValue;

            pEnum->nextElement ();
         }
         delete pEnum;
         std::cout << std::endl;

         pstringValue = pSheetCollate->getAllTranslation ();
         std::cout << pszJP
                   << " - allXlate: " << SAFE_PRINT_STRING (pstringValue)
                   << std::endl;
         delete pstringValue;
         pstringValue = pSheetCollate->getJobProperties (true);
         std::cout << pszJP
                   << " - deviceJP: " << SAFE_PRINT_STRING (pstringValue)
                   << std::endl;
         delete pstringValue;

         BinaryData *pbdData = pSheetCollate->getData ();
         std::cout << pszJP
                   << " - data: ";
         if (pbdData)
         {
            std::cout << *pbdData;
         }
         else
         {
            std::cout << "NULL";
         }
         std::cout << " deviceID: "
                   << SAFE_PRINT_PSZ (pSheetCollate->getDeviceID ())
                   << std::endl;
      }
      else
      {
         std::cerr << "Error: Job Properties (\"" << *pstringRet << "\", \"" << pszJP << "\") should match!"  << std::endl;
      }

      delete pstringRet;
   }
   else
   {
      std::cerr << "Error: Empty Job Properties on new object!" << std::endl;
   }

   delete pSheetCollate;
}

#endif

#ifdef INCLUDE_JP_COMMON_SIDE

void
showSide (Device *pDevice,
          PSZRO   pszJP)
{
   DeviceSide  *pSideCurrent = 0;
   DeviceSide  *pSide        = 0;
   std::string *pstringRet   = 0;

   pSideCurrent = pDevice->getCurrentSide ();

   if (!pSideCurrent)
   {
      std::cerr << "Error: No current Side for " << SAFE_PRINT_PSZ (pszJP) << std::endl;

      return;
   }

   pSide = pSideCurrent->create (pDevice, pszJP);

   if (!pSide)
   {
      std::cerr << "Error: Cannot create a new Side from \"" << pszJP << "\" !" << std::endl;

      return;
   }

   pstringRet = pSide->getJobProperties (false);
   if (pstringRet)
   {
      if (0 == pstringRet->compare (pszJP))
      {
         std::string *pstringValue = 0;

         pstringValue = pSide->getSide ();

         std::cout << pszJP
                   << " - Side: "
                   << SAFE_PRINT_STRING (pstringValue)
                   << " valid: "
                   << pSide->isValid (pszJP)
                   << " equal: "
                   << pSide->isEqual (pszJP)
                   << " supported: "
                   << pSide->isSupported (pszJP)
                   << std::endl;
         delete pstringValue;

         JobProperties          jp (pszJP);
         JobPropertyEnumerator *pEnum      = jp.getEnumeration ();

         std::cout << pszJP
                   << " - ";
         while (  pEnum
               && pEnum->hasMoreElements ()
               )
         {
            PSZCRO pszKey   = pEnum->getCurrentKey ();
            PSZCRO pszValue = pEnum->getCurrentValue ();

            std::cout << pSide->handlesKey (pszKey)
                      << " = handlesKey (\""
                      << pszKey
                      << "\")";

            pstringValue = pSide->getJobPropertyType (pszKey);
            std::cout << ", type: " << SAFE_PRINT_STRING (pstringValue);
            delete pstringValue;
            pstringValue = pSide->getJobProperty (pszKey);
            std::cout << ", property: " << SAFE_PRINT_STRING (pstringValue);
            delete pstringValue;
            pstringValue = pSide->translateKeyValue (pszKey, pszValue);
            std::cout << ", xlate: " << SAFE_PRINT_STRING (pstringValue) << " ";
            delete pstringValue;

            pEnum->nextElement ();
         }
         delete pEnum;
         std::cout << std::endl;

         pstringValue = pSide->getAllTranslation ();
         std::cout << pszJP
                   << " - allXlate: " << SAFE_PRINT_STRING (pstringValue)
                   << std::endl;
         delete pstringValue;
         pstringValue = pSide->getJobProperties (true);
         std::cout << pszJP
                   << " - deviceJP: " << SAFE_PRINT_STRING (pstringValue)
                   << std::endl;
         delete pstringValue;

         BinaryData *pbdData = pSide->getData ();
         std::cout << pszJP
                   << " - data: ";
         if (pbdData)
         {
            std::cout << *pbdData;
         }
         else
         {
            std::cout << "NULL";
         }
         std::cout << " simulation: "
                   << pSide->needsSimulation ()
                   << " deviceID: "
                   << SAFE_PRINT_PSZ (pSide->getDeviceID ())
                   << std::endl;
      }
      else
      {
         std::cerr << "Error: Job Properties (\"" << *pstringRet << "\", \"" << pszJP << "\") should match!"  << std::endl;
      }

      delete pstringRet;
   }
   else
   {
      std::cerr << "Error: Empty Job Properties on new object!" << std::endl;
   }

   delete pSide;
}

#endif

#ifdef INCLUDE_JP_COMMON_STITCHING

void
showStitching (Device *pDevice,
               PSZRO   pszJP)
{
   DeviceStitching  *pStitchingCurrent = 0;
   DeviceStitching  *pStitching        = 0;
   std::string      *pstringRet        = 0;

   pStitchingCurrent = pDevice->getCurrentStitching ();

   if (!pStitchingCurrent)
   {
      std::cerr << "Error: No current Stitching for " << SAFE_PRINT_PSZ (pszJP) << std::endl;

      return;
   }

   pStitching = pStitchingCurrent->create (pDevice, pszJP);

   if (!pStitching)
   {
      std::cerr << "Error: Cannot create a new Stitching from \"" << pszJP << "\" !" << std::endl;

      return;
   }

   pstringRet = pStitching->getJobProperties (false);
   if (pstringRet)
   {
      if (0 == pstringRet->compare (pszJP))
      {
         std::string *pstringValue = 0;

         pstringValue = pStitching->getStitchingReferenceEdge ();

         std::cout << pszJP
                   << " - StitchingPosition: "
                   << pStitching->getStitchingPosition ()
                   << ", StitchingReferenceEdge: "
                   << SAFE_PRINT_STRING (pstringValue);
         delete pstringValue;
         pstringValue = pStitching->getStitchingType ();
         std::cout << ", StitchingType: "
                   << SAFE_PRINT_STRING (pstringValue)
                   << ", StitchingCount: "
                   << pStitching->getStitchingCount ()
                   << ", StitchingAngle: "
                   << pStitching->getStitchingAngle ()
                   << " valid: "
                   << pStitching->isValid (pszJP)
                   << " equal: "
                   << pStitching->isEqual (pszJP)
                   << " supported: "
                   << pStitching->isSupported (pszJP)
                   << std::endl;
         delete pstringValue;

         JobProperties          jp (pszJP);
         JobPropertyEnumerator *pEnum      = jp.getEnumeration ();

         std::cout << pszJP
                   << " - ";
         while (  pEnum
               && pEnum->hasMoreElements ()
               )
         {
            PSZCRO pszKey   = pEnum->getCurrentKey ();
            PSZCRO pszValue = pEnum->getCurrentValue ();

            std::cout << pStitching->handlesKey (pszKey)
                      << " = handlesKey (\""
                      << pszKey
                      << "\")";

            pstringValue = pStitching->getJobPropertyType (pszKey);
            std::cout << ", type: " << SAFE_PRINT_STRING (pstringValue);
            delete pstringValue;
            pstringValue = pStitching->getJobProperty (pszKey);
            std::cout << ", property: " << SAFE_PRINT_STRING (pstringValue);
            delete pstringValue;
            pstringValue = pStitching->translateKeyValue (pszKey, pszValue);
            std::cout << ", xlate: " << SAFE_PRINT_STRING (pstringValue) << " ";
            delete pstringValue;

            pEnum->nextElement ();
         }
         delete pEnum;
         std::cout << std::endl;

         pstringValue = pStitching->getAllTranslation ();
         std::cout << pszJP
                   << " - allXlate: " << SAFE_PRINT_STRING (pstringValue)
                   << std::endl;
         delete pstringValue;
         pstringValue = pStitching->getJobProperties (true);
         std::cout << pszJP
                   << " - deviceJP: " << SAFE_PRINT_STRING (pstringValue)
                   << std::endl;
         delete pstringValue;

         BinaryData *pbdData = pStitching->getData ();
         std::cout << pszJP
                   << " - data: ";
         if (pbdData)
         {
            std::cout << *pbdData;
         }
         else
         {
            std::cout << "NULL";
         }
         std::cout << " deviceID: "
                   << SAFE_PRINT_PSZ (pStitching->getDeviceID ())
                   << std::endl;
      }
      else
      {
         std::cerr << "Error: Job Properties (\"" << *pstringRet << "\", \"" << pszJP << "\") should match!"  << std::endl;
      }

      delete pstringRet;
   }
   else
   {
      std::cerr << "Error: Empty Job Properties on new object!" << std::endl;
   }

   delete pStitching;
}

#endif

void
showTray (Device *pDevice,
          PSZRO   pszJP)
{
   DeviceTray  *pTrayCurrent = 0;
   DeviceTray  *pTray        = 0;
   std::string *pstringRet   = 0;

   pTrayCurrent = pDevice->getCurrentTray ();

   if (!pTrayCurrent)
   {
      std::cerr << "Error: No current Tray for " << SAFE_PRINT_PSZ (pszJP) << std::endl;

      return;
   }

   pTray = pTrayCurrent->create (pDevice, pszJP);

   if (!pTray)
   {
      std::cerr << "Error: Cannot create a new Tray from \"" << pszJP << "\" !" << std::endl;

      return;
   }

   pstringRet = pTray->getJobProperties (false);
   if (pstringRet)
   {
      if (0 == pstringRet->compare (pszJP))
      {
         std::string *pstringValue = 0;

         pstringValue = pTray->getInputTray ();

         std::cout << pszJP
                   << " - InputTray: "
                   << SAFE_PRINT_STRING (pstringValue)
                   << ", Type: "
                   << pTray->getType ()
                   << " valid: "
                   << pTray->isValid (pszJP)
                   << " equal: "
                   << pTray->isEqual (pszJP)
                   << " supported: "
                   << pTray->isSupported (pszJP)
                   << std::endl;
         delete pstringValue;

         JobProperties          jp (pszJP);
         JobPropertyEnumerator *pEnum      = jp.getEnumeration ();

         std::cout << pszJP
                   << " - ";
         while (  pEnum
               && pEnum->hasMoreElements ()
               )
         {
            PSZCRO pszKey   = pEnum->getCurrentKey ();
            PSZCRO pszValue = pEnum->getCurrentValue ();

            std::cout << pTray->handlesKey (pszKey)
                      << " = handlesKey (\""
                      << pszKey
                      << "\")";

            pstringValue = pTray->getJobPropertyType (pszKey);
            std::cout << ", type: " << SAFE_PRINT_STRING (pstringValue);
            delete pstringValue;
            pstringValue = pTray->getJobProperty (pszKey);
            std::cout << ", property: " << SAFE_PRINT_STRING (pstringValue);
            delete pstringValue;
            pstringValue = pTray->translateKeyValue (pszKey, pszValue);
            std::cout << ", xlate: " << SAFE_PRINT_STRING (pstringValue) << " ";
            delete pstringValue;

            pEnum->nextElement ();
         }
         delete pEnum;
         std::cout << std::endl;

         pstringValue = pTray->getAllTranslation ();
         std::cout << pszJP
                   << " - allXlate: " << SAFE_PRINT_STRING (pstringValue)
                   << std::endl;
         delete pstringValue;
         pstringValue = pTray->getJobProperties (true);
         std::cout << pszJP
                   << " - deviceJP: " << SAFE_PRINT_STRING (pstringValue)
                   << std::endl;
         delete pstringValue;

         BinaryData *pbdData = pTray->getData ();
         std::cout << pszJP
                   << " - data: ";
         if (pbdData)
         {
            std::cout << *pbdData;
         }
         else
         {
            std::cout << "NULL";
         }
         std::cout << " deviceID: "
                   << SAFE_PRINT_PSZ (pTray->getDeviceID ())
                   << std::endl;
      }
      else
      {
         std::cerr << "Error: Job Properties (\"" << *pstringRet << "\", \"" << pszJP << "\") should match!"  << std::endl;
      }

      delete pstringRet;
   }
   else
   {
      std::cerr << "Error: Empty Job Properties on new object!" << std::endl;
   }

   delete pTray;
}

#ifdef INCLUDE_JP_COMMON_TRIMMING

void
showTrimming (Device *pDevice,
              PSZRO   pszJP)
{
   DeviceTrimming  *pTrimmingCurrent = 0;
   DeviceTrimming  *pTrimming        = 0;
   std::string     *pstringRet       = 0;

   pTrimmingCurrent = pDevice->getCurrentTrimming ();

   if (!pTrimmingCurrent)
   {
      std::cerr << "Error: No current Trimming for " << SAFE_PRINT_PSZ (pszJP) << std::endl;

      return;
   }

   pTrimming = pTrimmingCurrent->create (pDevice, pszJP);

   if (!pTrimming)
   {
      std::cerr << "Error: Cannot create a new Trimming from \"" << pszJP << "\" !" << std::endl;

      return;
   }

   pstringRet = pTrimming->getJobProperties (false);
   if (pstringRet)
   {
      if (0 == pstringRet->compare (pszJP))
      {
         std::string *pstringValue = 0;

         pstringValue = pTrimming->getTrimming ();

         std::cout << pszJP
                   << " - Trimming: "
                   << SAFE_PRINT_STRING (pstringValue)
                   << " valid: "
                   << pTrimming->isValid (pszJP)
                   << " equal: "
                   << pTrimming->isEqual (pszJP)
                   << " supported: "
                   << pTrimming->isSupported (pszJP)
                   << std::endl;
         delete pstringValue;

         JobProperties          jp (pszJP);
         JobPropertyEnumerator *pEnum      = jp.getEnumeration ();

         std::cout << pszJP
                   << " - ";
         while (  pEnum
               && pEnum->hasMoreElements ()
               )
         {
            PSZCRO pszKey   = pEnum->getCurrentKey ();
            PSZCRO pszValue = pEnum->getCurrentValue ();

            std::cout << pTrimming->handlesKey (pszKey)
                      << " = handlesKey (\""
                      << pszKey
                      << "\")";

            pstringValue = pTrimming->getJobPropertyType (pszKey);
            std::cout << ", type: " << SAFE_PRINT_STRING (pstringValue);
            delete pstringValue;
            pstringValue = pTrimming->getJobProperty (pszKey);
            std::cout << ", property: " << SAFE_PRINT_STRING (pstringValue);
            delete pstringValue;
            pstringValue = pTrimming->translateKeyValue (pszKey, pszValue);
            std::cout << ", xlate: " << SAFE_PRINT_STRING (pstringValue) << " ";
            delete pstringValue;

            pEnum->nextElement ();
         }
         delete pEnum;
         std::cout << std::endl;

         pstringValue = pTrimming->getAllTranslation ();
         std::cout << pszJP
                   << " - allXlate: " << SAFE_PRINT_STRING (pstringValue)
                   << std::endl;
         delete pstringValue;
         pstringValue = pTrimming->getJobProperties (true);
         std::cout << pszJP
                   << " - deviceJP: " << SAFE_PRINT_STRING (pstringValue)
                   << std::endl;
         delete pstringValue;

         BinaryData *pbdData = pTrimming->getData ();
         std::cout << pszJP
                   << " - data: ";
         if (pbdData)
         {
            std::cout << *pbdData;
         }
         else
         {
            std::cout << "NULL";
         }
         std::cout << " deviceID: "
                   << SAFE_PRINT_PSZ (pTrimming->getDeviceID ())
                   << std::endl;
      }
      else
      {
         std::cerr << "Error: Job Properties (\"" << *pstringRet << "\", \"" << pszJP << "\") should match!"  << std::endl;
      }

      delete pstringRet;
   }
   else
   {
      std::cerr << "Error: Empty Job Properties on new object!" << std::endl;
   }

   delete pTrimming;
}

#endif

int
showObjects (Device *pDevice,
             PSZCRO  pszJobProperties)
{
   typedef std::set<std::string> SetList;

   SetList        setElements;
   JobProperties *pJP         = 0;
   PSZRO          pszJP       = 0;
   Enumeration   *pEnum       = 0;

#ifdef INCLUDE_JP_UPDF_BOOKLET
   // @TBD
#endif

#ifdef INCLUDE_JP_COMMON_COPIES
   DeviceCopies *pCopyCurrent = pDevice->getCurrentCopies ();

   setElements.clear ();

   pEnum = pCopyCurrent->getEnumeration ();
   while (pEnum->hasMoreElements ())
   {
      pJP = (JobProperties *)pEnum->nextElement ();

      if (!pJP)
      {
         std::cerr << "Error: Copy Enumertation returned NULL!" << std::endl;
         continue;
      }

      pszJP = pJP->getJobProperties (false);

      if (!pszJP)
      {
         std::cerr << "Error: Copy Job Properties returns NULL!" << std::endl;
         delete pJP;
         break;
      }

      setElements.insert (std::string (pszJP));

      delete pJP;
      free ((void *)pszJP);
   }

   delete pEnum;

   for ( SetList::iterator next = setElements.begin ();
         next != setElements.end ();
         next++
       )
   {
      showCopies (pDevice, next->c_str ());
   }
#endif

   DeviceForm *pFormCurrent = pDevice->getCurrentForm ();

   setElements.clear ();

   pEnum = pFormCurrent->getEnumeration ();
   while (pEnum->hasMoreElements ())
   {
      pJP = (JobProperties *)pEnum->nextElement ();

      if (!pJP)
      {
         std::cerr << "Error: Form Enumertation returned NULL!" << std::endl;
         continue;
      }

      pszJP = pJP->getJobProperties (false);

      if (!pszJP)
      {
         std::cerr << "Error: Form Job Properties returns NULL!" << std::endl;
         delete pJP;
         break;
      }

      setElements.insert (std::string (pszJP));

      delete pJP;
      free ((void *)pszJP);
   }

   delete pEnum;

   for ( SetList::iterator next = setElements.begin ();
         next != setElements.end ();
         next++
       )
   {
      showForm (pDevice, next->c_str ());
   }

#ifdef INCLUDE_JP_UPDF_JOGGING
   // @TBD
#endif

   DeviceMedia *pMediaCurrent = pDevice->getCurrentMedia ();

   setElements.clear ();

   pEnum = pMediaCurrent->getEnumeration ();
   while (pEnum->hasMoreElements ())
   {
      pJP = (JobProperties *)pEnum->nextElement ();

      if (!pJP)
      {
         std::cerr << "Error: Media Enumertation returned NULL!" << std::endl;
         continue;
      }

      pszJP = pJP->getJobProperties (false);

      if (!pszJP)
      {
         std::cerr << "Error: Media Job Properties returns NULL!" << std::endl;
         delete pJP;
         break;
      }

      setElements.insert (std::string (pszJP));

      delete pJP;
      free ((void *)pszJP);
   }

   delete pEnum;

   for ( SetList::iterator next = setElements.begin ();
         next != setElements.end ();
         next++
       )
   {
      showMedia (pDevice, next->c_str ());
   }

#ifdef INCLUDE_JP_COMMON_NUP
   DeviceNUp *pNUpCurrent = pDevice->getCurrentNUp ();

   setElements.clear ();

   pEnum = pNUpCurrent->getEnumeration ();
   while (pEnum->hasMoreElements ())
   {
      pJP = (JobProperties *)pEnum->nextElement ();

      if (!pJP)
      {
         std::cerr << "Error: NUp Enumertation returned NULL!" << std::endl;
         continue;
      }

      pszJP = pJP->getJobProperties (false);

      if (!pszJP)
      {
         std::cerr << "Error: NUp Job Properties returns NULL!" << std::endl;
         delete pJP;
         break;
      }

      setElements.insert (std::string (pszJP));

      delete pJP;
      free ((void *)pszJP);
   }

   delete pEnum;

   for ( SetList::iterator next = setElements.begin ();
         next != setElements.end ();
         next++
       )
   {
      showNUp (pDevice, next->c_str ());
   }
#endif

   DeviceOrientation *pOrientationCurrent = pDevice->getCurrentOrientation ();

   setElements.clear ();

   pEnum = pOrientationCurrent->getEnumeration ();
   while (pEnum->hasMoreElements ())
   {
      pJP = (JobProperties *)pEnum->nextElement ();

      if (!pJP)
      {
         std::cerr << "Error: Orientation Enumertation returned NULL!" << std::endl;
         continue;
      }

      pszJP = pJP->getJobProperties (false);

      if (!pszJP)
      {
         std::cerr << "Error: Orientation Job Properties returns NULL!" << std::endl;
         delete pJP;
         break;
      }

      setElements.insert (std::string (pszJP));

      delete pJP;
      free ((void *)pszJP);
   }

   delete pEnum;

   for ( SetList::iterator next = setElements.begin ();
         next != setElements.end ();
         next++
       )
   {
      showOrientation (pDevice, next->c_str ());
   }

#ifdef INCLUDE_JP_COMMON_OUTPUT_BIN
   DeviceOutputBin *pOutputBinCurrent = pDevice->getCurrentOutputBin ();

   setElements.clear ();

   pEnum = pOutputBinCurrent->getEnumeration ();
   while (pEnum->hasMoreElements ())
   {
      pJP = (JobProperties *)pEnum->nextElement ();

      if (!pJP)
      {
         std::cerr << "Error: OutputBin Enumertation returned NULL!" << std::endl;
         continue;
      }

      pszJP = pJP->getJobProperties (false);

      if (!pszJP)
      {
         std::cerr << "Error: OutputBin Job Properties returns NULL!" << std::endl;
         delete pJP;
         break;
      }

      setElements.insert (std::string (pszJP));

      delete pJP;
      free ((void *)pszJP);
   }

   delete pEnum;

   for ( SetList::iterator next = setElements.begin ();
         next != setElements.end ();
         next++
       )
   {
      showOutputBin (pDevice, next->c_str ());
   }
#endif

   DevicePrintMode *pPrintModeCurrent = pDevice->getCurrentPrintMode ();

   setElements.clear ();

   pEnum = pPrintModeCurrent->getEnumeration ();
   while (pEnum->hasMoreElements ())
   {
      pJP = (JobProperties *)pEnum->nextElement ();

      if (!pJP)
      {
         std::cerr << "Error: PrintMode Enumertation returned NULL!" << std::endl;
         continue;
      }

      pszJP = pJP->getJobProperties (false);

      if (!pszJP)
      {
         std::cerr << "Error: PrintMode Job Properties returns NULL!" << std::endl;
         delete pJP;
         break;
      }

      setElements.insert (std::string (pszJP));

      delete pJP;
      free ((void *)pszJP);
   }

   delete pEnum;

   for ( SetList::iterator next = setElements.begin ();
         next != setElements.end ();
         next++
       )
   {
      showPrintMode (pDevice, next->c_str ());
   }

   DeviceResolution *pResolutionCurrent = pDevice->getCurrentResolution ();

   setElements.clear ();

   pEnum = pResolutionCurrent->getEnumeration ();
   while (pEnum->hasMoreElements ())
   {
      pJP = (JobProperties *)pEnum->nextElement ();

      if (!pJP)
      {
         std::cerr << "Error: Resolution Enumertation returned NULL!" << std::endl;
         continue;
      }

      pszJP = pJP->getJobProperties (false);

      if (!pszJP)
      {
         std::cerr << "Error: Resolution Job Properties returns NULL!" << std::endl;
         delete pJP;
         break;
      }

      setElements.insert (std::string (pszJP));

      delete pJP;
      free ((void *)pszJP);
   }

   delete pEnum;

   for ( SetList::iterator next = setElements.begin ();
         next != setElements.end ();
         next++
       )
   {
      showResolution (pDevice, next->c_str ());
   }

#ifdef INCLUDE_JP_COMMON_SCALING
   DeviceScaling *pScalingCurrent = pDevice->getCurrentScaling ();

   setElements.clear ();

   pEnum = pScalingCurrent->getEnumeration ();
   while (pEnum->hasMoreElements ())
   {
      pJP = (JobProperties *)pEnum->nextElement ();

      if (!pJP)
      {
         std::cerr << "Error: Scaling Enumertation returned NULL!" << std::endl;
         continue;
      }

      pszJP = pJP->getJobProperties (false);

      if (!pszJP)
      {
         std::cerr << "Error: Scaling Job Properties returns NULL!" << std::endl;
         delete pJP;
         break;
      }

      setElements.insert (std::string (pszJP));

      delete pJP;
      free ((void *)pszJP);
   }

   delete pEnum;

   for ( SetList::iterator next = setElements.begin ();
         next != setElements.end ();
         next++
       )
   {
      showScaling (pDevice, next->c_str ());
   }
#endif

#ifdef INCLUDE_JP_COMMON_SHEET_COLLATE
   DeviceSheetCollate *pSheetCollateCurrent = pDevice->getCurrentSheetCollate ();

   setElements.clear ();

   pEnum = pSheetCollateCurrent->getEnumeration ();
   while (pEnum->hasMoreElements ())
   {
      pJP = (JobProperties *)pEnum->nextElement ();

      if (!pJP)
      {
         std::cerr << "Error: SheetCollate Enumertation returned NULL!" << std::endl;
         continue;
      }

      pszJP = pJP->getJobProperties (false);

      if (!pszJP)
      {
         std::cerr << "Error: SheetCollate Job Properties returns NULL!" << std::endl;
         delete pJP;
         break;
      }

      setElements.insert (std::string (pszJP));

      delete pJP;
      free ((void *)pszJP);
   }

   delete pEnum;

   for ( SetList::iterator next = setElements.begin ();
         next != setElements.end ();
         next++
       )
   {
      showSheetCollate (pDevice, next->c_str ());
   }
#endif

#ifdef INCLUDE_JP_COMMON_SIDE
   DeviceSide *pSideCurrent = pDevice->getCurrentSide ();

   setElements.clear ();

   pEnum = pSideCurrent->getEnumeration ();
   while (pEnum->hasMoreElements ())
   {
      pJP = (JobProperties *)pEnum->nextElement ();

      if (!pJP)
      {
         std::cerr << "Error: Side Enumertation returned NULL!" << std::endl;
         continue;
      }

      pszJP = pJP->getJobProperties (false);

      if (!pszJP)
      {
         std::cerr << "Error: Side Job Properties returns NULL!" << std::endl;
         delete pJP;
         break;
      }

      setElements.insert (std::string (pszJP));

      delete pJP;
      free ((void *)pszJP);
   }

   delete pEnum;

   for ( SetList::iterator next = setElements.begin ();
         next != setElements.end ();
         next++
       )
   {
      showSide (pDevice, next->c_str ());
   }
#endif

#ifdef INCLUDE_JP_COMMON_STITCHING
   DeviceStitching *pStitchingCurrent = pDevice->getCurrentStitching ();

   setElements.clear ();

   pEnum = pStitchingCurrent->getEnumeration ();
   while (pEnum->hasMoreElements ())
   {
      pJP = (JobProperties *)pEnum->nextElement ();

      if (!pJP)
      {
         std::cerr << "Error: Stitching Enumertation returned NULL!" << std::endl;
         continue;
      }

      pszJP = pJP->getJobProperties (false);

      if (!pszJP)
      {
         std::cerr << "Error: Stitching Job Properties returns NULL!" << std::endl;
         delete pJP;
         break;
      }

      setElements.insert (std::string (pszJP));

      delete pJP;
      free ((void *)pszJP);
   }

   delete pEnum;

   for ( SetList::iterator next = setElements.begin ();
         next != setElements.end ();
         next++
       )
   {
      showStitching (pDevice, next->c_str ());
   }
#endif

   DeviceTray *pTrayCurrent = pDevice->getCurrentTray ();

   setElements.clear ();

   pEnum = pTrayCurrent->getEnumeration ();
   while (pEnum->hasMoreElements ())
   {
      pJP = (JobProperties *)pEnum->nextElement ();

      if (!pJP)
      {
         std::cerr << "Error: Tray Enumertation returned NULL!" << std::endl;
         continue;
      }

      pszJP = pJP->getJobProperties (false);

      if (!pszJP)
      {
         std::cerr << "Error: Tray Job Properties returns NULL!" << std::endl;
         delete pJP;
         break;
      }

      setElements.insert (std::string (pszJP));

      delete pJP;
      free ((void *)pszJP);
   }

   delete pEnum;

   for ( SetList::iterator next = setElements.begin ();
         next != setElements.end ();
         next++
       )
   {
      showTray (pDevice, next->c_str ());
   }

#ifdef INCLUDE_JP_COMMON_TRIMMING
   DeviceTrimming *pTrimmingCurrent = pDevice->getCurrentTrimming ();

   setElements.clear ();

   pEnum = pTrimmingCurrent->getEnumeration ();
   while (pEnum->hasMoreElements ())
   {
      pJP = (JobProperties *)pEnum->nextElement ();

      if (!pJP)
      {
         std::cerr << "Error: Trimming Enumertation returned NULL!" << std::endl;
         continue;
      }

      pszJP = pJP->getJobProperties (false);

      if (!pszJP)
      {
         std::cerr << "Error: Trimming Job Properties returns NULL!" << std::endl;
         delete pJP;
         break;
      }

      setElements.insert (std::string (pszJP));

      delete pJP;
      free ((void *)pszJP);
   }

   delete pEnum;

   for ( SetList::iterator next = setElements.begin ();
         next != setElements.end ();
         next++
       )
   {
      showTrimming (pDevice, next->c_str ());
   }
#endif

   return 0;
}

int
executeDevice (PSZRO  pszFullDeviceName,
               bool   fUsePDC,
               PSZCRO pszJobProperties)
{
   Device       *pDevice      = 0;
   GModule      *hmodDevice   = 0;
   int           rc           = 0;

#ifndef RETAIL
   if (DebugOutput::shouldOutputDeviceTester ()) DebugOutput::getErrorStream () << "OmniDeviceOptions::" << __FUNCTION__ << ": pszFullDeviceName = \"" << SAFE_PRINT_PSZ (pszFullDeviceName) << "\" pszJobProperties = \"" << SAFE_PRINT_PSZ (pszJobProperties) << "\"" << std::endl;
#endif

   if (fUsePDC)
   {
#ifdef PDC_INTERFACE_ENABLED
      const char *pszSlash = strchr (pszFullDeviceName, '/');

      while (pszSlash)
      {
         pszFullDeviceName = const_cast<char*>(pszSlash + 1);

         pszSlash = strchr (pszFullDeviceName, '/');
      }

      pDevice = new OmniPDCProxy (0,                 // client exe to spawn
                                  pszFullDeviceName, // device name
                                  pszJobProperties,  // job properties
                                  true);             // is renderer
#endif
   }
   else
   {
      PFNNEWDEVICE       pfnNewDevice      = 0;
      PFNNEWDEVICEWARGS  pfnNewDeviceWArgs = 0;

      hmodDevice = g_module_open (pszFullDeviceName, (GModuleFlags)0);

#ifndef RETAIL
      if (DebugOutput::shouldOutputDeviceTester ()) DebugOutput::getErrorStream () << "OmniDeviceOptions::" << __FUNCTION__ << ": hmodDevice        = " << std::hex << hmodDevice << std::dec << std::endl;
#endif

      if (!hmodDevice)
      {
         DebugOutput::getErrorStream ()
            << "Error: trying to load \""
            << SAFE_PRINT_PSZ (pszFullDeviceName)
            << "\" g_module_error returns "
            << g_module_error ()
            << std::endl;

         return 2;
      }

      g_module_symbol (hmodDevice, "newDeviceW_Advanced",         (gpointer *)&pfnNewDevice);
      g_module_symbol (hmodDevice, "newDeviceW_JopProp_Advanced", (gpointer *)&pfnNewDeviceWArgs);

#ifndef RETAIL
      if (DebugOutput::shouldOutputDeviceTester ()) DebugOutput::getErrorStream () << "OmniDeviceOptions::" << __FUNCTION__ << std::hex << ": pfnNewDevice      = 0x" << (int)pfnNewDevice << std::endl;
      if (DebugOutput::shouldOutputDeviceTester ()) DebugOutput::getErrorStream () << "OmniDeviceOptions::" << __FUNCTION__ << std::hex << ": pfnNewDeviceWArgs = 0x" << (int)pfnNewDeviceWArgs << std::endl;
#endif

      if (  !pfnNewDevice
         || !pfnNewDeviceWArgs
         )
      {
         DebugOutput::getErrorStream () << "Error: trying to load newDeviceW[_JobProp]_Advanced. g_module_error returns " << g_module_error () << std::endl;

         return 3;
      }

      if (  !pszJobProperties
         || !*pszJobProperties
         )
      {
         pDevice = pfnNewDevice (true);
      }
      else
      {
         pDevice = pfnNewDeviceWArgs (pszJobProperties, true);
      }
   }

#ifndef RETAIL
   if (DebugOutput::shouldOutputDeviceTester ()) DebugOutput::getErrorStream () << ": pDevice = " << *pDevice << std::endl;
#endif

   if (!pDevice)
   {
      DebugOutput::getErrorStream () << "Error: No Device was created!" << std::endl;

      return 4;
   }

   // Check for an error
   if (pDevice->hasError ())
   {
      DebugOutput::getErrorStream () << "Error: The device has an error." << std::endl;

      return 5;
   }

   rc = showObjects (pDevice, pszJobProperties);

   if (pDevice->hasError ())
   {
      DebugOutput::getErrorStream () << "Error: The device has an error." << std::endl;

      return 6;
   }

   delete pDevice;

   if (hmodDevice)
   {
#ifndef RETAIL
      int rc2 =
#endif

                g_module_close (hmodDevice);

#ifndef RETAIL
      if (DebugOutput::shouldOutputDeviceTester ()) DebugOutput::getErrorStream () << "OmniDeviceOptions::" << __FUNCTION__ << ": g_module_close returns " << rc2 << std::endl;
#endif
   }

   return rc;
}

int
main (int argc, char *argv[])
{
   int            rc                = 0;
   bool           fUsePDC           = false;
   PSZRO          pszJobProperties  = 0;
   bool           fBuildOnly        = false;
   JobProperties  jobProp;

   if (!g_module_supported ())
   {
      DebugOutput::getErrorStream () << "Error: This program needs glib's module routines!" << std::endl;

      return __LINE__;
   }

   if (2 > argc)
   {
      printUsage (argv[0]);

      return __LINE__;
   }

   Omni::initialize ();

   for (int i = 1; i < argc; i++)
   {
      if (0 == strcasecmp ("--buildOnly", argv[i]))
      {
         fBuildOnly = true;
      }
      else if (0 == strcasecmp (argv[i], "--all"))
      {
         Enumeration *pEnum = Omni::listDevices (fBuildOnly);

         while (pEnum->hasMoreElements ())
         {
            OmniDevice *pOD = (OmniDevice *)pEnum->nextElement ();

            if (pOD)
            {
               PSZCRO             pszLibName           = pOD->getLibraryName ();
               PSZRO              pszJobProps          = pOD->getJobProperties ();
               JobProperties      jobPropNew           = jobProp;
               std::ostringstream oss;
               std::string        stringOutputFilename;
               std::string        stringErrorFilename;
               GModule           *phmodDevice          = 0;
               Device            *pDevice              = 0;
               int                rcTDO                = 0;

               pDevice = Omni::createDevice (pOD, &phmodDevice);

               if (pDevice)
               {
                  oss << pDevice->getShortName ()
                      << ".out";

                  stringOutputFilename = oss.str ();

                  oss.str ("");

                  oss << pDevice->getShortName ()
                      << ".err";

                  stringErrorFilename = oss.str ();

                  oss.str ("");
                  delete pDevice;
               }

               g_module_close (phmodDevice);

               jobPropNew.setJobProperties (pszJobProps);

               pszJobProps = jobPropNew.getJobProperties ();

               oss << "TestDeviceObjects";
               if (fUsePDC)
               {
                  oss << " --UsePDC";
               }
               if (fBuildOnly)
               {
                  oss << " --buildOnly";
               }
               if (pszJobProps)
               {
                  oss << " -sproperties='"
                      << pszJobProps
                      << "'";
               }
               oss << " --driver \""
                   << pszLibName
                   << "\"";
               if (stringOutputFilename[0])
               {
                  oss << " > \""
                      << stringOutputFilename
                      << "\"";

                  unlink (stringOutputFilename.c_str ());
               }
               if (stringErrorFilename[0])
               {
                  oss << " 2> \""
                      << stringErrorFilename
                      << "\"";

                  unlink (stringErrorFilename.c_str ());
               }

               std::cout << oss.str () << " = ";
               std::cout.flush ();

               rcTDO = Omni::my_system (oss.str ().c_str ());

               std::cout << rcTDO << std::endl;

               if (pszJobProps)
               {
                  free ((void*)pszJobProps);
                  pszJobProps = 0;
               }

               delete pOD;
            }
            else
            {
               std::cerr << "Error: No OmniDevice was returned!" << std::endl;

               rc = __LINE__;
            }
         }

         delete pEnum;
      }
      else if (0 == strcasecmp (argv[i], "--UsePDC"))
      {
         fUsePDC = true;
      }
      else if (0 == strcasecmp (argv[i], "--!UsePDC"))
      {
         fUsePDC = false;
      }
      else if (0 == strcasecmp (argv[i], "--driver"))
      {
         std::ostringstream  oss;
         std::string         stringOss;
         char               *pszDriverLibrary = argv[i + 1];

         if ( 0 == strchr (pszDriverLibrary, '/')
            && 0 != strncasecmp (pszDriverLibrary, "lib", 3)
            )
         {
            oss << "lib"
                << pszDriverLibrary
                << ".so";

            stringOss        = oss.str ();
            pszDriverLibrary = (char *)stringOss.c_str ();
         }

         rc |= executeDevice (pszDriverLibrary,
                              fUsePDC,
                              pszJobProperties);

         i++;
      }
      else if (0 == strncasecmp ("-sproperties=", argv[i], 13))
      {
         char *pszJobProp = argv[i] + 13;

         jobProp.setJobProperties (pszJobProp);

#ifndef RETAIL
         jobProp.applyAllDebugOutput ();
#endif

         if (pszJobProperties)
         {
            free ((void *)pszJobProperties);
            pszJobProperties = 0;
         }
         pszJobProperties = jobProp.getJobProperties ();
      }
      else
      {
         DeviceInfo *pDI = Omni::findDeviceInfoFromShortName (argv[i], fBuildOnly);

         if (pDI)
         {
            Device     *pDevice    = 0;
            GModule    *hmodDevice = 0;
            OmniDevice *pOD        = 0;
            PSZRO       pszJobProp = 0;

            pDevice    = pDI->getDevice ();
            hmodDevice = pDI->getHmodDevice ();
            pOD        = pDI->getOmniDevice ();

            if (pOD)
            {
               pszJobProp = pOD->getJobProperties ();

               jobProp.setJobProperties (pszJobProp);

               pszJobProp = jobProp.getJobProperties ();
            }

            std::cout << "Info: For faster results use:"
                      << std::endl
                      << "Info:\t"
                      << argv[0];

            if (pszJobProp)
            {
               std::cout << " '-sproperties="
                         << pszJobProp
                         << "'";
            }

            std::cout << " --driver "
                      << pDevice->getLibraryName ()
                      << std::endl;

            rc |= executeDevice (pDevice->getLibraryName (),
                                 fUsePDC,
                                 pszJobProp);

            if (pszJobProp)
            {
               free ((void *)pszJobProp);
               pszJobProp = 0;
            }

            delete pDI;
         }
      }
   }

   // Clean up!
   if (pszJobProperties)
   {
      free ((void *)pszJobProperties);
      pszJobProperties = 0;
   }

   // Clean up!
   Omni::terminate ();

   return rc;
}
