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
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <assert.h>

#include "hppcl3.hpp"
#include "databuf.hpp"

/****************************************************************************/
/*                                                                          */
/****************************************************************************/
DataBuffer ::
DataBuffer (char *pszFileName, int iMaxSize)
{
   iMaxSize_d = iMaxSize;

   fp_d = fopen (pszFileName, "rb");

   iNdx_d        = 0;
   iBytesLeft_d  = 0;

   pbData_d = (PBYTE)malloc (iMaxSize);

   fAddToTotalIndex_d = false;
   iTotalIndex_d      = 0;
}

/****************************************************************************/
/*                                                                          */
/****************************************************************************/
int DataBuffer ::
GetByte (int iBytesToSave)
{
   int    iRet          = 0;
   int    iBytesToRead;
   int    iBytesRead;

   if (!pbData_d)
      return -1;

   if (iBytesLeft_d == 0)
   {
      DBPRINTIF ((DEBUG_LEVEL & DEBUG_IO,
                  "%s: Saving %d bytes...\n",
                  __FUNCTION__, iBytesToSave));
      DBPRINTIF ((DEBUG_LEVEL & DEBUG_IO,
                  "%s: %d bytes left\n",
                  __FUNCTION__, iBytesLeft_d));

      DBPRINTIF ((DEBUG_LEVEL & DEBUG_IO,
                  "%s: fAddToTotalIndex_d = %d\n",
                  __FUNCTION__, fAddToTotalIndex_d));
      DBPRINTIF ((DEBUG_LEVEL & DEBUG_IO,
                  "%s: iTotalIndex_d = %d\n",
                  __FUNCTION__, iTotalIndex_d));

      if (iBytesToSave > iNdx_d)
      {
         assertT (iBytesToSave > iNdx_d);

         iBytesToSave = iNdx_d;
      }

      memmove (pbData_d,
               pbData_d + iNdx_d - iBytesToSave,
               iBytesToSave);
      iBytesLeft_d = iBytesToSave;

      iBytesToRead = iMaxSize_d - iBytesLeft_d;

      iBytesRead = fread (pbData_d + iBytesLeft_d,
                          sizeof (BYTE),
                          iBytesToRead,
                          fp_d);

      DBPRINTIF ((DEBUG_LEVEL & DEBUG_IO,
                  "%s: Read %d bytes\n",
                  __FUNCTION__, iBytesRead));
////////////assertT (iBytesRead != iBytesToRead);

      if (fAddToTotalIndex_d)
         iTotalIndex_d += iBytesRead;

      iBytesLeft_d      += iBytesRead;

      iNdx_d             = iBytesToSave;
      iBytesLeft_d      -= iBytesToSave;

      fAddToTotalIndex_d = true;

      DBPRINTIF ((DEBUG_LEVEL & DEBUG_IO,
                  "%s: fAddToTotalIndex_d = %d\n",
                  __FUNCTION__, fAddToTotalIndex_d));
      DBPRINTIF ((DEBUG_LEVEL & DEBUG_IO,
                  "%s: iTotalIndex_d = %d\n",
                  __FUNCTION__, iTotalIndex_d));
   }

   if (0 < iBytesLeft_d)
   {
      --iBytesLeft_d;

      iRet = pbData_d[iNdx_d];

      ++iNdx_d;
   }
   else
   {
      iRet = pbData_d[iNdx_d - 1];
   }

   return iRet;
}

/****************************************************************************/
/*                                                                          */
/****************************************************************************/
PBYTE DataBuffer ::
BufferBytes (int iBytesToBuffer,
             int iBytesToSave)
{
   int    iBytesToRead;
   int    iBytesRead;

   if (!pbData_d)
      return NULL;

   if (iBytesLeft_d < iBytesToBuffer)
   {
      DBPRINTIF ((DEBUG_LEVEL & DEBUG_IO,
                  "%s: Buffering %d bytes...\n",
                  __FUNCTION__, iBytesToBuffer));
      DBPRINTIF ((DEBUG_LEVEL & DEBUG_IO,
                  "%s: Saving %d bytes...\n",
                  __FUNCTION__, iBytesToSave));
      DBPRINTIF ((DEBUG_LEVEL & DEBUG_IO,
                  "%s: %d bytes left\n",
                  __FUNCTION__, iBytesLeft_d));

      DBPRINTIF ((DEBUG_LEVEL & DEBUG_IO,
                  "%s: fAddToTotalIndex_d = %d\n",
                  __FUNCTION__, fAddToTotalIndex_d));
      DBPRINTIF ((DEBUG_LEVEL & DEBUG_IO,
                  "%s: iTotalIndex_d = %d\n",
                  __FUNCTION__, iTotalIndex_d));

      if (iBytesToSave > iNdx_d)
      {
         assertT (iBytesToSave > iNdx_d);

         iBytesToSave = iNdx_d;
      }

      memmove (pbData_d,
               pbData_d + iNdx_d - iBytesToSave,
               iBytesToSave + iBytesLeft_d);
      iBytesLeft_d = iBytesToSave + iBytesLeft_d;

      iBytesToRead = iMaxSize_d - iBytesLeft_d;

      iBytesRead = fread (pbData_d + iBytesLeft_d,
                          sizeof (BYTE),
                          iBytesToRead,
                          fp_d);

      DBPRINTIF ((DEBUG_LEVEL & DEBUG_IO,
                  "%s: Read %d bytes\n",
                  __FUNCTION__, iBytesRead));
////////////assertT (iBytesRead != iBytesToRead);

      if (fAddToTotalIndex_d)
         iTotalIndex_d += iBytesRead;

      iBytesLeft_d      += iBytesRead;

      iNdx_d             = iBytesToSave;
      iBytesLeft_d      -= iBytesToSave;

      fAddToTotalIndex_d = true;

      DBPRINTIF ((DEBUG_LEVEL & DEBUG_IO,
                  "%s: fAddToTotalIndex_d = %d\n",
                  __FUNCTION__, fAddToTotalIndex_d));
      DBPRINTIF ((DEBUG_LEVEL & DEBUG_IO,
                  "%s: iTotalIndex_d = %d\n",
                  __FUNCTION__, iTotalIndex_d));
   }

   if (iBytesLeft_d > 0)
   {
      int      iOldIndx = iNdx_d;

      iNdx_d       += omni::min (iBytesToBuffer, iBytesLeft_d);
      iBytesLeft_d -= omni::min (iBytesToBuffer, iBytesLeft_d);

      return pbData_d + iOldIndx;
   }
   else
   {
      return NULL;
   }
}

/****************************************************************************/
/*                                                                          */
/****************************************************************************/
int DataBuffer ::
CurrentPosition (void)
{
   return iNdx_d + iTotalIndex_d;
}

/****************************************************************************/
/*                                                                          */
/****************************************************************************/
PBYTE DataBuffer ::
PreviousData (int iBytes)
{
   if (!pbData_d)
      return NULL;

   if (iNdx_d < iBytes)
      return NULL;

   return pbData_d + iNdx_d - iBytes;
}

/****************************************************************************/
/*                                                                          */
/****************************************************************************/
bool DataBuffer ::
MoreDataLeft (void)
{
   if (  pbData_d                    // have a buffer
      && (  (  0 < iBytesLeft_d )    // have bytes in it
         || (  0 < fp_d              // have a file handle
            && !feof (fp_d)          // not out of data in it
            )
         )
      )
      return true;
   else
      return false;
}

/****************************************************************************/
/*                                                                          */
/****************************************************************************/
DataBuffer ::
~DataBuffer (void)
{
   if (fp_d)
   {
      fclose (fp_d);
   }

   if (pbData_d)
   {
      free (pbData_d);
   }
}
